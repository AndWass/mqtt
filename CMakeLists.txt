cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    purple
    VERSION 0.1.0
    DESCRIPTION "Asio based MQTT library"
    HOMEPAGE_URL "https://github.com/AndWass/purple"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(purple_purple INTERFACE)
add_library(purple::purple ALIAS purple_purple)

set_property(
    TARGET purple_purple PROPERTY
    EXPORT_NAME purple
)

target_include_directories(
    purple_purple ${warning_guard}
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_compile_features(purple_purple INTERFACE cxx_std_14)

find_package(Boost REQUIRED)
target_link_libraries(purple_purple INTERFACE Boost::boost)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Examples ----

if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_EXAMPLES "Build examples tree." "${purple_DEVELOPER_MODE}")
  if(BUILD_EXAMPLES)
    find_package(Threads REQUIRED)
    add_subdirectory(example)
  endif()
endif()

# ---- Developer mode ----

if(NOT purple_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of purple"
  )
endif()

include(cmake/dev-mode.cmake)
