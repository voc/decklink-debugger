cmake_minimum_required(VERSION 2.8)

project(decklink-debugger)


set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

include_directories("${PROJECT_SOURCE_DIR}")

add_executable(hello ${PROJECT_SOURCE_DIR}/main.cpp)
