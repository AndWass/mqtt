//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdint>

#include <boost/asio/buffer.hpp>
#include <boost/container/vector.hpp>
#include <boost/core/span.hpp>

namespace mqtt {
namespace details {
namespace stream {
struct read_buffer {
private:
    std::unique_ptr<uint8_t[]> storage_;
    std::size_t ready_to_read_ = 0;
    std::size_t capacity_;
    uint8_t *out() {
        return storage_.get() + ready_to_read_;
    }
    uint8_t *end() {
        return storage_.get() + capacity_;
    }

public:
    using const_buffers_type = boost::asio::const_buffer;
    using mutable_buffers_type = boost::asio::mutable_buffer;

    explicit read_buffer(std::size_t capacity) : storage_(std::make_unique<uint8_t[]>(capacity)), capacity_(capacity) {
    }

    boost::asio::mutable_buffer mutable_buffer() {
        if (ready_to_read_ == capacity_) {
            throw std::length_error("Cannot grow the read buffer!");
        }
        return boost::asio::mutable_buffer(out(), capacity_ - ready_to_read_);
    }

    [[nodiscard]] boost::span<const uint8_t> readable_area() const {
        return boost::span<const uint8_t>(storage_.get(), ready_to_read_);
    }

    [[nodiscard]] boost::asio::const_buffer const_buffer() const {
        return boost::asio::const_buffer(storage_.get(), ready_to_read_);
    }

    void consume(size_t n) {
        if (n >= ready_to_read_) {
            memmove(storage_.get(), out(), end() - out());
            ready_to_read_ = 0;
        } else {
            const uint8_t *source = storage_.get() + n;
            auto num_to_copy = end() - source;
            memmove(storage_.get(), source, num_to_copy);
            ready_to_read_ -= n;
        }
    }

    void commit(size_t n) {
        ready_to_read_ += n;
    }

    void reset() {
        ready_to_read_ = 0;
    }

    [[nodiscard]] size_t size() const {
        return ready_to_read_;
    }

    [[nodiscard]] size_t capacity() const {
        return capacity_;
    }

    [[nodiscard]] size_t writable_capacity() const {
        return capacity_ - ready_to_read_;
    }

    [[nodiscard]] size_t max_size() const {
        return capacity();
    }
};
}// namespace stream
}// namespace details
}// namespace mqtt