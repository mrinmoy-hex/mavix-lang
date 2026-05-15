cmake_minimum_required(VERSION 3.19.2)

project(mavix 
    VERSION 0.1.0 
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Collect all C files automatically
file(GLOB SOURCES "src/*.c")

# Add the executable target
add_executable(mavix ${SOURCES})

# Use target-specific configurations
target_include_directories(mavix PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_options(mavix PRIVATE -Wall -Wextra -O2 -pedantic)
