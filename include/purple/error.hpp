//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include <type_traits>

#include <boost/system/error_category.hpp>
#include <boost/system/is_error_code_enum.hpp>

namespace purple {
enum class error {
    success = 0,
    invalid_connect_response,
    unacceptable_protocol_version,
    identifier_rejected,
    server_unavailable,
    bad_username_or_password,
    unauthorized,
    message_too_large,
    client_aborted,
    client_stopped,
    socket_disconnected,
};
}

namespace boost {
namespace system {
template<>
struct is_error_code_enum<purple::error> : std::true_type {};
}// namespace system
}// namespace boost

namespace purple {
class purple_category_impl final : public boost::system::error_category {
public:
    virtual ~purple_category_impl() = default;
    const char *name() const noexcept override {
        return "purple mqtt";
    }
    std::string message(int ev) const override {
        char buffer[64];
        return this->message(ev, buffer, sizeof(buffer));
    }

    const char *message(int ev, char *buffer, std::size_t len) const noexcept override {
        switch (static_cast<error>(ev)) {
        case error::success: return "No error";
        case error::invalid_connect_response: return "Invalid CONNECT response";
        case error::unacceptable_protocol_version: return "Unacceptable protocol version";
        case error::identifier_rejected: return "Identifier rejected";
        case error::server_unavailable: return "Server unavailable";
        case error::bad_username_or_password: return "Bad username or password";
        case error::unauthorized: return "Unauthorized";
        case error::message_too_large: return "Message too large";
        case error::client_aborted: return "Client aborted";
        case error::client_stopped: return "Client stopped";
        case error::socket_disconnected: return "Underlying socket disconnected";
        }

        std::snprintf(buffer, len, "Unknown MQTT error %d", ev);
        return buffer;
    }

private:
};

inline const boost::system::error_category &purple_category() {
    static const purple_category_impl instance;
    return instance;
}

inline boost::system::error_code make_error_code(error e) {
    return boost::system::error_code(static_cast<int>(e), purple_category());
}
}// namespace purple

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif