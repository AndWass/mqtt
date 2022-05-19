//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "details/resolve_connect.hpp"
#include "details/resolving_socket_impl.hpp"

#include <boost/asio/async_result.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/utility/string_view.hpp>

namespace purple {
namespace tcp {
class resolving_socket {
    details::resolving_socket_impl inner_;

public:
    using executor_type = boost::asio::ip::tcp::socket::executor_type;
    resolving_socket(const executor_type &exec, boost::string_view host, boost::string_view service)
        : inner_(exec, host, service) {
    }

    executor_type get_executor() {
        return inner_.socket_.get_executor();
    }

    const boost::asio::ip::tcp::socket& next_layer() const {
        return inner_.socket_;
    }

    boost::asio::ip::tcp::socket& next_layer() {
        return inner_.socket_;
    }

    template<class MutableBufferSequence, class Handler>
    auto async_read_some(const MutableBufferSequence &buffers, Handler &&handler) {
        return inner_.socket_.template async_read_some(buffers, std::forward<Handler>(handler));
    }

    template<class ConstBufferSequence, class Handler>
    auto async_write_some(const ConstBufferSequence &buffers, Handler &&handler) {
        return inner_.socket_.template async_write_some(buffers, std::forward<Handler>(handler));
    }

    template<class Handler>
    typename boost::asio::async_result<std::decay_t<Handler>, void(boost::system::error_code)>::return_type
    async_connect(Handler &&handler) {
        return boost::asio::async_compose<Handler, void(boost::system::error_code)>(
            purple::tcp::details::resolve_connect_op{inner_, {}, {}}, handler, inner_.socket_, inner_.resolver_);
    }
};
}// namespace tcp
}// namespace purple