set(tuuvm_sources
    arrayList.c
    assert.c
    context.c
    heap.c
    string.c
    tuple.c
    type.c
)

add_definitions(-DBUILD_LIB_TUUVM)
add_library(tuuvm SHARED ${tuuvm_sources})