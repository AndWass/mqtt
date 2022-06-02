find_package(Threads REQUIRED)

add_executable(stream_connect stream_connect.cpp)
target_link_libraries(stream_connect purple Threads::Threads)

add_executable(client client.cpp)
target_link_libraries(client purple Threads::Threads)