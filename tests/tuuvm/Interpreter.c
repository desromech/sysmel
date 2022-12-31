#include "TestMacros.h"
#include "tuuvm/interpreter.h"
#include "tuuvm/environment.h"
#include "tuuvm/string.h"

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

    TEST_CASE_WITH_FIXTURE(Symbol, TuuvmCore)
    {
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "first"), testAnalyzeAndEvaluate("#first"));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "second"), testAnalyzeAndEvaluate("#first #second"));
    }

    TEST_CASE_WITH_FIXTURE(Identifier, TuuvmCore)
    {
        TEST_ASSERT_EQUALS(TUUVM_NULL_TUPLE, testAnalyzeAndEvaluate("nil"));
        TEST_ASSERT_EQUALS(TUUVM_FALSE_TUPLE, testAnalyzeAndEvaluate("false"));
        TEST_ASSERT_EQUALS(TUUVM_TRUE_TUPLE, testAnalyzeAndEvaluate("true"));
        TEST_ASSERT_EQUALS(TUUVM_VOID_TUPLE, testAnalyzeAndEvaluate("void"));
    }

    TEST_CASE_WITH_FIXTURE(FunctionApplication, TuuvmCore)
    {
        TEST_ASSERT_EQUALS(tuuvm_tuple_identityHash(TUUVM_NULL_TUPLE), tuuvm_tuple_size_decode(testAnalyzeAndEvaluate("(RawTuple::identityHash nil)")));
        TEST_ASSERT_EQUALS(tuuvm_tuple_identityHash(TUUVM_FALSE_TUPLE), tuuvm_tuple_size_decode(testAnalyzeAndEvaluate("(RawTuple::identityHash false)")));
        TEST_ASSERT_EQUALS(tuuvm_tuple_identityHash(TUUVM_TRUE_TUPLE), tuuvm_tuple_size_decode(testAnalyzeAndEvaluate("(RawTuple::identityHash true)")));
        TEST_ASSERT_EQUALS(tuuvm_tuple_identityHash(TUUVM_VOID_TUPLE), tuuvm_tuple_size_decode(testAnalyzeAndEvaluate("(RawTuple::identityHash void)")));
    }

    TEST_CASE_WITH_FIXTURE(NullaryLambdaApplication, TuuvmCore)
    {
        TEST_ASSERT_EQUALS(TUUVM_NULL_TUPLE, testAnalyzeAndEvaluate("((lambda () nil))"));
        TEST_ASSERT_EQUALS(TUUVM_FALSE_TUPLE, testAnalyzeAndEvaluate("((lambda () false))"));
        TEST_ASSERT_EQUALS(TUUVM_TRUE_TUPLE, testAnalyzeAndEvaluate("((lambda () true))"));
        TEST_ASSERT_EQUALS(TUUVM_VOID_TUPLE, testAnalyzeAndEvaluate("((lambda () void))"));
    }

    TEST_CASE_WITH_FIXTURE(IdentityLambdaApplication, TuuvmCore)
    {
        TEST_ASSERT_EQUALS(TUUVM_NULL_TUPLE, testAnalyzeAndEvaluate("((lambda (x) x) nil)"));
        TEST_ASSERT_EQUALS(TUUVM_FALSE_TUPLE, testAnalyzeAndEvaluate("((lambda (x) x) false)"));
        TEST_ASSERT_EQUALS(TUUVM_TRUE_TUPLE, testAnalyzeAndEvaluate("((lambda (x) x) true)"));
        TEST_ASSERT_EQUALS(TUUVM_VOID_TUPLE, testAnalyzeAndEvaluate("((lambda (x) x) void)"));
    }

    TEST_CASE_WITH_FIXTURE(Define, TuuvmCore)
    {
        TEST_ASSERT_EQUALS(TUUVM_FALSE_TUPLE, testAnalyzeAndEvaluate("(define myvar false) myvar"));
        TEST_ASSERT_EQUALS(TUUVM_FALSE_TUPLE, testAnalyzeAndEvaluate("(define (myfunction) false) (myfunction)"));
    }
}
