set(TUUVM_TESTS_SOURCES
    ArrayList.c
    Immediate.c
    Integer.c
    Interpreter.c
    Main.c
    String.c
    Scanner.c
    Parser.c
)

add_executable(TestTuuvm ${TUUVM_TESTS_SOURCES})
target_link_libraries(TestTuuvm tuuvm)
add_test(NAME TestTuuvm COMMAND TestTuuvm)