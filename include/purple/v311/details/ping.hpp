//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/asio/coroutine.hpp>
#include <boost/asio/error.hpp>
#include <boost/weak_ptr.hpp>

#include <purple/packet_type.hpp>

namespace purple {
namespace v311 {
namespace details {
template<class ClientImpl>
struct ping_op {
    boost::weak_ptr<ClientImpl> client_;
    boost::asio::coroutine coro_;

    template<class Self>
    void operator()(Self &self, boost::system::error_code ec = {}, size_t /* n */ = {}) {
        if (ec.failed()) {
            self.complete(ec);
            return;
        }
        auto client = client_.lock();
        if (!client) {
            self.complete(purple::make_error_code(purple::error::client_aborted));
            return;
        }

        BOOST_ASIO_CORO_REENTER(coro_) {
            if (!client->try_acquire_write_lock()) {
                BOOST_ASIO_CORO_YIELD client->waiting_ping_.set(std::move(self));
                if (ec.failed()) {
                    goto done;
                }
            }
            // We now have write lock here!
            BOOST_ASIO_CORO_YIELD client->stream_.async_write(purple::packet_type::pingreq, boost::asio::const_buffer{},
                                                              std::move(self));
            client->release_write_lock();

        done:
            self.complete(ec);
        }
    }
};
}// namespace details
}// namespace v311
}// namespace purple