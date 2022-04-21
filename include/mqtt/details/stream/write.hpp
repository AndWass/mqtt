#pragma once

#include <boost/asio/coroutine.hpp>
#include <boost/system/error_code.hpp>

#include <boost/asio/write.hpp>

namespace mqtt {
namespace details {
namespace stream {
template<class AsyncWrite, class ConstBufferSequence>
struct write_op {
    std::unique_ptr<uint8_t[]> fixed_header_;
    uint8_t fixed_header_len_{};
    AsyncWrite &stream_;
    ConstBufferSequence buffer_;
    boost::asio::coroutine coro_;

    template<class Self>
    void operator()(Self &self, boost::system::error_code ec = {}, size_t n = 0) {
        namespace asio = boost::asio;
        BOOST_ASIO_CORO_REENTER(coro_) {
            BOOST_ASIO_CORO_YIELD asio::async_write(stream_, asio::buffer(fixed_header_.get(), fixed_header_len_), std::move(self));
            if (ec) {
                self.complete(ec, n);
            } else {
                BOOST_ASIO_CORO_YIELD asio::async_write(stream_, buffer_, std::move(self));
                self.complete(ec, n + fixed_header_len_);
            }
        }
    }
};
}// namespace stream
}// namespace details
}// namespace mqtt