//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "handshake.hpp"
#include "publish.hpp"
#include "subscribe.hpp"

#include <purple/binary.hpp>
#include <purple/connection_event.hpp>
#include <purple/stream.hpp>

#include <purple/v311/connect_message.hpp>

#include <boost/asio/executor.hpp>
#include <boost/asio/steady_timer.hpp>

#include <boost/beast/core/flat_buffer.hpp>

#include <boost/assert.hpp>
#include <boost/container/vector.hpp>
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
    class erased_handler {
        struct concept_t {
            virtual ~concept_t() = default;
            virtual void operator()(Args... args) = 0;
        };

        template<class Handler>
        struct impl_t : public concept_t {
            Handler h_;

            template<class H2, std::enable_if_t<!std::is_same_v<std::decay_t<H2>, impl_t<Handler>>> * = nullptr>
            explicit impl_t(H2 &&h) : h_(std::forward<H2>(h)) {
            }

            void operator()(Args... args) override {
                h_(std::move(args)...);
            }
        };

        std::unique_ptr<concept_t> impl_;

    public:
        erased_handler() = default;

        template<class Handler, std::enable_if_t<!std::is_same_v<std::decay_t<Handler>, erased_handler>> * = nullptr>
        explicit erased_handler(Handler &&h)
            : impl_(std::make_unique<impl_t<std::decay_t<Handler>>>(std::forward<Handler>(h))) {
        }

        void operator()(Args... args) {
            (*impl_)(std::move(args)...);
        }

        bool holds_handler() const {
            return impl_ != nullptr;
        }
    };

    enum class state_t {
        idle,
        socket_connecting,
        handshaking,
        connected,
        disconnected,
    };

    struct waiting_suback {
        uint16_t packet_id;
        erased_handler<boost::system::error_code> handler;

        waiting_suback(uint16_t pid, erased_handler<boost::system::error_code> h)
            : packet_id(pid), handler(std::move(h)) {
        }
    };

    purple::stream<AsyncDefaultConnectableStream> stream_;
    boost::beast::flat_buffer read_buffer_;
    boost::container::vector<uint8_t> write_buffer_;

    boost::asio::steady_timer ping_timer_;
    boost::asio::steady_timer connection_timeout_timer_;

    boost::container::vector<erased_handler<boost::system::error_code>> waiting_publishes_;

    erased_handler<boost::system::error_code> run_handler_;
    std::function<void(boost::system::error_code, connection_event)> connection_handler_;
    std::function<void(connect_message &)> connect_decorator_;
    std::function<void(purple::fixed_header)> message_callback_;

    state_t state_ = state_t::idle;

    std::chrono::seconds keep_alive_{};

    connect_message connect_;

    uint16_t packet_identifier_ = 0;

    bool write_lock_;

    bool try_acquire_write_lock() {
        if (state_ != state_t::connected) {
            return false;
        }
        if (!write_lock_) {
            write_lock_ = true;
            return true;
        }
        return false;
    }

    void release_write_lock() {
        // TODO
    }

    void close() {
        boost::beast::close_socket(boost::beast::get_lowest_layer(stream_.next_layer()));
        set_state(boost::system::errc::make_error_code(boost::system::errc::operation_canceled), state_t::disconnected);
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
                self->set_state(ec, state_t::connected);
            } else if (self) {
                self->close();
            }
        };
    }

    static auto subscribe_handler(boost::weak_ptr<Self> weak) {
        return [weak](boost::system::error_code ec, size_t) {
            if (ec) {
                auto self = weak.lock();
                if (self) {
                }
            }
        };
    }

    static auto socket_connection_handler(boost::weak_ptr<Self> weak) {
        return [weak](boost::system::error_code ec) {
            auto self = weak.lock();
            if (self && !ec) {
                self->set_state(ec, state_t::handshaking);
            } else {
                self->set_state(ec, state_t::disconnected);
            }
        };
    }

    void finish_all_pending(boost::system::error_code ec) {
        boost::asio::post(stream_.get_executor(), [h = std::move(run_handler_), pubs = std::move(waiting_publishes_), ec]() mutable {
            for(auto&& pub: pubs) {
                pub(ec);
            }
            if (h.holds_handler()) {
                h(ec);
            }
        });
    }

    void set_state(boost::system::error_code ec, state_t new_state) {
        if (state_ == state_t::idle && new_state == state_t::socket_connecting) {
            stream_.next_layer().async_connect(socket_connection_handler(this->weak_from_this()));
            report_connection_event(ec, connection_event::initiating_socket_connection);
        } else if (new_state == state_t::handshaking && state_ == state_t::socket_connecting) {
            connect_.keep_alive = keep_alive_;
            if (connect_decorator_) {
                connect_decorator_(connect_);
            }
            async_handshake(handshake_handler(this->weak_from_this()));
            start_connection_timeout_timer();
            report_connection_event(ec, connection_event::socket_connected);
        } else if (new_state == state_t::connected && state_ != state_t::connected) {
            start_read_one_message();
            start_connection_timeout_timer();
            start_ping_timer();
            report_connection_event(ec, connection_event::handshake_successful);
            if (!waiting_publishes_.empty()) {
                write_lock_ = true;
                auto front = std::move(waiting_publishes_.front());
                waiting_publishes_.erase(waiting_publishes_.begin());
                front(boost::system::error_code{});
            }
        } else if (new_state == state_t::disconnected && state_ != state_t::disconnected) {
            ping_timer_.cancel();
            connection_timeout_timer_.cancel();
            write_lock_ = false;
            report_connection_event(ec, connection_event::socket_disconnected);
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

    ~client_impl() {
        finish_all_pending(purple::make_error_code(purple::error::client_aborted));
    }

    template<class Handler>
    typename boost::asio::async_result<std::decay_t<Handler>, void(boost::system::error_code)>::return_type
    async_run(boost::string_view client_id, boost::string_view username, boost::string_view password,
              Handler &&handler) {
        BOOST_ASSERT(state_ == state_t::idle);

        connect_.client_id.assign(client_id.data(), client_id.size());
        connect_.username.assign(username.data(), username.size());
        if (!username.empty()) {
            connect_.password.assign(password.data(), password.size());
        } else {
            connect_.password = "";
        }

        return boost::asio::async_initiate<Handler, void(boost::system::error_code)>(
            [this, self = this](auto &&completion) {
                using handler_t = decltype(completion);
                run_handler_ = erased_handler<boost::system::error_code>(std::forward<handler_t>(completion));
                boost::asio::post(stream_.get_executor(), [weak = self->weak_from_this()]() {
                    if (auto self = weak.lock()) {
                        self->set_state({}, state_t::socket_connecting);
                    }
                });
            },
            handler);
    }

    template<class Handler>
    purple::async_result_t<Handler, boost::system::error_code>
    async_subscribe(boost::string_view topic, purple::qos quality_of_service, Handler &&handler) {
        throw std::runtime_error("Not implemented!");
    }

    template<class Handler>
    purple::async_result_t<Handler, boost::system::error_code>
    async_publish(boost::string_view topic, purple::qos quality_of_service, bool retain, purple::binary_view payload,
                  Handler &&handler) {
        if (quality_of_service != purple::qos::qos0) {
            throw std::runtime_error("Not implemented");
        }
        const size_t buffer_size = 2 + topic.size() + payload.size();
        uint8_t first_byte = 0x30 + (retain ? 1 : 0);
        std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(buffer_size);
        auto *ptr = purple::details::put_str(topic, buffer.get());
        memcpy(ptr, payload.data(), payload.size());
        return boost::asio::async_compose<Handler, void(boost::system::error_code)>(
            details::publish_op_qos0<Self>{this->weak_from_this(), std::move(buffer), buffer_size, {}, first_byte},
            handler, stream_);
    }

    void start(boost::string_view client_id, boost::string_view username, boost::string_view password) {
        async_run(client_id, username, password, [](boost::system::error_code) {
        });
    }

    void stop() {
        close();
        finish_all_pending(purple::make_error_code(purple::error::client_stopped));
    }
};
}// namespace details
}// namespace v311
}// namespace purple