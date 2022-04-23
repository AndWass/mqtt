cmake_minimum_required(VERSION 3.22)
project(mqtt)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

include(CTest)

add_library(mqtt
        INTERFACE
        )

target_compile_features(mqtt INTERFACE cxx_std_14)
target_include_directories(mqtt INTERFACE include)

target_link_libraries(mqtt INTERFACE
        CONAN_PKG::boost)

if (BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(examples)
endif ()