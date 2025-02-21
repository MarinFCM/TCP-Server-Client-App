cmake_minimum_required(VERSION 3.22.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(TCP-Server-Client-App LANGUAGES CXX)

enable_testing()
add_subdirectory(${PROJECT_SOURCE_DIR}/test)
include_directories(${PROJECT_SOURCE_DIR}/inc)

set(INC_DIR ${PROJECT_SOURCE_DIR}/inc)
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)

set(SERVER_SOURCES
    ${SRC_DIR}/tcp_server.cpp
    ${SRC_DIR}/tcp_connection.cpp
)

set(CLIENT_SOURCES
    ${SRC_DIR}/tcp_client.cpp
    ${SRC_DIR}/tcp_connection.cpp
)

add_compile_options(-Wall -Wextra -Wpedantic -O2)

#Creating a library so that it can be linked to the test executable
add_library(TCP-Server SHARED
  ${INC_DIR}/command_handler.hpp
  ${INC_DIR}/tcp_connection.hpp
  ${SRC_DIR}/tcp_connection.cpp
  ${INC_DIR}/tcp_server.hpp
  ${SRC_DIR}/tcp_server.cpp
  )

#Creating a library so that it can be linked to the test executable
add_library(TCP-Client SHARED
  ${INC_DIR}/command_handler.hpp
  ${INC_DIR}/tcp_client.hpp
  ${SRC_DIR}/tcp_client.cpp
  ${INC_DIR}/tcp_connection.hpp
  ${SRC_DIR}/tcp_connection.cpp
  )

#Creating an executable so it can be run from the command line
add_executable(tcp_server ${SERVER_SOURCES})

target_link_libraries(tcp_server pthread)

#Creating an executable so it can be run from the command line
add_executable(tcp_client ${CLIENT_SOURCES})

target_link_libraries(tcp_client pthread)

find_package(Boost 1.74.0 REQUIRED)
if (Boost_FOUND)
  target_include_directories(tcp_client PUBLIC ${Boost_INCLUDE_DIRS})
  target_include_directories(tcp_server PUBLIC ${Boost_INCLUDE_DIRS})
endif()