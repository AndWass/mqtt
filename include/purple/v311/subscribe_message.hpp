//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <purple/qos.hpp>

#include <boost/container/vector.hpp>
#include <boost/utility/string_view.hpp>

#include <cstdint>
#include <string>

namespace purple {
namespace v311 {
struct subscribe_message {
    struct subscription {
        std::string topic;
        purple::qos quality_of_service;

        subscription() = default;
        subscription(boost::string_view topic, purple::qos qos_) : topic(topic), quality_of_service(qos_) {
        }
    };
    uint16_t packet_identifier;
    boost::container::vector<subscription> subscriptions;
};
}// namespace v311
}// namespace purple