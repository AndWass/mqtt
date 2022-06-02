//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "resolving_socket_impl.hpp"

#include <memory>

#include <boost/asio/connect.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/beast/core/bind_handler.hpp>

namespace purple {
namespace tcp {
namespace details {
struct resolve_connect_op {
    resolving_socket_impl &inner_;
    boost::asio::ip::tcp::resolver::results_type resolve_results_;
    boost::asio::coroutine coro_;

    template<class Self>
    void operator()(Self &self, boost::system::error_code ec = {}) {
        BOOST_ASIO_CORO_REENTER(coro_) {
            BOOST_ASIO_CORO_YIELD inner_.resolver_.async_resolve(inner_.host_, inner_.service_, std::move(self));
            if (!ec) {
                BOOST_ASIO_CORO_YIELD boost::asio::async_connect(inner_.socket_, resolve_results_, std::move(self));
            }

            self.complete(ec);
        }
    }

    template<class Self>
    void operator()(Self &self, boost::system::error_code ec, boost::asio::ip::tcp::resolver::results_type results) {
        resolve_results_ = std::move(results);
        (*this)(self, ec);
    }

    template<class Self>
    void operator()(Self &self, const boost::system::error_code &ec, const boost::asio::ip::tcp::endpoint &ep) {
        (void)ep;
        (*this)(self, ec);
    }
};
}// namespace details
}// namespace tcp
}// namespace purple
