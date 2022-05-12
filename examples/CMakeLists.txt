find_package(Threads REQUIRED)

add_executable(stream_connect stream_connect.cpp)
target_link_libraries(stream_connect purple Threads::Threads)

add_executable(client_stream_handshake client_stream_handshake.cpp)
target_link_libraries(client_stream_handshake purple Threads::Threads)