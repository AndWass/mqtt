#pragma once

#include <purple/byte_buffer.hpp>
#include <purple/fixed_header.hpp>
#include <purple/stream.hpp>

#include <boost/asio.hpp>
#include <boost/beast/_experimental/test/stream.hpp>

#include <vector>

struct client_stream {
    boost::beast::test::stream stream_;
    boost::beast::test::stream &server_;

    using executor_type = boost::beast::test::stream::executor_type;

    executor_type get_executor() {
        return stream_.get_executor();
    }

    explicit client_stream(boost::beast::test::stream &server) : stream_(server.get_executor()), server_(server) {
    }

    template<class Handler>
    auto async_connect(Handler &&handler) {
        stream_.connect(server_);
        return boost::asio::async_initiate<Handler, void(boost::system::error_code)>(
            [exec = stream_.get_executor()](Handler &&handler) {
                boost::asio::post(exec, [h = std::forward<Handler>(handler)]() mutable {
                    h({});
                });
            },
            handler);
    }

    template<class BufferSequence, class Handler>
    auto async_read_some(const BufferSequence &buffers, Handler &&handler) {
        return stream_.async_read_some(buffers, std::forward<Handler>(handler));
    }

    template<class ConstBufferSequence, class Handler>
    auto async_write_some(const ConstBufferSequence &buffers, Handler &&handler) {
        return stream_.async_write_some(buffers, std::forward<Handler>(handler));
    }
};

void beast_close_socket(client_stream &cs) {
    cs.stream_.close();
}

struct server_stream {
    struct received_item {
        purple::fixed_header header;
        purple::byte_buffer data;
    };
    std::vector<received_item> received_items;

    purple::stream<boost::beast::test::stream> stream_;

    std::function<void(const received_item &)> on_rx_;

    explicit server_stream(const boost::beast::test::stream::executor_type &exec) : stream_(exec) {
        start_receive();
    }

    void start_receive() {
        auto buffer = std::make_unique<purple::byte_buffer>(1024);
        auto &buf = *buffer;
        stream_.async_read(buf, [buf = std::move(buffer), this](auto ec, purple::fixed_header hdr) mutable {
            if (!ec) {
                received_items.push_back({hdr, std::move(*buf)});
                if (on_rx_) {
                    on_rx_(received_items.back());
                }
                start_receive();
            }
        });
    }

    void write(uint8_t first_byte, std::vector<uint8_t> data) {
        auto asio_buffer = boost::asio::buffer(data.data(), data.size());
        stream_.async_write(first_byte, asio_buffer, [_ = std::move(data)](auto...) {
        });
    }
};
