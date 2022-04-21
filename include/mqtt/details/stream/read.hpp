#pragma once

#include <boost/asio/read.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/system/error_code.hpp>
#include <cstdint>

#include <mqtt/fixed_header.hpp>
#include <mqtt/message_view.hpp>

namespace mqtt {
namespace details {
namespace stream {
namespace asio = boost::asio;
namespace system = boost::system;
template<class AsyncRead, class DynamicBuffer>
struct read_fixed_header_op {
    AsyncRead &stream_;
    DynamicBuffer buffer_;
    asio::coroutine coro_;

    [[nodiscard]] bool need_more_data() const {
        auto *last = static_cast<const uint8_t *>(buffer_.data().data()) + buffer_.data().size() - 1;
        return *last >= 128;
    }

    [[nodiscard]] fixed_header get_fixed_header() const {
        auto data = buffer_.data();
        fixed_header retval;
        if (data.size() != 0) {
            auto *bytes = static_cast<const uint8_t *>(data.data());
            const uint8_t *end = bytes + data.size();
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
        buffer_.commit(n);
        BOOST_ASIO_CORO_REENTER(coro_) {
            BOOST_ASIO_CORO_YIELD asio::async_read(stream_, buffer_.prepare(2), std::move(self));
            if (!ec) {
                while (need_more_data()) {
                    if (buffer_.data().size() == 5) {
                        ec = system::errc::make_error_code(system::errc::protocol_error);
                        break;
                    }
                    BOOST_ASIO_CORO_YIELD asio::async_read(stream_, buffer_.prepare(1), std::move(self));
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
    message_view mv_;

    template<class Self>
    void operator()(Self &self, system::error_code ec = {}, fixed_header header = {}, boost::span<const uint8_t> payload = {}) {
        BOOST_ASIO_CORO_REENTER(coro_) {
            BOOST_ASIO_CORO_YIELD stream_.async_read_fixed_header(buffer_, std::move(self));
            mv_.header = header;
            if (!ec) {
                buffer_.consume(buffer_.max_size());
                BOOST_ASIO_CORO_YIELD asio::async_read(stream_.next_layer(), buffer_.prepare(header.remaining_length), std::move(self));
                mv_.payload = payload;
            }
            self.complete(ec, mv_);
        }
    }

    template<class Self>
    void operator()(Self &self, system::error_code ec, size_t n) {
        buffer_.commit(n);
        boost::span<const uint8_t> payload(static_cast<const uint8_t*>(buffer_.data().data()), n);
        (*this)(self, ec, {}, payload);
    }
};
}
}// namespace details
}// namespace mqtt