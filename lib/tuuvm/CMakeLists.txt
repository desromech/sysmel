set(tuuvm_sources
    array.c
    arrayList.c
    arraySlice.c
    assert.c
    context.c
    errors.c
    function.c
    heap.c
    parser.c
    scanner.c
    set.c
    sourceCode.c
    sourcePosition.c
    string.c
    token.c
    tuple.c
    type.c
)

add_definitions(-DBUILD_TUUVM)
add_library(tuuvm SHARED ${tuuvm_sources})