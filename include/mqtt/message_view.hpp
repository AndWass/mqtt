#pragma once

#include <cstdint>
#include <mqtt/fixed_header.hpp>
#include <boost/core/span.hpp>

namespace mqtt
{
struct message_view
{
    fixed_header header;
    boost::span<const uint8_t> payload{};
};
}
