cmake_minimum_required(VERSION 3.14)

project(purpleTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(purple REQUIRED)
  enable_testing()
endif()

find_package(GTest REQUIRED)

# ---- Tests ----

add_executable(purple_test
        source/stream.cpp
        source/v311_client.cpp
        )

target_link_libraries(
    purple_test PRIVATE
    purple::purple
    GTest::gtest
    GTest::gtest_main
)
target_compile_features(purple_test PRIVATE cxx_std_14)

add_test(NAME purple_test COMMAND purple_test)

# ---- End-of-file commands ----

add_folders(Test)
