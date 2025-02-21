cmake_minimum_required(VERSION 3.10)
project(TCP-Server-Client-Test LANGUAGES CXX)

include_directories(${CMAKE_SOURCE_DIR}/inc)
include_directories(${PROJECT_SOURCE_DIR}/test)

add_executable(${PROJECT_NAME} tests.cpp)

target_link_libraries(${PROJECT_NAME} TCP-Server)
target_link_libraries(${PROJECT_NAME} TCP-Client)
target_link_libraries(${PROJECT_NAME} pthread)

find_package(GTest 1.11.0 REQUIRED)
if (GTest_FOUND)
  target_link_libraries(${PROJECT_NAME} GTest::gtest GTest::gtest_main)
endif()

include_directories(${GTEST_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} GTest::gmock GTest::gmock_main)