# Specify the minimum version of CMake required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(FileMonitorDaemon VERSION 1.0)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find pthread library
find_package(Threads REQUIRED)

# Add the executable, including the source files from the src/ directory
add_executable(Linux-FileOwl 
    src/file_owl.cpp
)

# Link the pthread library
target_link_libraries(Linux-FileOwl Threads::Threads)

# Optionally, set install rules if needed
install(TARGETS Linux-FileOwl DESTINATION /usr/local/bin)