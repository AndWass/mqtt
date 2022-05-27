//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/asio/async_result.hpp>

namespace purple
{
template<class CompletionToken, class... Results>
using async_result_t = typename boost::asio::async_result<std::decay_t<CompletionToken>, void(Results...)>::return_type;;
}