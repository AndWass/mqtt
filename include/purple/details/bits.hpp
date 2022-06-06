//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdint>
#include <stdexcept>

#include <cstring>

#include <boost/core/span.hpp>
#include <boost/utility/string_view.hpp>

namespace purple {
namespace details {
inline size_t num_varlen_int_bytes(size_t value) {
    if (value < 128) {
        return 1;
    } else if (value < 16384) {
        return 2;
    } else if (value < 2'097'152) {
        return 3;
    } else if (value < 268'435'456) {
        return 4;
    }
    throw std::length_error("Value cannot be encoded as varlen integer");
}

inline uint8_t *put_varlen_int(uint32_t value, uint8_t *out) {
    do {
        *out = value % 128;
        value /= 128;
        if (value > 0) {
            *out |= 0x80;
        }
        ++out;
    } while (value > 0);
    return out;
}

inline uint8_t *put_u16(uint16_t value, uint8_t *out) {
    *out++ = value >> 8;
    *out++ = value & 0x00FF;
    return out;
}

inline uint8_t *put_bin(boost::span<const uint8_t> value, uint8_t *out) {
    uint16_t len = static_cast<uint8_t>(value.size_bytes());
    out = put_u16(len, out);
    memcpy(out, value.data(), len);
    return out + len;
}

inline uint8_t *put_str(boost::string_view sv, uint8_t *out) {
    return put_bin({reinterpret_cast<const uint8_t *>(sv.data()), sv.size()}, out);
}

}// namespace details
}// namespace purple
