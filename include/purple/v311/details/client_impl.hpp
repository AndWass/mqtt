//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

/** @file */

#pragma once

#include "handshake.hpp"

#include <purple/binary.hpp>
#include <purple/connection_event.hpp>
#include <purple/stream.hpp>

#include <purple/v311/connect_message.hpp>

#include <boost/asio/executor.hpp>
#include <boost/asio/steady_timer.hpp>

#include <boost/beast/core/flat_buffer.hpp>

#include <boost/assert.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/utility/string_view.hpp>

#include <functional>

namespace purple {
namespace v311 {
namespace details {

template<class AsyncDefaultConnectableStream>
struct client_impl : public boost::enable_shared_from_this<client_impl<AsyncDefaultConnectableStream>> {
    using Self = client_impl<AsyncDefaultConnectableStream>;

    template<class... Args>
    struct erased_handler_base {
        virtual ~erased_handler_base() = default;
        virtual void operator()(Args...) = 0;
    };

    template<class Handler, class... Args>
    struct erased_handler : public erased_handler_base<Args...> {
        Handler h_;

        template<class H2>
        explicit erased_handler(H2 &&h) : h_(std::forward<H2>(h)) {
        }

        void operator()(Args... args) override {
            h_(args...);
        }
    };

    enum class state_t {
        idle,
        socket_connecting,
        handshaking,
        connected,
        connection_closed,
    };
    purple::stream<AsyncDefaultConnectableStream> stream_;
    boost::beast::flat_buffer read_buffer_;

    boost::asio::steady_timer ping_timer_;
    boost::asio::steady_timer connection_timeout_timer_;

    std::unique_ptr<erased_handler_base<boost::system::error_code>> run_handler_;
    std::function<void(boost::system::error_code, connection_event)> connection_handler_;
    std::function<void(connect_message &)> connect_decorator_;
    std::function<void(purple::fixed_header)> message_callback_;

    state_t state_ = state_t::idle;

    std::chrono::seconds keep_alive_{};

    connect_message connect_;

    void close() {
        boost::beast::close_socket(boost::beast::get_lowest_layer(stream_.next_layer()));
        set_state(state_t::connection_closed);
    }

    void start_read_one_message() {
        stream_.async_read(read_buffer_, read_handler(this->weak_from_this()));
    }

    void start_connection_timeout_timer() {
        if (connect_.keep_alive.count() > 0) {
            std::chrono::milliseconds keep_alive = connect_.keep_alive;
            connection_timeout_timer_.expires_after(keep_alive + keep_alive / 2);
            connection_timeout_timer_.async_wait(timeout_timer_handler(this->weak_from_this()));
        }
    }

    void start_ping_timer() {
        if (connect_.keep_alive.count() > 0) {
            start_connection_timeout_timer();
            ping_timer_.expires_after(connect_.keep_alive);
            ping_timer_.async_wait(ping_handler(this->weak_from_this()));
        }
    }

    void report_connection_event(boost::system::error_code ec, connection_event evt) {
        if (connection_handler_) {
            connection_handler_(ec, evt);
        }
    }

    static auto read_handler(boost::weak_ptr<client_impl> weak) {
        return [weak](boost::system::error_code ec, purple::fixed_header header) {
            auto self = weak.lock();
            if (self && !ec) {

                if (self->message_callback_) {
                    (self->message_callback_)(header);
                }
                self->read_buffer_.clear();
                self->start_ping_timer();
                self->start_read_one_message();
            }
        };
    }

    static auto ping_handler(boost::weak_ptr<Self> weak) {
        return [weak](boost::system::error_code ec) {
            if (!ec) {
                if (auto self = weak.lock()) {
                    self->stream_.async_write(0xc0, boost::asio::const_buffer{}, [](boost::system::error_code, size_t) {
                    });
                }
            } else {
            }
        };
    }

    static auto timeout_timer_handler(boost::weak_ptr<Self> weak) {
        return [weak](boost::system::error_code ec) {
            if (!ec) {
                if (auto self = weak.lock()) {
                    self->close();
                }
            }
        };
    }

