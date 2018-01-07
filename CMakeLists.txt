# CMake-Version
cmake_minimum_required(VERSION 2.8)

# Name
project(decklink-debugger)

# Compiler-Settings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")

# C++11
set (CMAKE_CXX_STANDARD 11)

include_directories(SYSTEM ${DECKLINK_INCLUDE_DIR})

# Compile resources to Header-Files
set(RESOURCE_COMPILER xxd)
set(RESOURCE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/rc")
set(RESOURCE_TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/rc.hex")

file(GLOB_RECURSE RESOURCES_TO_COMPILE RELATIVE "${RESOURCE_SOURCE_DIR}/" "${RESOURCE_SOURCE_DIR}/*")
file(MAKE_DIRECTORY "${RESOURCE_TARGET_DIR}")

foreach(INPUT_FILE_NAME ${RESOURCES_TO_COMPILE})
    set(INPUT_FILE  "${RESOURCE_SOURCE_DIR}/${INPUT_FILE_NAME}")
    set(OUTPUT_FILE "${RESOURCE_TARGET_DIR}/${INPUT_FILE_NAME}.hex")

    add_custom_command(
        OUTPUT ${OUTPUT_FILE}
        MAIN_DEPENDENCY ${INPUT_FILE}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMAND ${RESOURCE_COMPILER} -i rc/${INPUT_FILE_NAME} ${OUTPUT_FILE}
        COMMENT "Compiling rc/${INPUT_FILE_NAME} in ${CMAKE_CURRENT_SOURCE_DIR}/ to ${OUTPUT_FILE}")

    list(APPEND COMPILED_RESOURCES ${OUTPUT_FILE})
endforeach()

# Generate File referencing Resources
configure_file(rc.cpp.in rc.cpp)

# Include local Headers
include_directories("${PROJECT_SOURCE_DIR}")

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
	${COMPILED_RESOURCES}
)

# Specify Link-Libraries
target_link_libraries(decklink-debugger dl pthread microhttpd png)

install(PROGRAMS decklink-debugger DESTINATION bin)
