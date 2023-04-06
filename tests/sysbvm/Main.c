#include "TestMacros.h"

const char *sysbvm_test_currentTestSuiteName;
const char *sysbvm_test_currentTestCaseName;
int sysbvm_test_currentHasError;
int sysbvm_test_runCount;
int sysbvm_test_errorCount;

sysbvm_context_t *sysbvm_test_context;

TEST_SUITE_FIXTURE_INITIALIZE(TuuvmCore)
{
    sysbvm_test_context = sysbvm_context_create();
}

TEST_SUITE_FIXTURE_SHUTDOWN(TuuvmCore)
{
    sysbvm_context_destroy(sysbvm_test_context);
}

#define TEST_SUITE_NAME(name) DECLARE_TEST_SUITE(name);
#include "TestSuites.inc"
#undef TEST_SUITE_NAME


int main(int argc, const char *argv[])
{
    (void)argc;
    (void)argv;

#define TEST_SUITE_NAME(name) RUN_TEST_SUITE(name);
#include "TestSuites.inc"
#undef TEST_SUITE_NAME

    if(sysbvm_test_errorCount)
        printf("Run %d tests. Errors: %d\n", sysbvm_test_runCount, sysbvm_test_errorCount);
    else
        printf("Run %d tests. All tests passed.\n", sysbvm_test_runCount);

    return sysbvm_test_errorCount > 0 ? 1 : 0;
}
