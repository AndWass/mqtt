#include <mqtt/byte_buffer.hpp>
#include <mqtt/stream.hpp>

#include <boost/asio.hpp>

#include <boost/utility/string_view.hpp>

#include <boost/beast/core/bind_handler.hpp>

#include <cstdio>

namespace net = boost::asio;
namespace beast = boost::beast;

using boost::system::error_code;
using socket_type = net::ip::tcp::socket;
using resolver_type = net::ip::tcp::resolver;

// Helpers to help us construct the CONNECT packet payload
void add_u16(uint16_t data, std::vector<uint8_t> &out) {
    out.push_back(data >> 8);
    out.push_back(data & 0x00FF);
}

void add_string(boost::string_view str, std::vector<uint8_t> &out) {
    add_u16(str.size(), out);
    out.insert(out.end(), str.begin(), str.end());
}

struct client {
    mqtt::stream<socket_type> stream;
    resolver_type resolver;
    mqtt::byte_buffer read_buffer;

    template<class... Args>
    explicit client(Args &&...args)
        : stream(std::forward<Args>(args)...), resolver(stream.get_executor()), read_buffer(1024) {
        resolver.async_resolve("test.mosquitto.org", "1883", beast::bind_front_handler(&client::resolve_complete, this));
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
            std::vector<uint8_t> connect_packet;
            // Manually build the CONNECT payload
            // Don't include the fixed header, this is constructed and handled
            // by stream.async_write().
            add_string("MQTT", connect_packet);
            connect_packet.push_back(4);
            connect_packet.push_back(0x02);
            add_u16(10, connect_packet);
            add_string("ASIOMQTTCLIENT", connect_packet);

            // Start reading so we can get the CONNACK response
            stream.async_read(read_buffer, beast::bind_front_handler(&client::message_received, this));
            // We will move the vector, so ensure that the buffer view refers to the correct data
            auto to_write = net::buffer(connect_packet);
            // By moving the vector we ensure that the data is kept alive until connect_sent is invoked
            stream.async_write(0x10, to_write, beast::bind_front_handler(&client::connect_sent, std::move(connect_packet)));
        }
    }

    void message_received(error_code ec, mqtt::message_view message) {
        if (!ec) {
            std::printf("Received first byte 0x%x\n", (unsigned)message.header.first_byte);
            if (message.header.first_byte != 0x20 || message.payload.size() != 2) {
                stream.async_read(read_buffer, beast::bind_front_handler(&client::message_received, this));
            } else {
                std::printf("CONNACK received (%d, %d)\n", (int)message.payload[0], (int)message.payload[1]);
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