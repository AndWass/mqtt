#pragma once

#include <cstdint>
#include <stdexcept>

namespace mqtt
{
namespace details
{
namespace stream
{
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
}
}
}