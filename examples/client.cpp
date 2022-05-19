#include <purple/tcp/resolving_socket.hpp>

#include <purple/v311/client.hpp>

#include <boost/asio.hpp>

#include <iostream>

int main() {
    boost::asio::io_context io;
    purple::v311::client<purple::tcp::resolving_socket> client(io.get_executor(), "test.mosquitto.org", "1883");

    client.set_connection_event_callback([](boost::system::error_code ec, purple::connection_event evt) {
        std::cout << "Connection event " << (int)evt << " : " << ec.what() << std::endl;
    });

    client.set_keep_alive(std::chrono::seconds(10));
    client.set_connect_decorator([](purple::v311::connect_message &cm) {
        std::cout << "Connecting using client ID " << cm.client_id << std::endl;
    });

    client.set_message_callback([](purple::fixed_header header) {
        std::cout << "Received message " << std::hex << unsigned(header.first_byte) << " len "
                  << header.remaining_length << std::endl;
    });

    client.start("ASIOMQTTCLIENT", "", "");

    io.run();
}
