# CMake-Version
cmake_minimum_required(VERSION 2.8)

# Name
project(decklink-debugger)

# Compiler-Settings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")

# Output-Dir
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

# C++11
set (CMAKE_CXX_STANDARD 11)

# DeckLink SDK
set(DECKLINK_INCLUDE_DIR "" CACHE FILEPATH "Path to DeckLink linux/include directory")

if(NOT EXISTS ${DECKLINK_INCLUDE_DIR}/DeckLinkAPI.h)
  message(SEND_ERROR "Can't find DeckLinkAPI.h in '${DECKLINK_INCLUDE_DIR}', set dir with 'cmake -D DECKLINK_INCLUDE_DIR=path/to/linux/include .'")
endif()

include_directories(${DECKLINK_INCLUDE_DIR})


# Include local Headers
include_directories("${PROJECT_SOURCE_DIR}")

# Build Binary
add_executable(decklink-debugger
	main.cpp
	tostring.cpp
	CaptureDelegate.cpp
	DeviceProber.cpp
	HttpServer.cpp
	${DECKLINK_INCLUDE_DIR}/DeckLinkAPIDispatch.cpp
)

# Specify Link-Libraries
target_link_libraries(decklink-debugger dl pthread microhttpd)
