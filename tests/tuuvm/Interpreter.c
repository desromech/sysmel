#include "TestMacros.h"
#include "tuuvm/interpreter.h"
#include "tuuvm/environment.h"

static tuuvm_tuple_t testAnalyzeAndEvaluate(const char *sourceCode)
{
    return tuuvm_interpreter_analyzeAndEvaluateCStringWithEnvironment(tuuvm_test_context,
        tuuvm_environment_createDefaultForEvaluation(tuuvm_test_context),
        sourceCode, "test");
}

TEST_SUITE(Interpreter)
{
    TEST_CASE_WITH_FIXTURE(EmptyString, TuuvmCore)
    {
        TEST_ASSERT_EQUALS(TUUVM_VOID_TUPLE, testAnalyzeAndEvaluate(""));
    }
}
