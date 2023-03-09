set(tuuvm_sources
    array.c
    arrayList.c
    arraySlice.c
    assert.c
    ast.c
    boolean.c
    context.c
    dictionary.c
    environment.c
    errors.c
    filesystem.c
    float.c
    function.c
    gc.c
    heap.c
    integer.c
    interpreter.c
    io.c
    macro.c
    parser.c
    primitiveIntegers.c
    scanner.c
    set.c
    sourceCode.c
    sourcePosition.c
    stackFrame.c
    string.c
    stringBuilder.c
    sysmelParser.c
    token.c
    tuple.c
    type.c
)

add_definitions(-DBUILD_TUUVM)
add_library(tuuvm STATIC ${tuuvm_sources})
target_link_libraries(tuuvm ${TUUVM_DEP_LIBS})