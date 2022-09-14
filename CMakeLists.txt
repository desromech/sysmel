# AbstractGPU cmake build script.
cmake_minimum_required(VERSION 3.0)
project(Tuuvm C)

set(CMAKE_C_FLAGS "-Wall -Wextra ${CMAKE_C_FLAGS}")

if(APPLE)
	set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    .
)

add_subdirectory(lib)
add_subdirectory(apps)
add_subdirectory(tests)
