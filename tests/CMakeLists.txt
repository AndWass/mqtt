
add_executable(
        mqtt-unittests
        stream.cpp
)

target_link_libraries(mqtt-unittests PUBLIC
        CONAN_PKG::gtest
        mqtt
        )

add_test(NAME mqtt-unittests COMMAND mqtt-unittests)