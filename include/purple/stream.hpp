//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <purple/async_result.hpp>
#include <purple/fixed_header.hpp>

#include "details/bits.hpp"

#include "details/stream/read.hpp"
#include "details/stream/read_buffer.hpp"
#include "details/stream/write.hpp"
#include "details/stream/write_buffer.hpp"

#include <boost/asio/async_result.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <boost/beast/core/buffer_traits.hpp>
#include <boost/beast/core/stream_traits.hpp>

#include <array>
#include <vector>

namespace purple {
namespace beast = boost::beast;
namespace asio = boost::asio;
namespace system = boost::system;

/* tag::reference[]
[#purple_stream]
= `purple::stream`

`purple::stream` is a low level stream to read and write MQTT messages on
the binary format specified by
the http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718018[MQTT standard].

It can be used to implement custom MQTT clients or server, but for most client use-cases one should prefer
to use <<purple_v311_client>> instead.

No validation of the validity of messages is done, it simply makes sure
to send data using the correct wire format.

 */ // end::reference[]
template<class NextLayer>
class stream {
    NextLayer next_;
    purple::details::stream::read_buffer read_buffer_;
    purple::details::stream::write_buffer write_buffer_;

public:
    using executor_type = beast::executor_type<NextLayer>;
    using next_layer_type = NextLayer;

    template<class... Args>
    stream(std::size_t read_buffer_size, std::size_t write_buffer_size, Args &&...args)
        : next_(std::forward<Args>(args)...), read_buffer_(read_buffer_size), write_buffer_(write_buffer_size) {
        if (read_buffer_size < 5) {
            throw std::length_error("The internal read buffer must be at least 5 bytes long");
        }

        if (write_buffer_size < 5) {
            throw std::length_error("The internal write buffer must be at least 5 bytes long");
        }
    }

    template<class... Args>
    explicit stream(Args &&...args) : stream(size_t(1024), size_t(1024), std::forward<Args>(args)...) {
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

    template<class ReadHandler>
    async_result_t<ReadHandler, system::error_code, fixed_header> async_read(const boost::asio::mutable_buffer &buffer,
                                                                             ReadHandler &&handler) {
        return asio::async_compose<ReadHandler, void(system::error_code, fixed_header)>(
            details::stream::read_into_fixed_op<NextLayer>{next_, read_buffer_, buffer, {}}, handler, next_);
    }

    template<class DynamicBuffer, class ReadHandler,
             std::enable_if_t<boost::asio::is_dynamic_buffer<DynamicBuffer>::value> * = nullptr>
    async_result_t<ReadHandler, system::error_code, fixed_header> async_read(DynamicBuffer &buffer,
                                                                             ReadHandler &&handler) {
        return asio::async_compose<ReadHandler, void(system::error_code, fixed_header)>(
            details::stream::read_op<NextLayer, DynamicBuffer>(next_, read_buffer_, buffer), handler, next_);
    }

    template<class ConstBufferSequence, class WriteHandler>
    async_result_t<WriteHandler, system::error_code, size_t>
    async_write(uint8_t first_byte, const ConstBufferSequence &buffer, WriteHandler &&handler) {
        const size_t buffer_len = beast::buffer_bytes(buffer);
        const size_t fixed_header_len = 1 + details::num_varlen_int_bytes(buffer_len);

        uint8_t *data = write_buffer_.data();
        data[0] = first_byte;
        details::put_varlen_int(buffer_len, data + 1);

        if (fixed_header_len + buffer_len < write_buffer_.capacity()) {
            boost::asio::buffer_copy(write_buffer_.mutable_buffer(fixed_header_len), buffer);
            return asio::async_write(next_, write_buffer_.const_buffer(fixed_header_len + buffer_len),
                                     std::forward<WriteHandler>(handler));
        }

        return asio::async_compose<WriteHandler, void(system::error_code, size_t)>(
            details::stream::write_op<NextLayer, ConstBufferSequence>{next_,
                                                                      write_buffer_.const_buffer(fixed_header_len),
                                                                      buffer,
                                                                      {}},
            handler, next_);
    }

    void reset() {
        read_buffer_.reset();
    }
};
}// namespace purple