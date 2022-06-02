#pragma once

#include <purple/error.hpp>
#include <purple/fixed_header.hpp>
#include <purple/stream.hpp>

#include <purple/packet_type.hpp>

#include <boost/asio/coroutine.hpp>

#include <memory>

namespace purple {
namespace v311 {
namespace details {

template<class NextLayer>
struct handshake_op {
    purple::stream<NextLayer> &stream_;
    boost::asio::mutable_buffer write_read_buffer;
    boost::asio::coroutine coro_;

    template<class Self>
    void operator()(Self &self, boost::system::error_code ec = {}, purple::fixed_header header = {}) {
        BOOST_ASIO_CORO_REENTER(coro_) {
            BOOST_ASIO_CORO_YIELD stream_.async_write(purple::packet_type::connect, write_read_buffer, std::move(self));
            if (!ec.failed()) {
                BOOST_ASIO_CORO_YIELD stream_.async_read(write_read_buffer, std::move(self));
                if (!ec) {
                    uint8_t response_code = *(static_cast<uint8_t *>(write_read_buffer.data()) + 1);
                    if (header.first_byte != purple::packet_type::connack || header.remaining_length != 2) {
                        ec = purple::make_error_code(error::invalid_connect_response);
                    } else if (response_code == 1) {
                        ec = purple::make_error_code(error::unacceptable_protocol_version);
                    } else if (response_code == 2) {
                        ec = purple::make_error_code(error::identifier_rejected);
                    } else if (response_code == 3) {
                        ec = purple::make_error_code(error::server_unavailable);
                    } else if (response_code == 4) {
                        ec = purple::make_error_code(error::bad_username_or_password);
                    } else if (response_code == 5) {
                        ec = purple::make_error_code(error::unauthorized);
                    }
                }
            }
            self.complete(ec, *static_cast<uint8_t *>(write_read_buffer.data()) == 1);
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