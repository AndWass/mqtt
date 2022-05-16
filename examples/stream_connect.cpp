//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <purple/byte_buffer.hpp>
#include <purple/stream.hpp>

#include <purple/tcp/resolving_socket.hpp>

#include <purple/v311/connect_opts.hpp>

#include <boost/asio.hpp>

#include <boost/utility/string_view.hpp>

#include <boost/beast/core/bind_handler.hpp>

#include <cstdio>

namespace net = boost::asio;
namespace beast = boost::beast;

using boost::system::error_code;
using socket_type = purple::tcp::resolving_socket;
using resolver_type = net::ip::tcp::resolver;

struct client {
    purple::stream<socket_type> stream;
    purple::byte_buffer read_buffer;

    explicit client(const socket_type::executor_type &exec)
        : stream(exec, "test.mosquitto.org", "1883"), read_buffer(1024) {
        stream.next_layer().async_connect(beast::bind_front_handler(&client::connected, this));
    }

    void connected(error_code ec) {
        if (!ec) {
            std::printf("TCP connection established, sending CONNECT\n");
            purple::v311::connect_opts connect;
            connect.client_id = "ASIOMQTTCLIENT";
            connect.keep_alive = std::chrono::minutes(2);

            std::vector<uint8_t> connect_packet;
            connect_packet.resize(connect.wire_size());
            connect.write_to(connect_packet.data());

            // Start reading so we can get the CONNACK response
            stream.async_read(read_buffer, beast::bind_front_handler(&client::message_received, this));
            // We will move the vector, so ensure that the buffer view refers to the correct data
            auto to_write = net::buffer(connect_packet);
            // By moving the vector we ensure that the data is kept alive until connect_sent is invoked
            stream.async_write(0x10, to_write,
                               beast::bind_front_handler(&client::connect_sent, std::move(connect_packet)));
        }
    }

    void message_received(error_code ec, purple::fixed_header header) {
        if (!ec) {
            std::printf("Received first byte 0x%x\n", (unsigned)header.first_byte);
            if (header.first_byte != 0x20 || read_buffer.size() != 2) {
                stream.async_read(read_buffer, beast::bind_front_handler(&client::message_received, this));
            } else {
                std::printf("CONNACK received (%d, %d)\n", (int)read_buffer.at(0), (int)read_buffer.at(1));
            }
        }
    }

    static void connect_sent(const std::vector<uint8_t> &, error_code ec, size_t n) {
        if (!ec) {
            std::printf("CONNECT sent, wrote %d bytes\n", (int)n);
        }
    }
};

int main() {
    net::io_context io;
    client c(io.get_executor());
    io.run();
}