//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <chrono>
#include <cstring>
#include <string>

#include <purple/binary.hpp>
#include <purple/v311/will.hpp>

#include <purple/details/bits.hpp>

#include <boost/optional.hpp>

namespace purple {
namespace v311 {
struct connect_message {
    std::string client_id;

    std::string username;
    std::string password;
    boost::optional<purple::v311::will_t> will;

    std::chrono::seconds keep_alive;
    bool clean_session = true;

    [[nodiscard]] uint8_t flag_byte() const {
        uint8_t flags = clean_session ? 0x02 : 0;
        if (!username.empty()) {
            flags |= 0x80 | 0x40;
        }

        if (will.has_value()) {
            flags |= 0x04;
            flags |= static_cast<uint8_t>(will->quality_of_service) << 3;
            flags |= will->retain ? 0x20 : 0;
        }

        return flags;
    }

    [[nodiscard]] size_t wire_size() const {
        size_t retval = 10 + 2 + client_id.size();
        if (!username.empty()) {
            retval += 4 + static_cast<uint16_t>(username.size()) + static_cast<uint16_t>(password.size());
        }

        if (will.has_value()) {
            retval += 4 + will->topic.size() + will->payload.size();
        }

        return retval;
    }

    uint8_t *write_to(uint8_t *out) const {
        memcpy(out, "\x00\x04MQTT\x04", 7);// NOLINT(bugprone-not-null-terminated-result)
        out += 7;
        *out++ = flag_byte();
        out = purple::details::put_u16(keep_alive.count(), out);
        out = purple::details::put_str(client_id, out);

        if (will.has_value()) {
            out = purple::details::put_str(will->topic, out);
            out = purple::details::put_bin(will->payload, out);
        }

        if (!username.empty()) {
            out = purple::details::put_str(username, out);
            out = purple::details::put_str(password, out);
        }

        return out;
    }
};
}// namespace v311
}// namespace purple