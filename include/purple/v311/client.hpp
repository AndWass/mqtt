//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <purple/binary.hpp>
#include <purple/stream.hpp>

#include "details/client_state_machine.hpp"

#include <boost/asio/executor.hpp>

namespace purple {
namespace v311 {
template<class AsyncDefaultConnectableStream>
class client {
    purple::stream<AsyncDefaultConnectableStream> stream_;

public:
    using executor_type = typename purple::stream<AsyncDefaultConnectableStream>::executor_type;
    using next_layer_type = typename purple::stream<AsyncDefaultConnectableStream>::next_layer_type;

    template<class... Args>
    explicit client(Args &&...args) : stream_(std::forward<Args>(args)...) {
    }
};
}// namespace v311
}// namespace purple