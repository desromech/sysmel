#include "TestMacros.h"

int tuuvm_test_currentHasError;
int tuuvm_test_runCount;
int tuuvm_test_errorCount;

TEST_SUITE(StringTests)
{
}

int main(int argc, const char *argv[])
{
    (void)argc;
    (void)argv;
    RUN_TEST_SUITE(StringTests);

    printf("Run %d tests\n", tuuvm_test_runCount);
    if(tuuvm_test_errorCount)
        printf("Error %d tests\n", tuuvm_test_errorCount);
    return tuuvm_test_errorCount > 0 ? 1 : 0;
}
