#include "TestMacros.h"
#include "tuuvm/integer.h"

#define TEST_ASSERT_INTEGER_EQUALS(expected, gotten) TEST_ASSERT_DESCRIPTION(tuuvm_integer_equals(tuuvm_test_context, (expected), (gotten) == TUUVM_TRUE_TUPLE), "Obtained value is not equal to " #expected)

#define INTEGER(string) tuuvm_integer_parseCString(tuuvm_test_context, string)

TEST_SUITE(Integer)
{
    TEST_CASE_WITH_FIXTURE(Negation, TuuvmCore)
    {
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("0"), INTEGER("-0"));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("0"), tuuvm_integer_negate(tuuvm_test_context, INTEGER("0")));

        TEST_ASSERT_INTEGER_EQUALS(INTEGER("-1"), tuuvm_integer_negate(tuuvm_test_context, INTEGER("1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("1"), tuuvm_integer_negate(tuuvm_test_context, INTEGER("-1")));

        TEST_ASSERT_INTEGER_EQUALS(INTEGER("-2"), tuuvm_integer_negate(tuuvm_test_context, INTEGER("2")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("2"), tuuvm_integer_negate(tuuvm_test_context, INTEGER("-2")));
    }
}
