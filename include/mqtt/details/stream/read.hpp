#pragma once

#include <boost/asio/coroutine.hpp>
#include <boost/asio/read.hpp>
#include <boost/system/error_code.hpp>
#include <cstdint>

#include <mqtt/fixed_header.hpp>
#include <mqtt/message_view.hpp>

#include <vector>

namespace mqtt {
namespace details {
namespace stream {
namespace asio = boost::asio;
namespace system = boost::system;
template<class AsyncRead>
struct read_fixed_header_op {
    AsyncRead &stream_;
    std::vector<uint8_t> buffer_;
    asio::coroutine coro_;

    [[nodiscard]] bool need_more_data() const {
        return buffer_.back() >= 128;
    }

    [[nodiscard]] fixed_header get_fixed_header() const {
        fixed_header retval{};
        if (!buffer_.empty()) {
            auto *bytes = buffer_.data();
            auto *end = bytes + buffer_.size();
            retval.first_byte = *bytes++;
            uint32_t multiplier = 1;
            while (bytes != end) {
                retval.remaining_length += (*bytes & 0x7F) * multiplier;
                multiplier *= 128;
                bytes++;
            }
        }
        return retval;
    }

    template<class Self>
    void operator()(Self &self, system::error_code ec = {}, size_t n = 0) {
        BOOST_ASIO_CORO_REENTER(coro_) {
            buffer_.reserve(5);
            buffer_.resize(2, 0);
            BOOST_ASIO_CORO_YIELD asio::async_read(stream_, asio::buffer(buffer_), std::move(self));
            if (!ec) {
                while (need_more_data()) {
                    if (buffer_.size() == 5) {
                        ec = system::errc::make_error_code(system::errc::protocol_error);
                        break;
                    }
                    buffer_.push_back(0);
                    BOOST_ASIO_CORO_YIELD asio::async_read(stream_, asio::buffer(&buffer_.back(), 1), std::move(self));
                    if (ec) {
                        break;
                    }
                }
            }
            self.complete(ec, get_fixed_header());
        }
    }
};

template<class Mqtt, class DynamicBuffer>
struct read_op {
    Mqtt &stream_;
    DynamicBuffer &buffer_;
    asio::coroutine coro_;
    fixed_header header_;

    template<class Self>
    void operator()(Self &self, system::error_code ec = {}, fixed_header header = {}) {
        BOOST_ASIO_CORO_REENTER(coro_) {
            BOOST_ASIO_CORO_YIELD stream_.async_read_fixed_header(std::move(self));
            header_ = header;
            if (!ec) {
                BOOST_ASIO_CORO_YIELD asio::async_read(stream_.next_layer(), buffer_.prepare(header.remaining_length), std::move(self));
            }
            self.complete(ec, header_);
        }
    }

    template<class Self>
    void operator()(Self &self, system::error_code ec, size_t n) {
        buffer_.commit(n);
        (*this)(self, ec, header_);
    }
};
}
}// namespace details
}// namespace mqtt