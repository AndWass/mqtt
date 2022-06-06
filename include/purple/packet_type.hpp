//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdint>

namespace purple {
namespace packet_type {
static constexpr uint8_t connect = 0x10, connack = 0x20, publish = 0x30, puback = 0x40, pubrec = 0x50, pubrel = 0x60,
                         pubcomp = 0x70, subscribe = 0x80, suback = 0x90, unsubscribe = 0xa0, unsuback = 0xb0,
                         pingreq = 0xc0, pingresp = 0xd0, disconnect = 0xe0;
}
}// namespace purple