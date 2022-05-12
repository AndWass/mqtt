//          Copyright Andreas Wass 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/container/vector.hpp>
#include <boost/utility/string_view.hpp>
#include <cstdint>

namespace purple {
using binary_t = boost::container::vector<uint8_t, void,
                                          boost::container::vector_options_t<boost::container::stored_size<uint16_t>>>;

inline void assign_string(binary_t &bin, boost::string_view str) {
    bin.reserve(str.size());
    bin.insert(bin.end(), str.begin(), str.end());
}
}// namespace purple