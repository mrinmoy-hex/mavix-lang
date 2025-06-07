cmake_minimum_required(VERSION 3.19.2)

# Project name and version
project(mavix VERSION 0.1.0)

# Set the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Collect the source files
file(GLOB SOURCES "src/*.c")

# Add the executable
add_executable("mavix" ${SOURCES})

# Include directories for header files
include_directories("include")

# Compiler flags
add_compile_options(-Wall -Wextra -O2 -pedantic)
