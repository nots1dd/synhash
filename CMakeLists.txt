# Minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(SyntaxHighlighter VERSION 1.0)

# Set C standard to C99
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Include directories
include_directories(${PROJECT_SOURCE_DIR})

# Source files
set(SRCS yaml-parser.c src/hashtable.c)

# Header files
set(HEADERS include/hashtable.h )

# Define the executable
add_executable(syntax_highlighter ${SRCS} ${HEADERS})

# Link libraries
target_link_libraries(syntax_highlighter ncurses yaml)
