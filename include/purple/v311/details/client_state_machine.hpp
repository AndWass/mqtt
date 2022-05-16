//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace purple
{
namespace v311
{
namespace details
{
struct client_state_machine
{
    enum class state
    {
        waiting_start,

        handshaking,

    };
};
}
}
}