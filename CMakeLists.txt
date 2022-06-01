cmake_minimum_required(VERSION 3.22)
project(purple)

find_package(Boost 1.79.0 REQUIRED COMPONENTS container)

include(CTest)

add_library(purple
        INTERFACE
        )

add_library(purple::purple ALIAS purple)

target_compile_features(purple INTERFACE cxx_std_14)
target_include_directories(purple INTERFACE include)

target_link_libraries(purple INTERFACE
        Boost::boost Boost::container)

if (BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(examples)
endif ()