//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace purple {
/**
 * @enum purple::v311::connection_event
 * @ingroup v311_client
 * @brief Connection events reported by @ref client
 *
 * Values of this type are passed to the connection event callback
 * set using @ref client::set_connection_event_callback
 *
 */
enum class connection_event {
    initiating_socket_connection, /**< A socket connection has been started. */
    socket_connected,             /**< Socket connection established, handshake started. */
    handshake_successful,         /**< Handshake successful, connection fully established */
    socket_disconnected,          /**< Socket has disconnected. */
};
}// namespace purple