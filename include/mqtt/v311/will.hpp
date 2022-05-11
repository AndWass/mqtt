//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string>

#include <mqtt/binary.hpp>
#include <mqtt/qos.hpp>

namespace mqtt {
namespace v311 {
struct will_t {
    std::string topic;
    binary_t payload;
    mqtt::qos quality_of_service = mqtt::qos::qos1;
    bool retain = false;
};
}// namespace v311
}// namespace mqtt