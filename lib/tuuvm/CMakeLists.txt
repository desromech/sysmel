set(tuuvm_sources
    heap.c
    tuple.c
)

add_definitions(-DBUILD_LIB_TUUVM)
add_library(tuuvm SHARED ${tuuvm_sources})