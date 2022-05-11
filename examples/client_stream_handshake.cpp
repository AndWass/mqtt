//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <mqtt/byte_buffer.hpp>
#include <mqtt/v311/client_stream.hpp>
#include <mqtt/v311/connect_opts.hpp>

#include <boost/asio.hpp>

#include <boost/beast/core/bind_handler.hpp>

#include <cstdio>

namespace net = boost::asio;
namespace beast = boost::beast;

using boost::system::error_code;
using socket_type = net::ip::tcp::socket;
using resolver_type = net::ip::tcp::resolver;

struct client {
    mqtt::v311::client_stream<socket_type> stream;
    resolver_type resolver;
    mqtt::byte_buffer read_buffer;

    template<class... Args>
    explicit client(Args &&...args)
        : stream(std::forward<Args>(args)...), resolver(stream.get_executor()), read_buffer(1024) {
        resolver.async_resolve("test.mosquitto.org", "1883",
                               beast::bind_front_handler(&client::resolve_complete, this));
    }

    void resolve_complete(error_code ec, const resolver_type::results_type &results) {
        if (!ec) {
            std::printf("Establishing TCP connection\n");
            stream.next_layer().async_connect(*results, beast::bind_front_handler(&client::connected, this));
        }
    }

    void connected(error_code ec) {
        if (!ec) {
            std::printf("TCP connection established, sending CONNECT\n");
            mqtt::v311::connect_opts connect;
            connect.client_id = "ASIOMQTTCLIENT";
            connect.keep_alive = std::chrono::seconds(10);
            connect.will.emplace();
            connect.will->topic = "/asiomqtt";
            mqtt::assign_string(connect.will->payload, "Hello world");

            stream.async_handshake(connect, beast::bind_front_handler(&client::handshake, this));
        }
    }

    void handshake(const error_code &ec, bool session_present) {
        if (!ec) {
            std::printf("Handshake complete, session present = %d\n", (int)session_present);
        } else {
            std::printf("Error: %s", ec.message().c_str());
        }
    }
};

int main() {
    net::io_context io;
    client c(io.get_executor());
    io.run();
}