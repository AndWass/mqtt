//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

/** @file */

#pragma once

#include <purple/binary.hpp>

#include "details/client_impl.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

namespace purple {
namespace v311 {

/* tag::reference[]
[#purple_v311_client]
= `purple::v311::client`

 */ // end::reference[]
template<class AsyncDefaultConnectableStream>
class client {
    using inner_t = details::client_impl<AsyncDefaultConnectableStream>;
    boost::shared_ptr<inner_t> inner_;

public:
    using executor_type = typename purple::stream<AsyncDefaultConnectableStream>::executor_type;
    using next_layer_type = typename purple::stream<AsyncDefaultConnectableStream>::next_layer_type;

    template<class... Args>
    explicit client(Args &&...args);

    executor_type get_executor();

    next_layer_type &next_layer();

    next_layer_type const &next_layer() const;

    void set_keep_alive(std::chrono::seconds keep_alive);

    template<class Handler>
    purple::async_result_t<Handler, boost::system::error_code>
    async_run(boost::string_view client_id, boost::string_view username, boost::string_view password,
              Handler &&handler);

    template<class Handler = boost::asio::default_completion_token_t<executor_type>>
    purple::async_result_t<Handler, boost::system::error_code>
    async_publish(boost::string_view topic, purple::qos quality_of_service,
                  purple::binary_view payload,
                  Handler&& handler = boost::asio::default_completion_token_t<executor_type>{})
    {
        return inner_->async_publish(topic, quality_of_service, false, payload, std::forward<Handler>(handler));
    }


    void set_connection_event_callback(std::function<void(boost::system::error_code, connection_event)> callback);

    void set_connect_decorator(std::function<void(connect_message &)> decorator);

    void set_message_callback(std::function<void(fixed_header)> callback);

    void start(boost::string_view client_id, boost::string_view username, boost::string_view password);

    void stop();
};

/* tag::reference[]
== Constructors
[source]
----
template<class... Args>
client(Args &&...args);
----

Creates a client and initializes the internal <<purple_stream>> object with `args...`.

*/ // end::reference[]
template<class Stream>
template<class... Args>
client<Stream>::client(Args &&...args)
    : inner_(boost::make_shared<details::client_impl<Stream>>(std::forward<Args>(args)...)) {
}

/* tag::reference[]
== Constructors
[source]
----
template<class... Args>
client(Args &&...args);
----

Creates a client and initializes the internal <<purple_stream>> object with `args...`.

*/ // end::reference[]
template<class Stream>
inline typename client<Stream>::executor_type client<Stream>::get_executor() {
    return inner_->stream_.get_executor();
}
template<class Stream>
inline typename client<Stream>::next_layer_type &client<Stream>::next_layer() {
    return inner_->stream_.next_layer();
}
template<class Stream>
inline typename client<Stream>::next_layer_type const &client<Stream>::next_layer() const {
    return inner_->stream_.next_layer();
}

template<class Stream>
inline void client<Stream>::set_keep_alive(std::chrono::seconds keep_alive) {
    inner_->keep_alive_ = keep_alive;
}

template<class Stream>
template<class Handler>
typename boost::asio::async_result<std::decay_t<Handler>, void(system::error_code)>::return_type
client<Stream>::async_run(boost::string_view client_id, boost::string_view username, boost::string_view password,
                          Handler &&handler) {
    return inner_->async_run(client_id, username, password, std::forward<Handler>(handler));
}

template<class Stream>
inline void client<Stream>::set_connection_event_callback(
    std::function<void(boost::system::error_code, connection_event)> callback) {
    inner_->connection_handler_ = std::move(callback);
}

template<class Stream>
inline void client<Stream>::set_connect_decorator(std::function<void(connect_message &)> decorator) {
    inner_->connect_decorator_ = std::move(decorator);
}

template<class Stream>
inline void client<Stream>::set_message_callback(std::function<void(fixed_header)> callback) {
    inner_->message_callback_ = std::move(callback);
}

template<class Stream>
inline void client<Stream>::start(boost::string_view client_id, boost::string_view username,
                                  boost::string_view password) {
    inner_->start(client_id, username, password);
}

template<class Stream>
inline void client<Stream>::stop() {
    inner_->stop();
}

}// namespace v311
}// namespace purple