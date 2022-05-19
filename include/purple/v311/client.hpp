//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

/** @file */

#pragma once

#include "details/client_impl.hpp"

#include <boost/shared_ptr.hpp>

namespace purple {
namespace v311 {

/**
 * @ingroup v311_client
 * @brief A high-level client for <a href="">MQTT Version 3.1.1</a>
 * @tparam AsyncDefaultConnectableStream
 */
template<class AsyncDefaultConnectableStream>
class client {

    boost::shared_ptr<details::client_impl<AsyncDefaultConnectableStream>> inner_;

public:
    using executor_type = typename purple::stream<AsyncDefaultConnectableStream>::executor_type;
    using next_layer_type = typename purple::stream<AsyncDefaultConnectableStream>::next_layer_type;

    template<class... Args>
    explicit client(Args &&...args): inner_(boost::make_shared<details::client_impl<AsyncDefaultConnectableStream>>(std::forward<Args>(args)...))  {
    }

    executor_type get_executor() {
        return inner_->stream_.get_executor();
    }

    next_layer_type &next_layer() {
        return inner_->stream_.next_layer();
    }

    const next_layer_type &next_layer() const {
        return inner_->stream_.next_layer();
    }

    void set_keep_alive(std::chrono::seconds keep_alive) {
        inner_->keep_alive_ = keep_alive;
    }

#ifndef DOXYGEN
    template<class Handler>
    typename boost::asio::async_result<std::decay_t<Handler>, void(boost::system::error_code)>::return_type
#else
    template<class Handler>
    DEDUCED
#endif
    async_run(boost::string_view client_id, boost::string_view username, boost::string_view password,
              Handler &&handler) {
        return inner_->async_run(client_id, username, password, std::forward<Handler>(handler));
    }

    /**
     * @brief Set a callback to be called when a connection-related event happens.
     * @param callback Callback to invoke.
     *
     * The callback must be invocable
     * with the equivalent signature of
     * @code
     * void(
     *      boost::system::error_code, // The error if any has occurred.
     *      connection_event           // The event that triggered the callback to be called.
     * )
     * @endcode
     *
     * The implementation type-erases the callback which may require a dynamic allocation.
     */
    void set_connection_event_callback(std::function<void(boost::system::error_code, connection_event)> callback) {
        inner_->connection_handler_ = std::move(callback);
    }

    void set_connect_decorator(std::function<void(connect_message &)> decorator) {
        inner_->connect_decorator_ = std::move(decorator);
    }

    void set_message_callback(std::function<void(fixed_header)> callback) {
        inner_->message_callback_ = std::move(callback);
    }

    void start(boost::string_view client_id, boost::string_view username, boost::string_view password) {
        inner_->start(client_id, username, password);
    }

    void stop() {
        inner_->stop();
    }
};
}// namespace v311
}// namespace purple