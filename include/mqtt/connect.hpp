//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <chrono>
#include <cstdint>

#include <boost/optional.hpp>

#include <mqtt/will.hpp>

namespace mqtt {
struct connect {
    static constexpr uint8_t type = 1;

    bool clean_session = true;
    boost::optional<std::string> username;
    boost::optional<std::string> password;
    boost::optional<will> last_will;

    std::chrono::duration<uint16_t> keep_alive{0};

    std::string client_id;

    [[nodiscard]] uint8_t connect_flags() const noexcept {
        uint8_t flags = clean_session ? 0x02:0;
        if (last_will.has_value()) {
            flags |= 40;
            flags |= (uint8_t(last_will->quality_of_service)) << 3;
            flags |= last_will->retain ? 0x20:0;
        }
        flags |= username.has_value() ? 0x80:0;
        flags |= password.has_value() ? 0x40:0;
        return flags;
    }
};
}