#pragma once

#include <cstdint>

namespace mqtt {
struct fixed_header {
    uint8_t first_byte{};
    uint32_t remaining_length{};
};
}// namespace mqtt