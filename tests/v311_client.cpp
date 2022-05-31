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
    server_stream server(ioc.get_executor(), 2);
    server.on_rx_ = [&](const server_stream::received_item& item) {
        if (item.header.first_byte == purple::packet_type::connect) {
            ASSERT_EQ(server.received_items.size(), 1);
            server.write(purple::packet_type::connack, {0, 0});
        }
        else {
            ASSERT_EQ(item.header.first_byte & 0xF0, purple::packet_type::publish);
            ASSERT_EQ(server.received_items.size(), 2);
        }
    };
    purple::v311::client<client_stream> client(server.stream_.next_layer());
    auto fut = client.async_run("abc", "", "", boost::asio::use_future);
    client.async_publish("/test", purple::qos::qos0, "Hello", [&](boost::system::error_code ec) {
        client.stop();
    });

    ioc.run();
    try {
        fut.get();
        EXPECT_FALSE(true);
    }
    catch(const boost::system::system_error& ex) {
        EXPECT_EQ(ex.code(), purple::error::client_stopped);
    }
    EXPECT_EQ(server.received_items.size(), 2);
}
