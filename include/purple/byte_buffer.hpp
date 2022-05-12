//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/beast/core/flat_buffer.hpp>
#include <memory>

namespace purple {
template<class Allocator>
class basic_byte_buffer : public boost::beast::basic_flat_buffer<Allocator> {
public:
    using value_type = typename Allocator::value_type;
    explicit basic_byte_buffer(size_t max_size) : boost::beast::basic_flat_buffer<Allocator>(max_size) {
        static_assert(sizeof(value_type) == 1, "value_type must have size 1");
    }

    const value_type &operator[](size_t index) const {
        return *(static_cast<const value_type *>(this->data().data()) + index);
    }

    const value_type &at(size_t index) const {
        if (index >= this->size()) {
            throw std::out_of_range("Index out of range");
        }
        return *(static_cast<const value_type *>(this->data().data()) + index);
    }

    const value_type *begin() const {
        return static_cast<const value_type *>(this->data().data());
    }

    const value_type *end() const {
        return static_cast<const value_type *>(this->data().data()) + this->size();
    }
};

using byte_buffer = basic_byte_buffer<std::allocator<unsigned char>>;
}// namespace purple