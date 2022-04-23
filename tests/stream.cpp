//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <gtest/gtest.h>

#include <mqtt/byte_buffer.hpp>
#include <mqtt/stream.hpp>

#include <boost/asio.hpp>

#include <boost/beast/_experimental/test/stream.hpp>

TEST(Stream, AsyncWrite) {
    static const char *data = "MQTT";

    namespace net = boost::asio;
    net::io_context io;
    mqtt::stream<boost::beast::test::stream> stream(io);

    boost::beast::test::stream server(io);
    stream.next_layer().connect(server);

    boost::system::error_code final_ec;
    size_t amount_written = 0;
    bool called = false;
    stream.async_write(0x10, net::buffer(data, 4), [&](boost::system::error_code ec, size_t n) {
        final_ec = ec;
        amount_written = n;
        called = true;
    });

    io.run();
    EXPECT_TRUE(called);
    EXPECT_FALSE(final_ec.failed());
    EXPECT_EQ(amount_written, 6);
}

TEST(Stream, VarlenMax) {
    EXPECT_EQ(mqtt::details::stream::num_varlen_int_bytes(268'435'455), 4);
    EXPECT_THROW(mqtt::details::stream::num_varlen_int_bytes(268'435'456), std::runtime_error);
}

TEST(Stream, AsyncReadFixedHeader) {
    namespace net = boost::asio;
    net::io_context io;
    mqtt::stream<boost::beast::test::stream> stream(io);

    boost::beast::test::stream server(io);
    stream.next_layer().connect(server);

    net::async_write(server, net::buffer("\x20\x03\x00\x00", 4), [](auto...) {
    });

    boost::system::error_code final_ec;
    mqtt::fixed_header header;

    stream.async_read_fixed_header([&](boost::system::error_code ec, mqtt::fixed_header h) {
        final_ec = ec;
        header = h;
    });

    io.run();
    EXPECT_FALSE(final_ec.failed());
    EXPECT_EQ(header.first_byte, 0x20);
    EXPECT_EQ(header.remaining_length, 0x03);
}

TEST(Stream, AsyncReadLargeFixedHeader) {
    namespace net = boost::asio;
    net::io_context io;
    mqtt::stream<boost::beast::test::stream> stream(io);

    boost::beast::test::stream server(io);
    stream.next_layer().connect(server);

    net::async_write(server, net::buffer("\x20\xFF\xFF\xFF\x7F", 5), [](auto...) {});

    boost::system::error_code final_ec;
    mqtt::fixed_header header;

    stream.async_read_fixed_header([&](boost::system::error_code ec, mqtt::fixed_header h) {
        final_ec = ec;
        header = h;
    });

    io.run();
    EXPECT_FALSE(final_ec.failed());
    EXPECT_EQ(header.first_byte, 0x20);
    EXPECT_EQ(header.remaining_length, 268'435'455);
}

TEST(Stream, AsyncReadBadFixedHeader) {
    namespace net = boost::asio;
    net::io_context io;
    mqtt::stream<boost::beast::test::stream> stream(io);

    boost::beast::test::stream server(io);
    stream.next_layer().connect(server);

    net::async_write(server, net::buffer("\x20\xFF\xFF\xFF\xFF", 5), [](auto...) {});

    boost::system::error_code final_ec;
    mqtt::fixed_header header;

    stream.async_read_fixed_header([&](boost::system::error_code ec, mqtt::fixed_header h) {
        final_ec = ec;
        header = h;
    });

    io.run();
    EXPECT_TRUE(final_ec.failed());
}

TEST(Stream, AsyncReadWeirdFixedHeader) {
    namespace net = boost::asio;
    net::io_context io;
    mqtt::stream<boost::beast::test::stream> stream(io);

    boost::beast::test::stream server(io);
    stream.next_layer().connect(server);

    net::async_write(server, net::buffer("\x20\x80\x80\x80\x00", 5), [](auto...) {});

    boost::system::error_code final_ec;
    mqtt::fixed_header header;

    stream.async_read_fixed_header([&](boost::system::error_code ec, mqtt::fixed_header h) {
        final_ec = ec;
        header = h;
    });

    io.run();
    EXPECT_FALSE(final_ec.failed());
    EXPECT_EQ(header.first_byte, 0x20);
    EXPECT_EQ(header.remaining_length, 0);
}

TEST(Stream, AsyncReadSmall) {
    namespace net = boost::asio;
    net::io_context io;
    mqtt::stream<boost::beast::test::stream> stream(io);

    boost::beast::test::stream server(io);
    stream.next_layer().connect(server);

    net::async_write(server, net::buffer("\x20\x03\x40\x41\x42", 5), [](auto...) {
    });

    boost::system::error_code final_ec;
    mqtt::fixed_header header;

    mqtt::byte_buffer buf(1024);
    stream.async_read(buf, [&](boost::system::error_code ec, mqtt::fixed_header h) {
        final_ec = ec;
        header = h;
    });

    io.run();
    EXPECT_FALSE(final_ec.failed());
    ASSERT_EQ(buf.size(), 3);
    EXPECT_EQ(buf.at(0), 0x40);
    EXPECT_EQ(buf.at(1), 0x41);
    EXPECT_EQ(buf.at(2), 0x42);

    EXPECT_EQ(header.first_byte, 0x20);
    EXPECT_EQ(header.remaining_length, 0x03);
}

TEST(Stream, AsyncReadEmpty) {
    namespace net = boost::asio;
    net::io_context io;
    mqtt::stream<boost::beast::test::stream> stream(io);

    boost::beast::test::stream server(io);
    stream.next_layer().connect(server);

    net::async_write(server, net::buffer("\x20\x00\x40\x41\x42", 5), [](auto...) {
    });

    boost::system::error_code final_ec;
    mqtt::fixed_header header;

    mqtt::byte_buffer buf(1024);
    stream.async_read(buf, [&](boost::system::error_code ec, mqtt::fixed_header h) {
        final_ec = ec;
        header = h;
    });

    io.run();
    EXPECT_FALSE(final_ec.failed());
    ASSERT_EQ(buf.size(), 0);

    EXPECT_EQ(header.first_byte, 0x20);
    EXPECT_EQ(header.remaining_length, 0);
}
