//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/utility/string_view.hpp>

#include <string>

namespace purple {
namespace tcp {
namespace details {
struct resolving_socket_impl {
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::resolver resolver_;
    std::string host_;
    std::string service_;

    using executor_type = boost::asio::ip::tcp::socket::executor_type;

    resolving_socket_impl(const executor_type &exec, boost::string_view host, boost::string_view service)
        : socket_(exec), resolver_(exec), host_(host), service_(service) {
    }
};
}// namespace details
}// namespace tcp
}// namespace purple