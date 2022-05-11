//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/asio/coroutine.hpp>
#include <boost/system/error_code.hpp>

#include <boost/asio/write.hpp>

namespace mqtt {
namespace details {
namespace stream {

struct no_buffer_tag {};

template<class AsyncStream, class ConstBufferSequence, class Handler, class = void>
struct use_asio_write : std::false_type {};

template<class AsyncStream, class ConstBufferSequence, class Handler>
struct use_asio_write<
    AsyncStream, ConstBufferSequence, Handler,
    std::void_t<decltype(boost::asio::async_write(
        std::declval<AsyncStream &>(), std::declval<const ConstBufferSequence &>(), std::declval<Handler &&>()))>>
    : std::true_type {};

template<bool>
struct async_write_impl {
    template<class AS, class CBS, class Handler>
    static auto async_write(AS &as, const CBS &cbs, Handler &&h) {
        return as.async_write(cbs, std::forward<Handler>(h));
    }
};

template<>
struct async_write_impl<true> {
    template<class AS, class CBS, class Handler>
    static auto async_write(AS &as, const CBS &cbs, Handler &&h) {
        return boost::asio::async_write(as, cbs, std::forward<Handler>(h));
    }
};

template<class AsyncStream, class ConstBufferSequence, class Handler>
auto async_write(AsyncStream &stream, const ConstBufferSequence &buffer, Handler &&handler) {
    async_write_impl<use_asio_write<AsyncStream, ConstBufferSequence, Handler>::value>::async_write(
        stream, buffer, std::forward<Handler>(handler));
}

template<class AsyncWrite, class ConstBufferSequence>
struct write_op {
    AsyncWrite &stream_;
    boost::asio::const_buffer header_;
    ConstBufferSequence buffer_;
    boost::asio::coroutine coro_;

    template<class Self>
    void operator()(Self &self, boost::system::error_code ec = {}, size_t n = 0) {
        namespace asio = boost::asio;
        BOOST_ASIO_CORO_REENTER(coro_) {
            BOOST_ASIO_CORO_YIELD mqtt::details::stream::async_write(stream_, header_, std::move(self));
            if (ec) {
                self.complete(ec, n);
            } else {
                BOOST_ASIO_CORO_YIELD mqtt::details::stream::async_write(stream_, buffer_, std::move(self));
                self.complete(ec, n + header_.size());
            }
        }
    }
};

template<class AsyncWrite>
struct write_op<AsyncWrite, no_buffer_tag> {
    const uint8_t *fixed_header_{};
    uint8_t fixed_header_len_{};
    AsyncWrite &stream_;
    boost::asio::coroutine coro_;

    template<class Self>
    void operator()(Self &self, boost::system::error_code ec = {}, size_t n = 0) {
        namespace asio = boost::asio;
        BOOST_ASIO_CORO_REENTER(coro_) {
            BOOST_ASIO_CORO_YIELD mqtt::details::stream::async_write(
                stream_, asio::buffer(fixed_header_, fixed_header_len_), std::move(self));
            self.complete(ec, n);
        }
    }
};
}// namespace stream
}// namespace details
}// namespace mqtt