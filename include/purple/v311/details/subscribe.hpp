//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/weak_ptr.hpp>

namespace purple
{
namespace v311
{
namespace details
{
template<class ClientImpl>
struct subscribe_op {
    ClientImpl& client_;
    std::unique_ptr<uint8_t[]> message_;

    template<class Self>
    void operator()(Self& self, boost::system::error_code ec = {}, size_t = {})
    {
        
    }
};
}
}
}