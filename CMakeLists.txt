cmake_minimum_required(VERSION 3.22.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(TCP-Server-Client-App LANGUAGES CXX)


include_directories(${PROJECT_SOURCE_DIR}/inc)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)

set(SERVER_SOURCES
    ${SRC_DIR}/tcp_server.cpp
    ${SRC_DIR}/tcp_connection.cpp
)

set(CLIENT_SOURCES
    ${SRC_DIR}/tcp_client.cpp
    ${SRC_DIR}/tcp_connection.cpp
)

add_executable(tcp_server ${SERVER_SOURCES})

target_link_libraries(tcp_server pthread)

add_executable(tcp_client ${CLIENT_SOURCES})

target_link_libraries(tcp_client pthread)

find_package(Boost 1.74.0 REQUIRED)
if (Boost_FOUND)
  target_include_directories(tcp_client PUBLIC ${Boost_INCLUDE_DIRS})
  target_include_directories(tcp_server PUBLIC ${Boost_INCLUDE_DIRS})
endif()

# Optional: Specify paths for ASIO headers (if not header-only)
# target_include_directories(tcp_server PRIVATE ${PROJECT_SOURCE_DIR}/path/to/asio)
# target_include_directories(tcp_client PRIVATE ${PROJECT_SOURCE_DIR}/path/to/asio)