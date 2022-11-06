set(tuuvm_sources
    array.c
    arrayList.c
    arraySlice.c
    assert.c
    ast.c
    context.c
    dictionary.c
    environment.c
    errors.c
    function.c
    heap.c
    interpreter.c
    io.c
    macro.c
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