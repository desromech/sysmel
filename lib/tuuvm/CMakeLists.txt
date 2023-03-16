set(tuuvm_sources
    array.c
    arrayList.c
    arraySlice.c
    assert.c
    association.c
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
    message.c
    parser.c
    primitiveIntegers.c
    scanner.c
    set.c
    sourceCode.c
    sourcePosition.c
    stackFrame.c
    string.c
    stringStream.c
    sysmelParser.c
    token.c
    tuple.c
    type.c
)

set(USE_UNITY_BUILD False)
if(USE_UNITY_BUILD)
    set(tuuvm_sources unityBuild.c)
endif()

add_definitions(-DBUILD_TUUVM)
add_library(tuuvm STATIC ${tuuvm_sources})
target_link_libraries(tuuvm ${TUUVM_DEP_LIBS})