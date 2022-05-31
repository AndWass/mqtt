#include <purple/packet_type.hpp>
#include <purple/v311/client.hpp>

#include "test_stream.hpp"

#include <gtest/gtest.h>

TEST(V311Client, AbortOnDestruction) {
    boost::asio::io_context ioc;
    boost::beast::test::stream server(ioc.get_executor());
    boost::system::error_code run_ec, publish_ec;
    {
        purple::v311::client<client_stream> client(server);
        client.async_run("abc", "", "", [&run_ec](boost::system::error_code ec) {
            run_ec = ec;
        });
        client.async_publish("/test", purple::qos::qos0, "Hello", [&publish_ec](boost::system::error_code ec) {
            publish_ec = ec;
        });
    }
    ioc.run();
    EXPECT_EQ(run_ec, purple::error::client_aborted);
    EXPECT_EQ(publish_ec, purple::error::client_aborted);
}

TEST(V311Client, PublishAfterHandshake) {
    boost::asio::io_context ioc;
    purple::stream<boost::beast::test::stream> server(ioc.get_executor());
    purple::v311::client<client_stream> client(server.next_layer());
    auto fut = client.async_run("abc", "", "", boost::asio::use_future);
    client.async_publish("/test", purple::qos::qos0, "Hello", [&](boost::system::error_code ec) {
        client.stop();
    });

    purple::byte_buffer server_buffer(1000);
    server.async_read(server_buffer, [&](boost::system::error_code ec, purple::fixed_header hdr) {
        ASSERT_EQ(hdr.first_byte, purple::packet_type::connect);
        std::vector<uint8_t> resp_data{0, 0};
        auto data = boost::asio::buffer(resp_data.data(), 2);
        server.async_write(purple::packet_type::connack, data, [x = std::move(resp_data)](auto...) {});
    });

    ioc.run();
    try {
        fut.get();
        EXPECT_FALSE(true);
    }
    catch(const boost::system::system_error& ex) {
        EXPECT_EQ(ex.code(), purple::error::client_stopped);
    }
}
