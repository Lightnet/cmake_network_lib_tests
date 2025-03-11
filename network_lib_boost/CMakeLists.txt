# Minimum CMake version
cmake_minimum_required(VERSION 3.14)

# Project name and languages
project(network_test LANGUAGES CXX)

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

# Define _WIN32_WINNT for Windows builds
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)  # Windows 7 or later
endif()

# Download CPM.cmake for dependency management
file(DOWNLOAD
    "https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.7/CPM.cmake"
    "${CMAKE_BINARY_DIR}/CPM_0.40.7.cmake"
)
include(${CMAKE_BINARY_DIR}/CPM_0.40.7.cmake)

# Add Boost from GitHub with minimal components for Asio networking
CPMAddPackage(
    NAME Boost
    GITHUB_REPOSITORY "boostorg/boost"
    GIT_TAG "boost-1.87.0"
    OPTIONS
        "BOOST_ENABLE_CMAKE ON"           # Enable CMake support
        "BOOST_INCLUDE_LIBRARIES asio;system;thread"  # Core networking libs
)

# Find required system threading libraries (e.g., pthread on Unix, or Windows threads)
find_package(Threads REQUIRED)

# Add executable for the network test
add_executable(network_test src/main.cpp)

# Link Boost and threading libraries to the executable
target_link_libraries(network_test PRIVATE 
  Boost::asio 
  Boost::system 
  #Boost::thread 
  Threads::Threads
)

# Include directories (handled by Boost:: targets, but explicit if needed)
target_include_directories(network_test PRIVATE ${Boost_SOURCE_DIR})