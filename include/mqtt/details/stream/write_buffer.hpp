//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memory>

namespace mqtt {
namespace details {
namespace stream {
class write_buffer {
    std::unique_ptr<uint8_t[]> buffer_;
    size_t capacity_;

public:
    explicit write_buffer(size_t capacity) : buffer_(std::make_unique<uint8_t[]>(capacity)), capacity_(capacity) {
    }

    [[nodiscard]] size_t capacity() const {
        return capacity_;
    }

    boost::asio::mutable_buffer mutable_buffer(size_t offset) {
        return boost::asio::mutable_buffer(buffer_.get() + offset, capacity_);
    }

    boost::asio::const_buffer const_buffer(size_t len) {
        return boost::asio::const_buffer(buffer_.get(), len);
    }

    uint8_t *data() {
        return buffer_.get();
    }
};
}// namespace stream
}// namespace details
}// namespace mqtt