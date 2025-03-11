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

# Add ENet dependency using CPM
CPMAddPackage(
    NAME enet
    GITHUB_REPOSITORY lsalzman/enet
    GIT_TAG v1.3.18
    OPTIONS
        "ENET_STATIC ON"  # Build as static library
)

# Create executable
add_executable(network_test
    main.c
)

# Link ENet to our executable
target_link_libraries(network_test PRIVATE enet)

# Include ENet headers
target_include_directories(network_test PRIVATE
    ${enet_SOURCE_DIR}/include
)

# Platform-specific settings
if(WIN32)
    target_link_libraries(network_test PRIVATE ws2_32 winmm)
endif()