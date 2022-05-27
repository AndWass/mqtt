//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/asio/coroutine.hpp>
#include <boost/asio/error.hpp>
#include <boost/weak_ptr.hpp>

namespace purple {
namespace v311 {
namespace details {
template<class ClientImpl>
struct publish_op_qos0 {
    boost::weak_ptr<ClientImpl> client_;
    std::unique_ptr<uint8_t[]> buffer_;
    size_t buffer_size_;
    boost::asio::coroutine coro_;
    uint8_t first_byte_;

    template<class Self>
    void operator()(Self &self, boost::system::error_code ec = {}, size_t /* n */ = {}) {
        auto client = client_.lock();
        if (!client) {
            self.complete(boost::asio::error::make_error_code(boost::asio::error::operation_aborted));
            return;
        }
        BOOST_ASIO_CORO_REENTER(coro_) {
            if (!client->try_acquire_write_lock()) {
                BOOST_ASIO_CORO_YIELD client->waiting_publishes_.emplace_back(std::move(self));
                if (ec.failed()) {
                    goto done;
                }
            }
            // We now have write lock here!
            BOOST_ASIO_CORO_YIELD client->stream_.async_write(
                first_byte_, boost::asio::buffer(buffer_.get(), buffer_size_), std::move(self));
            client->release_write_lock();

        done:
            self.complete(ec);
        }
    }
};
}// namespace details
}// namespace v311
}// namespace purple