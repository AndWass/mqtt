//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/asio/coroutine.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/read.hpp>
#include <boost/beast/core/bind_handler.hpp>

#include <boost/system/error_code.hpp>
#include <cstdint>

#include <purple/byte_buffer.hpp>
#include <purple/error.hpp>
#include <purple/fixed_header.hpp>

#include "read_buffer.hpp"

namespace purple {
namespace details {
namespace stream {
namespace asio = boost::asio;
namespace system = boost::system;

template<class AsyncRead, class DynamicBuffer>
struct read_op {
    AsyncRead &stream_;
    read_buffer &internal_buffer_;
    DynamicBuffer &user_buffer_;
    asio::coroutine coro_;
    fixed_header header_;
    size_t payload_left_to_read_ = 0;

    bool needs_post_ = true;

    struct internal_buffer_tag {};
    struct user_buffer_tag {};

    [[nodiscard]] bool need_more_data_for_header(boost::system::error_code &ec) const {
        ec.clear();
        auto readable = internal_buffer_.readable_area();
        for (size_t i = 1; i < readable.size(); i++) {
            if (readable[i] < 128) {
                return false;
            } else if (i >= 5) {
                ec = boost::system::errc::make_error_code(boost::system::errc::protocol_error);
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] fixed_header consume_fixed_header(boost::system::error_code &ec) const {
        ec.clear();

        fixed_header retval{};
        auto readable = internal_buffer_.readable_area();
        if (readable.size() >= 2) {
            size_t i = 0;
            retval.first_byte = readable[i++];
            uint32_t multiplier = 1;
            while (i < readable.size()) {
                uint8_t value = readable[i++];
                retval.remaining_length += (value & 0x7F) * multiplier;
                if (value < 128) {
                    break;
                }

                multiplier *= 128;
                if (multiplier > 128 * 128 * 128) {
                    ec = boost::system::errc::make_error_code(boost::system::errc::protocol_error);
                    return retval;
                }
            }

            internal_buffer_.consume(i);
        }
        return retval;
    }

    template<class Self>
    void operator()(Self &self, system::error_code ec = {}, fixed_header header = {}) {
        BOOST_ASIO_CORO_REENTER(coro_) {
            while (need_more_data_for_header(ec)) {
                if (ec) {
                    break;
                }
                BOOST_ASIO_CORO_YIELD stream_.async_read_some(
                    internal_buffer_.mutable_buffer(),
                    boost::beast::bind_front_handler(std::move(self), internal_buffer_tag{}));
                needs_post_ = false;
                if (ec) {
                    break;
                }
            }
            if (!ec) {
                // At this point we know that the internal buffer contains a valid fixed header.
                header_ = consume_fixed_header(ec);
                if (!ec) {
                    {
                        const size_t amount_to_copy =
                            (std::min<size_t>)(internal_buffer_.size(), header_.remaining_length);
                        payload_left_to_read_ = header_.remaining_length - amount_to_copy;
                        if (amount_to_copy > 0) {
                            boost::asio::buffer_copy(user_buffer_.prepare(amount_to_copy),
                                                     internal_buffer_.const_buffer(), amount_to_copy);
                            user_buffer_.commit(amount_to_copy);
                            internal_buffer_.consume(amount_to_copy);
                        }
                    }
                    while (payload_left_to_read_ > 0) {
                        BOOST_ASIO_CORO_YIELD stream_.async_read_some(
                            user_buffer_.prepare(payload_left_to_read_),
                            boost::beast::bind_front_handler(std::move(self), user_buffer_tag{}));
                        needs_post_ = false;
                        if (ec) {
                            break;
                        }
                    }
                }
            } else {
                header_ = fixed_header{};
            }
            if (needs_post_) {
                auto exec = boost::asio::get_associated_executor(self);
                boost::asio::post(exec, [self = std::move(self), ec, header = this->header_]() mutable {
                    self.complete(ec, header);
                });
            } else {
                self.complete(ec, header_);
            }
        }
    }

    template<class Self>
    void operator()(Self &self, internal_buffer_tag, system::error_code ec, size_t n) {
        internal_buffer_.commit(n);
        (*this)(self, ec, header_);
    }

    template<class Self>
    void operator()(Self &self, user_buffer_tag, system::error_code ec, size_t n) {
        user_buffer_.commit(n);
        payload_left_to_read_ -= n;
        (*this)(self, ec, header_);
    }
};

template<class AsyncRead>
struct read_into_fixed_op {
    AsyncRead &stream_;
    read_buffer &internal_buffer_;
    boost::asio::mutable_buffer user_buffer_;
    asio::coroutine coro_;
    fixed_header header_;
    size_t payload_left_to_read_ = 0;

    bool needs_post_ = true;

    struct internal_buffer_tag {};
    struct user_buffer_tag {};

    [[nodiscard]] bool need_more_data_for_header(boost::system::error_code &ec) const {
        ec.clear();
        auto readable = internal_buffer_.readable_area();
        for (size_t i = 1; i < readable.size(); i++) {
            if (readable[i] < 128) {
                return false;
            } else if (i >= 5) {
                ec = boost::system::errc::make_error_code(boost::system::errc::protocol_error);
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] fixed_header peek_fixed_header(boost::system::error_code &ec, size_t &header_byte_len) const {
        ec.clear();
        header_byte_len = 0;
        fixed_header retval{};
        auto readable = internal_buffer_.readable_area();
        if (readable.size() >= 2) {
            size_t i = 0;
            retval.first_byte = readable[i++];
            uint32_t multiplier = 1;
            while (i < readable.size()) {
                uint8_t value = readable[i++];
                retval.remaining_length += (value & 0x7F) * multiplier;
                if (value < 128) {
                    break;
                }

                multiplier *= 128;
                if (multiplier > 128 * 128 * 128) {
                    ec = boost::system::errc::make_error_code(boost::system::errc::protocol_error);
                    return retval;
                }
            }

            header_byte_len = i;
        }
        return retval;
    }

    template<class Self>
    void operator()(Self &self, system::error_code ec = {}, fixed_header header = {}) {
        BOOST_ASIO_CORO_REENTER(coro_) {
            while (need_more_data_for_header(ec)) {
                if (ec) {
                    break;
                }
                BOOST_ASIO_CORO_YIELD stream_.async_read_some(
                    internal_buffer_.mutable_buffer(),
                    boost::beast::bind_front_handler(std::move(self), internal_buffer_tag{}));
                needs_post_ = false;
                if (ec) {
                    break;
                }
            }
            if (!ec) {
                // At this point we know that the internal buffer contains a valid fixed header.
                {
                    size_t to_consume = 0;
                    header_ = peek_fixed_header(ec, to_consume);
                    if (!ec) {
                        if (header_.remaining_length > boost::asio::buffer_size(user_buffer_)) {
                            header_ = fixed_header{};
                            ec = ::purple::make_error_code(purple::error::message_too_large);
                        } else {
                            internal_buffer_.consume(to_consume);
                        }
                    }
                }
                if (!ec) {
                    {
                        const size_t amount_to_copy =
                            (std::min<size_t>)(internal_buffer_.size(), header_.remaining_length);
                        payload_left_to_read_ = header_.remaining_length - amount_to_copy;
                        if (amount_to_copy > 0) {
                            boost::asio::buffer_copy(user_buffer_, internal_buffer_.const_buffer(), amount_to_copy);
                            user_buffer_ += amount_to_copy;
                            internal_buffer_.consume(amount_to_copy);
                        }
                    }
                    while (payload_left_to_read_ > 0) {
                        BOOST_ASIO_CORO_YIELD stream_.async_read_some(
                            user_buffer_, boost::beast::bind_front_handler(std::move(self), user_buffer_tag{}));
                        needs_post_ = false;
                        if (ec) {
                            break;
                        }
                    }
                }
            } else {
                header_ = fixed_header{};
            }
            if (needs_post_) {
                auto exec = boost::asio::get_associated_executor(self);
                boost::asio::post(exec, [self = std::move(self), ec, header = this->header_]() mutable {
                    self.complete(ec, header);
                });
            } else {
                self.complete(ec, header_);
            }
        }
    }

    template<class Self>
    void operator()(Self &self, internal_buffer_tag, system::error_code ec, size_t n) {
        internal_buffer_.commit(n);
        (*this)(self, ec, header_);
    }

    template<class Self>
    void operator()(Self &self, user_buffer_tag, system::error_code ec, size_t n) {
        user_buffer_ += n;
        payload_left_to_read_ -= n;
        (*this)(self, ec, header_);
    }
};
}// namespace stream
}// namespace details
}// namespace purple