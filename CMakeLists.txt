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

include_directories(SYSTEM ${DECKLINK_INCLUDE_DIR})

# Compile resources to Header-Files
set(RESOURCE_COMPILER xxd)
file(GLOB_RECURSE RESOURCES_TO_COMPILE RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/rc" "${CMAKE_CURRENT_SOURCE_DIR}/rc/*")
file(MAKE_DIRECTORY "rc.hex")
foreach(INPUT_FILE_NAME ${RESOURCES_TO_COMPILE})
    set(INPUT_FILE  "rc/${INPUT_FILE_NAME}")
    set(OUTPUT_FILE "rc.hex/${INPUT_FILE_NAME}.hex")
    add_custom_command(
        OUTPUT ${OUTPUT_FILE}
        COMMAND ${RESOURCE_COMPILER} -i ${INPUT_FILE} ${OUTPUT_FILE}
        COMMENT "Compiling ${INPUT_FILE} to ${OUTPUT_FILE}")
    list(APPEND COMPILED_RESOURCES ${OUTPUT_FILE})
endforeach()

# Include local Headers
include_directories("${PROJECT_SOURCE_DIR}")

# Build Binary
add_executable(decklink-debugger
	decklink-debugger.cpp
	tostring.cpp
	rc.cpp
	CaptureDelegate.cpp
	DeviceProber.cpp
	HttpServer.cpp
	MutableVideoFrame.cpp
	ImageEncoder.cpp
	TablePrinter.cpp
	DeckLinkAPIDispatch.cpp
	${COMPILED_RESOURCES}
)

# Specify Link-Libraries
target_link_libraries(decklink-debugger dl pthread microhttpd png)

install(PROGRAMS decklink-debugger DESTINATION bin)
