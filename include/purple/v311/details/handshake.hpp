#pragma once

#include <purple/error.hpp>
#include <purple/fixed_header.hpp>
#include <purple/stream.hpp>

#include <boost/asio/coroutine.hpp>

#include <memory>

namespace purple {
namespace v311 {
namespace details {

template<class NextLayer>
struct handshake_op {
    purple::stream<NextLayer> &stream_;
    std::unique_ptr<uint8_t[]> buffer_;
    size_t buffer_size{};
    boost::asio::coroutine coro_;

    template<class Self>
    void operator()(Self &self, boost::system::error_code ec = {}, purple::fixed_header header = {}) {
        BOOST_ASIO_CORO_REENTER(coro_) {
            BOOST_ASIO_CORO_YIELD stream_.async_write(0x10, boost::asio::buffer(buffer_.get(), buffer_size),
                                                      std::move(self));
            if (!ec.failed()) {
                BOOST_ASIO_CORO_YIELD stream_.async_read(boost::asio::buffer(buffer_.get(), buffer_size),
                                                         std::move(self));
                if (!ec) {
                    if (header.first_byte != 0x20 || header.remaining_length != 2) {
                        ec = purple::make_error_code(error::invalid_connect_response);
                    } else if (buffer_[1] == 1) {
                        ec = purple::make_error_code(error::unacceptable_protocol_version);
                    } else if (buffer_[1] == 2) {
                        ec = purple::make_error_code(error::identifier_rejected);
                    } else if (buffer_[1] == 3) {
                        ec = purple::make_error_code(error::server_unavailable);
                    } else if (buffer_[1] == 4) {
                        ec = purple::make_error_code(error::bad_username_or_password);
                    } else if (buffer_[1] == 5) {
                        ec = purple::make_error_code(error::unauthorized);
                    }
                }
            }
            self.complete(ec, buffer_[0] == 1);
        }
    }

    template<class Self>
    void operator()(Self &self, boost::system::error_code ec, size_t /*n*/) {
        (*this)(self, ec);
    }
};

}// namespace details
}// namespace v311
}// namespace purple