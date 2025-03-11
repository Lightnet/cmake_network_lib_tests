# Minimum CMake version
cmake_minimum_required(VERSION 3.14)

# Project name and languages
project(network_test LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Set output directories for binaries, libraries, and archives
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Download CPM.cmake for dependency management
file(DOWNLOAD
    "https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.7/CPM.cmake"
    "${CMAKE_BINARY_DIR}/CPM_0.40.7.cmake"
)
include(${CMAKE_BINARY_DIR}/CPM_0.40.7.cmake)

# Add ENet dependency
CPMAddPackage(
    NAME enet
    GITHUB_REPOSITORY zpl-c/enet
    GIT_TAG v2.6.2
    OPTIONS
        "ENET_STATIC ON"
)

# Add Raylib dependency
CPMAddPackage(
    NAME raylib
    GITHUB_REPOSITORY raysan5/raylib
    GIT_TAG 5.5  # Note: Using 5.0 as 5.5 isn't an official tag yet as of March 2025
    OPTIONS
        "BUILD_EXAMPLES OFF"
)

# Add Raygui dependency
CPMAddPackage(
    NAME raygui
    GITHUB_REPOSITORY raysan5/raygui
    GIT_TAG 4.0
    DOWNLOAD_ONLY YES
)

# Create server executable
add_executable(server src/server.c)
target_link_libraries(server PRIVATE "${CMAKE_BINARY_DIR}/$<CONFIG>/enet.lib")
target_include_directories(server PRIVATE ${enet_SOURCE_DIR}/include)
if(WIN32)
    target_link_libraries(server PRIVATE ws2_32 winmm)
endif()

# Create client executable
add_executable(client src/client.c)
target_link_libraries(client PRIVATE 
    "${CMAKE_BINARY_DIR}/$<CONFIG>/enet.lib"
    raylib
)
target_include_directories(client PRIVATE 
    ${enet_SOURCE_DIR}/include
    ${raylib_SOURCE_DIR}/src
    ${raygui_SOURCE_DIR}/src
)
if(WIN32)
    target_link_libraries(client PRIVATE ws2_32 winmm)
endif()