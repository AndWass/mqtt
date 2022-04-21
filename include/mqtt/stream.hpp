#pragma once

#include <mqtt/fixed_header.hpp>

#include <boost/asio/async_result.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <boost/beast/core/buffer_traits.hpp>
#include <boost/beast/core/stream_traits.hpp>

#include <memory>

namespace mqtt {
namespace beast = boost::beast;
namespace asio = boost::asio;
namespace system = boost::system;

template<class CompletionToken, class... Results>
using async_result_t = typename asio::async_result<std::decay_t<CompletionToken>, void(Results...)>::return_type;

namespace details::stream {

inline size_t num_varlen_int_bytes(uint32_t value) {
    if (value < 128) {
        return 1;
    } else if (value < 16384) {
        return 2;
    } else if (value < 2'097'152) {
        return 3;
    } else if (value < 268'435'455) {
        return 4;
    }
    throw std::runtime_error("Value cannot be encoded as varlen integer");
}

inline void encode_varlen_int(uint32_t value, uint8_t *out) {
    do {
        *out = value % 128;
        value /= 128;
        if (value > 0) {
            *out |= 0x80;
        }
        ++out;
    } while (value > 0);
}

template<class AsyncWrite, class ConstBufferSequence>
struct write_op {
    std::unique_ptr<uint8_t[]> fixed_header_;
    uint8_t fixed_header_len_{};
    AsyncWrite &stream_;
    ConstBufferSequence buffer_;
    asio::coroutine coro_;

    template<class Self>
    void operator()(Self &self, system::error_code ec = {}, size_t n = 0) {
        BOOST_ASIO_CORO_REENTER(coro_) {
            BOOST_ASIO_CORO_YIELD asio::async_write(stream_, asio::buffer(fixed_header_.get(), fixed_header_len_), std::move(self));
            if (ec) {
                self.complete(ec, n);
            } else {
                BOOST_ASIO_CORO_YIELD asio::async_write(stream_, buffer_, std::move(self));
                self.complete(/*ec*/ system::error_code{}, n + fixed_header_len_);
            }
        }
    }
};

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
    fixed_header header_;

    template<class Self>
    void operator()(Self &self, system::error_code ec = {}, fixed_header header = {}) {
        BOOST_ASIO_CORO_REENTER(coro_) {
            BOOST_ASIO_CORO_YIELD stream_.async_read_fixed_header(buffer_, std::move(self));
            if (!ec) {
                header_ = header;
                buffer_.consume(buffer_.max_size());
                BOOST_ASIO_CORO_YIELD asio::async_read(stream_.next_layer(), buffer_.prepare(header_.remaining_length), std::move(self));
            }
            self.complete(ec, header_);
        }
    }

    template<class Self>
    void operator()(Self &self, system::error_code ec, size_t n) {
        buffer_.commit(n);
        (*this)(self, ec);
    }
};
}// namespace details::stream

template<class NextLayer>
class stream {
    NextLayer next_;

public:
    using executor_type = beast::executor_type<NextLayer>;
    using next_layer_type = NextLayer;

    template<class... Args>
    explicit stream(Args &&...args)
        : next_(std::forward<Args>(args)...) {
    }

    executor_type get_executor() {
        next_.get_executor();
    }

    next_layer_type &next_layer() {
        return next_;
    }

    const next_layer_type &next_layer() const {
        return next_;
    }

    template<class ConstBufferSequence, class WriteHandler>
    async_result_t<WriteHandler, system::error_code, size_t> async_write(uint8_t first_byte, const ConstBufferSequence &buffer, WriteHandler &&handler) {
        auto buffer_len = beast::buffer_bytes(buffer);
        const size_t fixed_header_len = 1 + details::stream::num_varlen_int_bytes(buffer_len);
        auto fixed_header = std::make_unique<uint8_t[]>(fixed_header_len);
        fixed_header[0] = first_byte;
        details::stream::encode_varlen_int(buffer_len, fixed_header.get() + 1);
        return asio::async_compose<WriteHandler, void(system::error_code, size_t)>(
            details::stream::write_op<NextLayer, ConstBufferSequence>{
                std::move(fixed_header),
                static_cast<uint8_t>(fixed_header_len),
                next_,
                buffer,
                {}},
            handler, next_);
    }

    template<class DynamicBuffer, class ReadHandler>
    async_result_t<ReadHandler, system::error_code, fixed_header> async_read_fixed_header(DynamicBuffer &buffer, ReadHandler &&handler) {
        return asio::async_compose<ReadHandler, void(system::error_code, fixed_header)>(
            details::stream::read_fixed_header_op<NextLayer, DynamicBuffer>{
                next_,
                buffer,
                {}},
            handler, next_);
    }

    template<class DynamicBuffer, class ReadHandler>
    async_result_t<ReadHandler, system::error_code, fixed_header> async_read(DynamicBuffer &buffer, ReadHandler &&handler) {
        return asio::async_compose<ReadHandler, void(system::error_code, fixed_header)>(
            details::stream::read_op<stream, DynamicBuffer>{
                *this,
                buffer,
                {}},
            handler, next_);
    }
};
}// namespace mqtt