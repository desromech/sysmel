set(TUUVM_TESTS_SOURCES
    Main.c
)

add_executable(TestTuuvm ${TUUVM_TESTS_SOURCES})
target_link_libraries(TestTuuvm tuuvm)
add_test(NAME TestTuuvm COMMAND TestTuuvm)