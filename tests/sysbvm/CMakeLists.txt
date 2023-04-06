set(SYSBVM_TESTS_SOURCES
    ArrayList.c
    Immediate.c
    Integer.c
    Interpreter.c
    Main.c
    String.c
    Scanner.c
    SysmelParser.c
    Parser.c
)

add_executable(TestSysbvm ${SYSBVM_TESTS_SOURCES})
target_link_libraries(TestSysbvm sysbvm)
add_test(NAME TestSysbvm COMMAND TestSysbvm)