#include "TestMacros.h"
#include <tuuvm/context.h>
#include <tuuvm/string.h>

int tuuvm_test_currentHasError;
int tuuvm_test_runCount;
int tuuvm_test_errorCount;

tuuvm_context_t *tuuvm_test_context;

TEST_SUITE_FIXTURE_INITIALIZE(TuuvmContext)
{
    tuuvm_test_context = tuuvm_context_create();
}

TEST_SUITE_FIXTURE_SHUTDOWN(TuuvmContext)
{
    tuuvm_context_destroy(tuuvm_test_context);
}

TEST_SUITE_WITH_FIXTURE(StringTests, TuuvmContext)
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
