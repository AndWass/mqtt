//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/container/vector.hpp>
#include <boost/core/span.hpp>
#include <boost/utility/string_view.hpp>
#include <cstdint>

namespace purple {
using binary_t = boost::container::vector<uint8_t, void,
                                          boost::container::vector_options_t<boost::container::stored_size<uint16_t>>>;

class binary_view : public boost::span<const uint8_t> {
public:
    using base_type = boost::span<const uint8_t>;

    using base_type::span;
    binary_view(const char *str)// NOLINT(google-explicit-constructor)
        : boost::span<const uint8_t>(reinterpret_cast<const uint8_t *>(str), strlen(str)) {
    }
    binary_view(const char *str, size_t len) : boost::span<const uint8_t>(reinterpret_cast<const uint8_t *>(str), len) {
    }
    binary_view(boost::string_view str)// NOLINT(google-explicit-constructor)
        : boost::span<const uint8_t>(reinterpret_cast<const uint8_t *>(str.data()), str.size()) {
    }

    [[nodiscard]] binary_t to_owned() const {
        binary_t retval;
        if (size() > retval.max_size()) {
            throw std::length_error("binary_view too large to fit in an MQTT binary payload");
        }

        retval.insert(retval.begin(), begin(), end());
        return retval;
    }
};

inline void assign_string(binary_t &bin, boost::string_view str) {
    bin.reserve(str.size());
    bin.insert(bin.end(), str.begin(), str.end());
}
}// namespace purple