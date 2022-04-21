cmake_minimum_required(VERSION 3.22)
project(mqtt)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

include(CTest)

add_library(mqtt
        src/connect.cpp
        )

target_compile_features(mqtt PUBLIC cxx_std_14)
target_include_directories(mqtt PUBLIC include)

target_link_libraries(mqtt PUBLIC
        CONAN_PKG::boost)

if (BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(examples)
endif()