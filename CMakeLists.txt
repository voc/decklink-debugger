# CMake-Version
cmake_minimum_required(VERSION 3.1)

# Name
project(decklink-debugger)

# Compiler-Settings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")

# C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(SYSTEM ${DECKLINK_INCLUDE_DIR})

# Compile resources to Header-Files
set(RESOURCE_COMPILER xxd)
set(RESOURCE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/rc")
set(RESOURCE_TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/rc.hex")

file(GLOB_RECURSE RESOURCES_TO_COMPILE RELATIVE "${RESOURCE_SOURCE_DIR}/" "${RESOURCE_SOURCE_DIR}/*")
file(MAKE_DIRECTORY "${RESOURCE_TARGET_DIR}")

# target representing the resource custom_commands
add_custom_target(resources)

foreach(INPUT_FILE_NAME ${RESOURCES_TO_COMPILE})
    set(INPUT_FILE  "${RESOURCE_SOURCE_DIR}/${INPUT_FILE_NAME}")
    set(OUTPUT_FILE "${RESOURCE_TARGET_DIR}/${INPUT_FILE_NAME}.hex")

    add_custom_command(
        TARGET resources POST_BUILD
        MAIN_DEPENDENCY ${INPUT_FILE}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMAND ${RESOURCE_COMPILER} -i rc/${INPUT_FILE_NAME} ${OUTPUT_FILE}
        COMMENT "Compiling rc/${INPUT_FILE_NAME} in ${CMAKE_CURRENT_SOURCE_DIR}/ to ${OUTPUT_FILE}")
endforeach()

# Generate File referencing Resources
configure_file(rc.cpp.in rc.cpp)

# Include local Headers
include_directories("${PROJECT_SOURCE_DIR}")

# Check for Blackmagic SDK headers location on the system, and include location
# User may specify Blackmagic SDK directory as parameter to CMake
set(BLACKMAGIC_SDK_DIR "" CACHE STRING "Blackmagic SDK dir")

if(BLACKMAGIC_SDK_DIR)
message(STATUS "Using Blackmagic SDK dir: ${BLACKMAGIC_SDK_DIR}")
endif()

# Search for header in expected system location and user-specified location
find_path(DECKLINK_INCLUDE_DIR DeckLinkAPI.h HINTS /usr/lib/decklink-sdk ${BLACKMAGIC_SDK_DIR}/include)

# Exit with error state if Blackmagic SDK files aren't available
# This way, the user gets proper feedback instead of cryptic compiler errors
if(NOT DECKLINK_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find Blackmagic SDK headers. Try specifying the SDK dir using -DBLACKMAGIC_SDK_DIR=.../Blackmagic-DeckLink-SDK-10.10/Linux/")
else()
    message(STATUS "Including DeckLink API files in: ${DECKLINK_INCLUDE_DIR}")
    include_directories(${DECKLINK_INCLUDE_DIR})
endif()

# Build Binary
add_executable(decklink-debugger
	decklink-debugger.cpp
	tostring.cpp
	CaptureDelegate.cpp
	DeviceProber.cpp
	HttpServer.cpp
	MutableVideoFrame.cpp
	ImageEncoder.cpp
	TablePrinter.cpp
	DeckLinkAPIDispatch.cpp
	${CMAKE_CURRENT_BINARY_DIR}/rc.cpp
)
# Make target depend on resources to make sure the resources target is built before the binary
add_dependencies(decklink-debugger resources)

# Search for and link to dependencies
find_package(Threads REQUIRED)
find_package(PNG REQUIRED)
find_library(MICROHTTPD microhttpd)
target_link_libraries(decklink-debugger dl ${CMAKE_THREAD_LIBS_INIT} ${MICROHTTPD} ${PNG_LIBRARY})
target_include_directories(decklink-debugger PRIVATE ${PNG_INCLUDE_DIR})

install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/decklink-debugger" DESTINATION bin)
