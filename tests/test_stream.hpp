#pragma once

#include <boost/asio.hpp>
#include <boost/beast/_experimental/test/stream.hpp>

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
            }, handler);
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

void beast_close_socket(client_stream& cs) {
    cs.stream_.close();
}
