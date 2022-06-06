#include "purple/purple.hpp"

#include <catch2/catch.hpp>

TEST_CASE("Name is purple", "[library]") {
    REQUIRE(name() == "purple");
}
