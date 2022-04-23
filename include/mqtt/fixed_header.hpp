//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdint>

namespace mqtt {
struct fixed_header {
    uint8_t first_byte{};
    uint32_t remaining_length{};
};
}// namespace mqtt