# AbstractGPU cmake build script.
cmake_minimum_required(VERSION 3.0)
project(Tuuvm C)

if(MSVC)
    set(CMAKE_C_FLAGS "/std:c11 ${CMAKE_C_FLAGS}")
else()
    set(CMAKE_C_FLAGS "-Wall -Wextra ${CMAKE_C_FLAGS}")
endif()

if(APPLE)
	set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

include(CTest)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    .
)

set(MainOutputDirectory "${Tuuvm_BINARY_DIR}/dist")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MainOutputDirectory} )
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${MainOutputDirectory} )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MainOutputDirectory} )

add_subdirectory(lib)
add_subdirectory(apps)
add_subdirectory(tests)
