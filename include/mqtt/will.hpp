#pragma once

#include <mqtt/qos.hpp>
#include <vector>
namespace mqtt
{
struct will
{
    std::string topic;
    std::vector<uint8_t> message;
    qos quality_of_service = qos::qos1;
    bool retain = false;
};
}