//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <mqtt/fixed_header.hpp>
#include <mqtt/message_view.hpp>

#include "details/stream/read.hpp"
#include "details/stream/read_buffer.hpp"
#include "details/stream/varlen.hpp"
#include "details/stream/write.hpp"

#include <boost/asio/async_result.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <boost/beast/core/buffer_traits.hpp>
#include <boost/beast/core/stream_traits.hpp>

#include <array>
#include <vector>

namespace mqtt {
namespace beast = boost::beast;
namespace asio = boost::asio;
namespace system = boost::system;

template<class CompletionToken, class... Results>
using async_result_t = typename asio::async_result<std::decay_t<CompletionToken>, void(Results...)>::return_type;

template<class NextLayer>
class stream {
    NextLayer next_;
    mqtt::details::stream::read_buffer read_buffer_;
    std::array<uint8_t, 5> fixed_header_write_buffer_{};

public:
    using executor_type = beast::executor_type<NextLayer>;
    using next_layer_type = NextLayer;

    template<class... Args>
    explicit stream(std::size_t read_buffer_size, Args &&...args)
        : read_buffer_(read_buffer_size), next_(std::forward<Args>(args)...) {
        if (read_buffer_size < 5) {
            throw std::length_error("The internal read buffer must be at least 5 bytes long");
        }
    }

    template<class... Args>
    explicit stream(Args &&...args)
        : stream(size_t(1024), std::forward<Args>(args)...) {
    }

    executor_type get_executor() {
        return next_.get_executor();
    }

    next_layer_type &next_layer() {
        return next_;
    }

    const next_layer_type &next_layer() const {
        return next_;
    }

    template<class DynamicBuffer, class ReadHandler>
    async_result_t<ReadHandler, system::error_code, fixed_header> async_read(DynamicBuffer &buffer, ReadHandler &&handler) {
        return asio::async_compose<ReadHandler, void(system::error_code, fixed_header)>(
            details::stream::read_op<NextLayer, DynamicBuffer>{
                next_,
                read_buffer_,
                buffer,
                {}},
            handler, next_);
    }

    template<class ConstBufferSequence, class WriteHandler>
    async_result_t<WriteHandler, system::error_code, size_t> async_write(uint8_t first_byte, const ConstBufferSequence &buffer, WriteHandler &&handler) {
        const size_t buffer_len = beast::buffer_bytes(buffer);
        const size_t fixed_header_len = 1 + details::stream::num_varlen_int_bytes(buffer_len);
        fixed_header_write_buffer_[0] = first_byte;
        details::stream::encode_varlen_int(buffer_len, fixed_header_write_buffer_.data() + 1);
        return asio::async_compose<WriteHandler, void(system::error_code, size_t)>(
            details::stream::write_op<NextLayer, ConstBufferSequence>{
                fixed_header_write_buffer_.data(),
                static_cast<uint8_t>(fixed_header_len),
                next_,
                buffer,
                {}},
            handler, next_);
    }

    void reset() {
        read_buffer_.reset();
    }
};
}// namespace mqtt