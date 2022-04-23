//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <mqtt/qos.hpp>
#include <vector>
namespace mqtt {
struct will {
    std::string topic;
    std::vector<uint8_t> message;
    qos quality_of_service = qos::qos1;
    bool retain = false;
};
}