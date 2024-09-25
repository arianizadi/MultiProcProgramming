cmake_minimum_required(VERSION 3.10)

# Set the project name
project(MPIHelloWorld)

# Find MPI package
find_package(MPI REQUIRED)

# Get all C files in the current directory
file(GLOB C_FILES "*.c")

# Add an executable for each C file
foreach(C_FILE ${C_FILES})
    get_filename_component(EXECUTABLE_NAME ${C_FILE} NAME_WE)
    add_executable(${EXECUTABLE_NAME} ${C_FILE})
    target_link_libraries(${EXECUTABLE_NAME} MPI::MPI_C)
endforeach()

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

# Setup ccache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()