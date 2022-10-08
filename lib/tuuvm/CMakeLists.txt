set(tuuvm_sources
    array.c
    arrayList.c
    arraySlice.c
    assert.c
    context.c
    errors.c
    function.c
    heap.c
    set.c
    string.c
    tuple.c
    type.c
)

add_definitions(-DBUILD_LIB_TUUVM)
add_library(tuuvm SHARED ${tuuvm_sources})