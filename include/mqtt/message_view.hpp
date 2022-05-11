//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/core/span.hpp>
#include <cstdint>
#include <mqtt/fixed_header.hpp>

namespace purple {
struct message_view {
    fixed_header header;
    boost::span<const uint8_t> payload{};
};
}// namespace purple
