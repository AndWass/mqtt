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
    purple::stream<boost::beast::test::stream> stream(io);

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
    EXPECT_EQ(purple::details::num_varlen_int_bytes(268'435'455), 4);
    EXPECT_THROW(purple::details::num_varlen_int_bytes(268'435'456), std::length_error);
}

TEST(Stream, AsyncReadSmall) {
    namespace net = boost::asio;
    net::io_context io;
    purple::stream<boost::beast::test::stream> stream(io);

    boost::beast::test::stream server(io);
    stream.next_layer().connect(server);

    net::async_write(server, net::buffer("\x20\x03\x40\x41\x42", 5), [](auto...) {
    });

    boost::system::error_code final_ec;
    purple::fixed_header header;

    purple::byte_buffer buf(1024);
    stream.async_read(buf, [&](boost::system::error_code ec, purple::fixed_header h) {
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

TEST(Stream, AsyncReadLarge) {
    namespace net = boost::asio;
    net::io_context io;
    purple::stream<boost::beast::test::stream> stream(io);

    boost::beast::test::stream server(io);
    stream.next_layer().connect(server);

    net::async_write(server, net::buffer("\x20\x80\x80\x80\x01", 5), [](auto...) {
    });

    {
        constexpr size_t block_size = 512;
        std::shared_ptr<std::vector<uint8_t>> block(new std::vector<uint8_t>());
        for (size_t j = 0; j < block_size; j++) {
            block->push_back(j & 0x00FF);
        }
        for (size_t i = 0; i < 2'097'152; i += block_size) {
            net::async_write(server, net::buffer(*block), [block](auto...) {
            });
        }
    }

    boost::system::error_code final_ec;
    purple::fixed_header header;

    purple::byte_buffer buf(0x20000000);
    stream.async_read(buf, [&](boost::system::error_code ec, purple::fixed_header h) {
        final_ec = ec;
        header = h;
    });

    io.run();
    EXPECT_FALSE(final_ec.failed());
    ASSERT_EQ(buf.size(), 2'097'152);

    for (size_t i = 0; i < buf.size(); i++) {
        ASSERT_EQ(buf[i], i & 0x00FF);
    }

    EXPECT_EQ(header.first_byte, 0x20);
    EXPECT_EQ(header.remaining_length, 2'097'152);
}

TEST(Stream, AsyncReadBad) {
    namespace net = boost::asio;
    net::io_context io;
    purple::stream<boost::beast::test::stream> stream(io);

    boost::beast::test::stream server(io);
    stream.next_layer().connect(server);

    net::async_write(server, net::buffer("\x20\x80\x80\x80\x80", 5), [](auto...) {
    });

    {
        constexpr size_t block_size = 512;
        std::shared_ptr<std::vector<uint8_t>> block(new std::vector<uint8_t>());
        for (size_t j = 0; j < block_size; j++) {
            block->push_back(j & 0x00FF);
        }
        for (size_t i = 0; i < 2'097'152; i += block_size) {
            net::async_write(server, net::buffer(*block), [block](auto...) {
            });
        }
    }

    boost::system::error_code final_ec;
    purple::fixed_header header;

    purple::byte_buffer buf(0x20000000);
    stream.async_read(buf, [&](boost::system::error_code ec, purple::fixed_header h) {
        final_ec = ec;
        header = h;
    });

    io.run();
    EXPECT_TRUE(final_ec.failed());
    EXPECT_EQ(buf.size(), 0);
}

TEST(Stream, AsyncReadEmpty) {
    namespace net = boost::asio;
    net::io_context io;
    purple::stream<boost::beast::test::stream> stream(io);

    boost::beast::test::stream server(io);
    stream.next_layer().connect(server);

    net::async_write(server, net::buffer("\x20\x00\x40\x41\x42", 5), [](auto...) {
    });

    boost::system::error_code final_ec;
    purple::fixed_header header;

    purple::byte_buffer buf(1024);
    stream.async_read(buf, [&](boost::system::error_code ec, purple::fixed_header h) {
        final_ec = ec;
        header = h;
    });

    io.run();
    EXPECT_FALSE(final_ec.failed());
    ASSERT_EQ(buf.size(), 0);

    EXPECT_EQ(header.first_byte, 0x20);
    EXPECT_EQ(header.remaining_length, 0);
}

TEST(Stream, AsyncReadMultiple) {
    namespace net = boost::asio;
    net::io_context io;
    purple::stream<boost::beast::test::stream> stream(io);

    boost::beast::test::stream server(io);
    stream.next_layer().connect(server);

    net::async_write(server, net::buffer("\x20\x03\x40\x41\x42\x90\x04\x01\x02\x03\x04", 11), [](auto...) {
    });

    struct result {
        boost::system::error_code final_ec;
        purple::fixed_header header;

        purple::byte_buffer buf = purple::byte_buffer(1024);
    };

    result results[2];

    stream.async_read(results[0].buf, [&](boost::system::error_code ec, purple::fixed_header h) {
        results[0].final_ec = ec;
        results[0].header = h;

        stream.async_read(results[1].buf, [&](auto ec, auto h) {
            results[1].final_ec = ec;
            results[1].header = h;
        });
    });

    io.run();
    EXPECT_FALSE(results[0].final_ec.failed());
    ASSERT_EQ(results[0].buf.size(), 3);
    EXPECT_EQ(results[0].buf.at(0), 0x40);
    EXPECT_EQ(results[0].buf.at(1), 0x41);
    EXPECT_EQ(results[0].buf.at(2), 0x42);

    EXPECT_EQ(results[0].header.first_byte, 0x20);
    EXPECT_EQ(results[0].header.remaining_length, 0x03);

    EXPECT_FALSE(results[1].final_ec.failed());
    ASSERT_EQ(results[1].buf.size(), 4);
    EXPECT_EQ(results[1].buf.at(0), 0x01);
    EXPECT_EQ(results[1].buf.at(1), 0x02);
    EXPECT_EQ(results[1].buf.at(2), 0x03);
    EXPECT_EQ(results[1].buf.at(3), 0x04);

    EXPECT_EQ(results[1].header.first_byte, 0x90);
    EXPECT_EQ(results[1].header.remaining_length, 0x04);
}

TEST(Stream, AsyncReadFromExisting) {
    namespace net = boost::asio;
    net::io_context io;
    purple::stream<boost::beast::test::stream> stream(io);

    boost::beast::test::stream server(io);
    stream.next_layer().connect(server);

    net::write(server, net::buffer("\x20\x00\x21\x01\x42", 5));

    boost::system::error_code final_ec;
    purple::fixed_header header;
    bool called = false;

    purple::byte_buffer buf(1024);
    stream.async_read(buf, [&](boost::system::error_code ec, purple::fixed_header h) {
        final_ec = ec;
        header = h;
        called = true;
    });

    EXPECT_FALSE(called);

    io.run();
    EXPECT_FALSE(final_ec.failed());
    ASSERT_EQ(buf.size(), 0);

    EXPECT_EQ(header.first_byte, 0x20);
    EXPECT_EQ(header.remaining_length, 0);
    EXPECT_TRUE(called);

    called = false;

    stream.async_read(buf, [&](boost::system::error_code ec, purple::fixed_header h) {
        final_ec = ec;
        header = h;
        called = true;
    });

    EXPECT_FALSE(called);
    io.restart();
    io.run();
    EXPECT_FALSE(final_ec.failed());
    ASSERT_EQ(buf.size(), 1);

    EXPECT_EQ(header.first_byte, 0x21);
    EXPECT_EQ(header.remaining_length, 1);
    EXPECT_TRUE(called);
}
