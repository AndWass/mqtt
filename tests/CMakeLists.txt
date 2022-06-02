find_package(GTest CONFIG REQUIRED)

add_executable(
        purple-unittests
        stream.cpp
        v311_client.cpp
)

target_link_libraries(purple-unittests PUBLIC
        GTest::gtest GTest::gtest_main
        purple
        )

add_test(NAME purple-unittests COMMAND purple-unittests)