
add_executable(
        purple-unittests
        stream.cpp
        client_stream.cpp
        v311_client.cpp
)

target_link_libraries(purple-unittests PUBLIC
        CONAN_PKG::gtest
        purple
        )

add_test(NAME purple-unittests COMMAND purple-unittests)