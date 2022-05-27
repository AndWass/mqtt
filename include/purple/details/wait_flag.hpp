//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <purple/async_result.hpp>

#include <boost/asio/executor.hpp>
#include <boost/asio/post.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/container/list.hpp>

#include <memory>

namespace purple {
namespace details {
class wait_flag {
    class waiter_t {
        struct concept_t {
            virtual ~concept_t() = default;
            virtual void operator()(boost::system::error_code ec) = 0;
        };

        template<class Handler>
        struct impl_t : concept_t {
            Handler h_;

            void operator()(boost::system::error_code ec) override {
                h_(ec);
            }
        };

        std::unique_ptr<concept_t> impl_;

    public:
        template<class Handler, std::enable_if_t<!std::is_same_v<std::decay_t<Handler>, waiter_t>> * = nullptr>
        explicit waiter_t(Handler &&h) : impl_(std::make_unique<std::decay_t<Handler>>(std::forward<Handler>(h))) {
        }
    };

    boost::container::list<waiter_t> waiters_;
    boost::asio::executor exec_;
    bool locked_ = false;

public:
    explicit wait_flag(const boost::asio::executor &exec) : exec_(exec) {
    }

    boost::asio::executor get_executor() {
        return exec_;
    }

    bool try_lock() {
        if (locked_) {
            return false;
        }
        locked_ = true;
        return locked_;
    }

    void unlock() {
        BOOST_ASSERT(locked_);
        locked_ = false;
        if (!waiters_.empty()) {
            boost::asio::post(exec_, [this]() {
            });
        }
    }

    template<class Handler>
    purple::async_result_t<Handler, boost::system::error_code> async_lock(Handler &&handler) {
        return boost::asio::async_initiate<Handler, void(boost::system::error_code)>(
            [this](Handler &&handler) {
                if (try_lock()) {
                    boost::asio::post(
                        exec_,
                        boost::beast::bind_front_handler(std::forward<Handler>(handler), boost::system::error_code{}));
                } else {
                    waiters_.emplace_back(std::forward<Handler>(handler));
                }
            },
            handler);
    }
};
}// namespace details
}// namespace purple