    static auto handshake_handler(boost::weak_ptr<Self> weak) {
        return [weak](boost::system::error_code ec, bool session_present) {
            auto self = weak.lock();
            if (self && !ec) {
                self->set_state(state_t::connected);
                self->report_connection_event(ec, connection_event::handshake_successful);
            } else if (self) {
                self->close();
                self->set_state(state_t::connection_closed);
                self->report_connection_event(ec, connection_event::socket_disconnected);
            }
        };
    }

    static auto socket_connection_handler(boost::weak_ptr<Self> weak) {
        return [weak](boost::system::error_code ec) {
            auto self = weak.lock();
            if (self && !ec) {
                self->connect_.keep_alive = self->keep_alive_;
                if (self->connect_decorator_) {
                    self->connect_decorator_(self->connect_);
                }
                self->async_handshake(handshake_handler(weak));
                self->set_state(state_t::handshaking);
                self->report_connection_event(ec, connection_event::socket_connected);
            } else {
                self->set_state(state_t::connection_closed);
                self->report_connection_event(ec, connection_event::socket_disconnected);
            }
        };
    }

    void set_state(state_t new_state) {
        if (state_ == state_t::idle && new_state == state_t::socket_connecting) {
            stream_.next_layer().async_connect(socket_connection_handler(this->weak_from_this()));
            report_connection_event({}, connection_event::initiating_socket_connection);
        } else if (new_state == state_t::handshaking && state_ == state_t::socket_connecting) {
            start_connection_timeout_timer();
        } else if (new_state == state_t::connected && state_ != state_t::connected) {
            start_read_one_message();
            start_connection_timeout_timer();
            start_ping_timer();
        }

        state_ = new_state;
    }

    template<class Handler>
    void async_handshake(Handler &&handler) {
        const size_t size = connect_.wire_size();
        std::unique_ptr<uint8_t[]> to_write = std::make_unique<uint8_t[]>(size);
        connect_.write_to(to_write.get());

        boost::asio::async_compose<Handler, void(boost::system::error_code, bool)>(
            details::handshake_op<AsyncDefaultConnectableStream>{stream_, std::move(to_write), size, {}}, handler,
            stream_);
    }

    template<class... Args>
    explicit client_impl(Args &&...args)
        : stream_(std::forward<Args>(args)...), ping_timer_(stream_.get_executor()),
          connection_timeout_timer_(stream_.get_executor()) {
    }

    template<class Handler>
    typename boost::asio::async_result<std::decay_t<Handler>, void(boost::system::error_code)>::return_type
    async_run(boost::string_view client_id, boost::string_view username, boost::string_view password,
              Handler &&handler) {
        BOOST_ASSERT(state_ == state_t::idle);

        using handler_t = erased_handler<std::decay_t<Handler>, boost::system::error_code>;

        connect_.client_id.assign(client_id.data(), client_id.size());
        connect_.username.assign(username.data(), username.size());
        if (!username.empty()) {
            connect_.password.assign(password.data(), password.size());
        } else {
            connect_.password = "";
        }

        return boost::asio::async_initiate<Handler, void(boost::system::error_code)>(
            [this](Handler &&completion) {
                run_handler_ = std::make_unique<handler_t>(std::forward<Handler>(completion));
                boost::asio::post(stream_.get_executor(), [this]() {
                    set_state(state_t::socket_connecting);
                });
            },
            handler);
    }

    void start(boost::string_view client_id, boost::string_view username, boost::string_view password) {
        async_run(client_id, username, password, [](boost::system::error_code) {
        });
    }

    void stop() {
        auto run_handler = std::move(run_handler_);
        if (run_handler) {
            boost::asio::post(stream_.get_executor(), [h = std::move(run_handler)]() {
                (*h)(boost::system::errc::make_error_code(boost::system::errc::operation_canceled));
            });
        }
    }
};
}// namespace details
}// namespace v311
}// namespace purple