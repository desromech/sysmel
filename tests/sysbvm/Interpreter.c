#include "TestMacros.h"
#include "sysbvm/interpreter.h"
#include "sysbvm/environment.h"
#include "sysbvm/string.h"
#include "sysbvm/gc.h"

static sysbvm_tuple_t testAnalyzeAndEvaluate(const char *sourceCode)
{
    return sysbvm_interpreter_analyzeAndEvaluateCStringWithEnvironment(sysbvm_test_context,
        sysbvm_environment_createDefaultForEvaluation(sysbvm_test_context),
        sourceCode, "test", "tlisp");
}

static sysbvm_tuple_t testAnalyzeAndEvaluateSysmel(const char *sourceCode)
{
    return sysbvm_interpreter_analyzeAndEvaluateCStringWithEnvironment(sysbvm_test_context,
        sysbvm_environment_createDefaultForEvaluation(sysbvm_test_context),
        sourceCode, "test", "sysmel");
}

TEST_SUITE(Interpreter)
{
    TEST_CASE_WITH_FIXTURE(EmptyString, TuuvmCore)
    {
        TEST_ASSERT_EQUALS(SYSBVM_VOID_TUPLE, testAnalyzeAndEvaluate(""));
    }

    TEST_CASE_WITH_FIXTURE(StringSymbol, TuuvmCore)
    {
        sysbvm_gc_lock(sysbvm_test_context);
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "first"), testAnalyzeAndEvaluate("#first"));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "second"), testAnalyzeAndEvaluate("#first #second"));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "first"), testAnalyzeAndEvaluateSysmel("#first"));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "second"), testAnalyzeAndEvaluateSysmel("#first . #second"));
        sysbvm_gc_unlock(sysbvm_test_context);
    }

    TEST_CASE_WITH_FIXTURE(Identifier, TuuvmCore)
    {
        TEST_ASSERT_EQUALS(SYSBVM_NULL_TUPLE, testAnalyzeAndEvaluate("nil"));
        TEST_ASSERT_EQUALS(SYSBVM_FALSE_TUPLE, testAnalyzeAndEvaluate("false"));
        TEST_ASSERT_EQUALS(SYSBVM_TRUE_TUPLE, testAnalyzeAndEvaluate("true"));
        TEST_ASSERT_EQUALS(SYSBVM_VOID_TUPLE, testAnalyzeAndEvaluate("void"));

        TEST_ASSERT_EQUALS(SYSBVM_NULL_TUPLE, testAnalyzeAndEvaluateSysmel("nil"));
        TEST_ASSERT_EQUALS(SYSBVM_FALSE_TUPLE, testAnalyzeAndEvaluateSysmel("false"));
        TEST_ASSERT_EQUALS(SYSBVM_TRUE_TUPLE, testAnalyzeAndEvaluateSysmel("true"));
        TEST_ASSERT_EQUALS(SYSBVM_VOID_TUPLE, testAnalyzeAndEvaluateSysmel("void"));
    }

    TEST_CASE_WITH_FIXTURE(FunctionApplication, TuuvmCore)
    {
        TEST_ASSERT_EQUALS(sysbvm_tuple_identityHash(SYSBVM_NULL_TUPLE), sysbvm_tuple_size_decode(testAnalyzeAndEvaluate("(identityHash nil)")));
        TEST_ASSERT_EQUALS(sysbvm_tuple_identityHash(SYSBVM_FALSE_TUPLE), sysbvm_tuple_size_decode(testAnalyzeAndEvaluate("(identityHash false)")));
        TEST_ASSERT_EQUALS(sysbvm_tuple_identityHash(SYSBVM_TRUE_TUPLE), sysbvm_tuple_size_decode(testAnalyzeAndEvaluate("(identityHash true)")));
        TEST_ASSERT_EQUALS(sysbvm_tuple_identityHash(SYSBVM_VOID_TUPLE), sysbvm_tuple_size_decode(testAnalyzeAndEvaluate("(identityHash void)")));

        TEST_ASSERT_EQUALS(sysbvm_tuple_identityHash(SYSBVM_NULL_TUPLE), sysbvm_tuple_size_decode(testAnalyzeAndEvaluateSysmel("identityHash(nil)")));
        TEST_ASSERT_EQUALS(sysbvm_tuple_identityHash(SYSBVM_FALSE_TUPLE), sysbvm_tuple_size_decode(testAnalyzeAndEvaluateSysmel("identityHash(false)")));
        TEST_ASSERT_EQUALS(sysbvm_tuple_identityHash(SYSBVM_TRUE_TUPLE), sysbvm_tuple_size_decode(testAnalyzeAndEvaluateSysmel("identityHash(true)")));
        TEST_ASSERT_EQUALS(sysbvm_tuple_identityHash(SYSBVM_VOID_TUPLE), sysbvm_tuple_size_decode(testAnalyzeAndEvaluateSysmel("identityHash(void)")));
    }

    TEST_CASE_WITH_FIXTURE(NullaryLambdaApplication, TuuvmCore)
    {
        TEST_ASSERT_EQUALS(SYSBVM_NULL_TUPLE, testAnalyzeAndEvaluate("((lambda () nil))"));
        TEST_ASSERT_EQUALS(SYSBVM_FALSE_TUPLE, testAnalyzeAndEvaluate("((lambda () false))"));
        TEST_ASSERT_EQUALS(SYSBVM_TRUE_TUPLE, testAnalyzeAndEvaluate("((lambda () true))"));
        TEST_ASSERT_EQUALS(SYSBVM_VOID_TUPLE, testAnalyzeAndEvaluate("((lambda () void))"));

        TEST_ASSERT_EQUALS(SYSBVM_NULL_TUPLE, testAnalyzeAndEvaluateSysmel("{| nil} ()"));
        TEST_ASSERT_EQUALS(SYSBVM_FALSE_TUPLE, testAnalyzeAndEvaluateSysmel("{| false} ()"));
        TEST_ASSERT_EQUALS(SYSBVM_TRUE_TUPLE, testAnalyzeAndEvaluateSysmel("{| true} ()"));
        TEST_ASSERT_EQUALS(SYSBVM_VOID_TUPLE, testAnalyzeAndEvaluateSysmel("{| void} ()"));
    }

    TEST_CASE_WITH_FIXTURE(IdentityLambdaApplication, TuuvmCore)
    {
        TEST_ASSERT_EQUALS(SYSBVM_NULL_TUPLE, testAnalyzeAndEvaluate("((lambda (x) x) nil)"));
        TEST_ASSERT_EQUALS(SYSBVM_FALSE_TUPLE, testAnalyzeAndEvaluate("((lambda (x) x) false)"));
        TEST_ASSERT_EQUALS(SYSBVM_TRUE_TUPLE, testAnalyzeAndEvaluate("((lambda (x) x) true)"));
        TEST_ASSERT_EQUALS(SYSBVM_VOID_TUPLE, testAnalyzeAndEvaluate("((lambda (x) x) void)"));

        TEST_ASSERT_EQUALS(SYSBVM_NULL_TUPLE, testAnalyzeAndEvaluateSysmel("{:x | x} (nil)"));
        TEST_ASSERT_EQUALS(SYSBVM_FALSE_TUPLE, testAnalyzeAndEvaluateSysmel("{:x | x} (false)"));
        TEST_ASSERT_EQUALS(SYSBVM_TRUE_TUPLE, testAnalyzeAndEvaluateSysmel("{:x | x} (true)"));
        TEST_ASSERT_EQUALS(SYSBVM_VOID_TUPLE, testAnalyzeAndEvaluateSysmel("{:x | x} (void)"));
    }

    TEST_CASE_WITH_FIXTURE(IfExpression, TuuvmCore)
    {
        TEST_ASSERT_EQUALS(sysbvm_tuple_integer_encodeSmall(1), testAnalyzeAndEvaluate("(if:then:else: true 1 2)"));
        TEST_ASSERT_EQUALS(sysbvm_tuple_integer_encodeSmall(2), testAnalyzeAndEvaluate("(if:then:else: false 1 2)"));

        TEST_ASSERT_EQUALS(sysbvm_tuple_integer_encodeSmall(1), testAnalyzeAndEvaluate("((lambda (cond trueValue falseValue) (if:then:else: cond trueValue falseValue)) true 1 2)"));
        TEST_ASSERT_EQUALS(sysbvm_tuple_integer_encodeSmall(2), testAnalyzeAndEvaluate("((lambda (cond trueValue falseValue) (if:then:else: cond trueValue falseValue)) false 1 2)"));
    }
    
    TEST_CASE_WITH_FIXTURE(Define, TuuvmCore)
    {
        TEST_ASSERT_EQUALS(SYSBVM_FALSE_TUPLE, testAnalyzeAndEvaluate("(define myvar false) myvar"));
        TEST_ASSERT_EQUALS(SYSBVM_FALSE_TUPLE, testAnalyzeAndEvaluate("(define (myfunction) false) (myfunction)"));

        TEST_ASSERT_EQUALS(SYSBVM_FALSE_TUPLE, testAnalyzeAndEvaluateSysmel("let: #myvar with: false. myvar"));
        TEST_ASSERT_EQUALS(SYSBVM_FALSE_TUPLE, testAnalyzeAndEvaluateSysmel("let: #myfunction with: {| false}. myfunction()"));
    }
}
