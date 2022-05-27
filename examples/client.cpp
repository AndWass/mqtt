#include <iostream>

#include <purple/tcp/resolving_socket.hpp>

#include <purple/v311/client.hpp>

#include <boost/asio.hpp>


int main() {
    boost::asio::io_context io;

    purple::v311::client<purple::tcp::resolving_socket> client(io.get_executor(), "test.mosquitto.org", "1883");

    client.set_connection_event_callback([](boost::system::error_code ec, purple::connection_event evt) {
        if (!ec && evt == purple::connection_event::handshake_successful) {
            std::cout << "Connection established!" << std::endl;
        }
    });

    client.set_keep_alive(std::chrono::minutes(5));

    /*client.async_subscribe("/asio/mqttclient", purple::qos::qos0, [](boost::system::error_code ec) {
        if (ec) {
            std::cout << "Failed to subscribe to /asio/mqttclient: " << ec.what() << std::endl;
        }
    });*/

    client.async_publish("/asio/mqttclient", purple::qos::qos0, "Hello world!", [](boost::system::error_code ec) {
        std::cout << "Publish result: " << ec.message() << std::endl;
    });

    client.start("ASIOMQTTCLIENT", "", "");

    boost::asio::steady_timer stop_timer(io.get_executor());
    stop_timer.expires_after(std::chrono::seconds(5));
    stop_timer.async_wait([&](auto) {
        std::cout << "Stopping..." << std::endl;
        client.stop();
    });

    io.run();
}
