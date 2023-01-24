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

    TEST_CASE_WITH_FIXTURE(Addition, TuuvmCore)
    {
        // Neutral element
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("0"), tuuvm_integer_add(tuuvm_test_context, INTEGER("0"), INTEGER("0")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("1"), tuuvm_integer_add(tuuvm_test_context, INTEGER("0"), INTEGER("1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("1"), tuuvm_integer_add(tuuvm_test_context, INTEGER("1"), INTEGER("0")));

        // Cancellation
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("0"), tuuvm_integer_add(tuuvm_test_context, INTEGER("-1"), INTEGER("1")));

        // Simple sums
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("2"), tuuvm_integer_add(tuuvm_test_context, INTEGER("1"), INTEGER("1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("49"), tuuvm_integer_add(tuuvm_test_context, INTEGER("7"), INTEGER("42")));

        // Differing sign sums
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("-1"), tuuvm_integer_add(tuuvm_test_context, INTEGER("2"), INTEGER("-3")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("1"), tuuvm_integer_add(tuuvm_test_context, INTEGER("3"), INTEGER("-2")));
    }

    TEST_CASE_WITH_FIXTURE(Subtraction, TuuvmCore)
    {
        // Neutral element
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("0"), tuuvm_integer_subtract(tuuvm_test_context, INTEGER("0"), INTEGER("0")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("1"), tuuvm_integer_subtract(tuuvm_test_context, INTEGER("1"), INTEGER("0")));

        TEST_ASSERT_INTEGER_EQUALS(INTEGER("-1"), tuuvm_integer_subtract(tuuvm_test_context, INTEGER("0"), INTEGER("1")));

        // Simple subtractions
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("0"), tuuvm_integer_subtract(tuuvm_test_context, INTEGER("1"), INTEGER("1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("-35"), tuuvm_integer_subtract(tuuvm_test_context, INTEGER("7"), INTEGER("42")));

        TEST_ASSERT_INTEGER_EQUALS(INTEGER("5"), tuuvm_integer_subtract(tuuvm_test_context, INTEGER("2"), INTEGER("-3")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("5"), tuuvm_integer_subtract(tuuvm_test_context, INTEGER("3"), INTEGER("-2")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("-2"), tuuvm_integer_subtract(tuuvm_test_context, INTEGER("-1"), INTEGER("1")));
    }

    TEST_CASE_WITH_FIXTURE(Multiplication, TuuvmCore)
    {
        // Neutral element
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("1"), tuuvm_integer_multiply(tuuvm_test_context, INTEGER("1"), INTEGER("1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("2"), tuuvm_integer_multiply(tuuvm_test_context, INTEGER("2"), INTEGER("1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("2"), tuuvm_integer_multiply(tuuvm_test_context, INTEGER("1"), INTEGER("2")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("3"), tuuvm_integer_multiply(tuuvm_test_context, INTEGER("3"), INTEGER("1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("3"), tuuvm_integer_multiply(tuuvm_test_context, INTEGER("1"), INTEGER("3")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("-2"), tuuvm_integer_multiply(tuuvm_test_context, INTEGER("-2"), INTEGER("1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("-2"), tuuvm_integer_multiply(tuuvm_test_context, INTEGER("1"), INTEGER("-2")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("-3"), tuuvm_integer_multiply(tuuvm_test_context, INTEGER("-3"), INTEGER("1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("-3"), tuuvm_integer_multiply(tuuvm_test_context, INTEGER("1"), INTEGER("-3")));

        // Absorbing element
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("0"), tuuvm_integer_multiply(tuuvm_test_context, INTEGER("0"), INTEGER("1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("0"), tuuvm_integer_multiply(tuuvm_test_context, INTEGER("1"), INTEGER("0")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("0"), tuuvm_integer_multiply(tuuvm_test_context, INTEGER("0"), INTEGER("-1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("0"), tuuvm_integer_multiply(tuuvm_test_context, INTEGER("1"), INTEGER("0")));

        // Negation
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("1"), tuuvm_integer_multiply(tuuvm_test_context, INTEGER("-1"), INTEGER("-1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("-1"), tuuvm_integer_multiply(tuuvm_test_context, INTEGER("1"), INTEGER("-1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("-1"), tuuvm_integer_multiply(tuuvm_test_context, INTEGER("-1"), INTEGER("1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("-2"), tuuvm_integer_multiply(tuuvm_test_context, INTEGER("-1"), INTEGER("2")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("-2"), tuuvm_integer_multiply(tuuvm_test_context, INTEGER("2"), INTEGER("-1")));

        // Simple multiplications.
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("6"), tuuvm_integer_multiply(tuuvm_test_context, INTEGER("2"), INTEGER("3")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("49"), tuuvm_integer_multiply(tuuvm_test_context, INTEGER("7"), INTEGER("7")));
    }

    TEST_CASE_WITH_FIXTURE(Division, TuuvmCore)
    {
        // Neutral element
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("2"), tuuvm_integer_divide(tuuvm_test_context, INTEGER("2"), INTEGER("1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("3"), tuuvm_integer_divide(tuuvm_test_context, INTEGER("3"), INTEGER("1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("-2"), tuuvm_integer_divide(tuuvm_test_context, INTEGER("-2"), INTEGER("1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("-3"), tuuvm_integer_divide(tuuvm_test_context, INTEGER("-3"), INTEGER("1")));

        // Division from zero
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("0"), tuuvm_integer_divide(tuuvm_test_context, INTEGER("0"), INTEGER("1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("0"), tuuvm_integer_divide(tuuvm_test_context, INTEGER("0"), INTEGER("-1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("0"), tuuvm_integer_divide(tuuvm_test_context, INTEGER("0"), INTEGER("2")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("0"), tuuvm_integer_divide(tuuvm_test_context, INTEGER("0"), INTEGER("-2")));

        // Negation
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("1"), tuuvm_integer_divide(tuuvm_test_context, INTEGER("-1"), INTEGER("-1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("-1"), tuuvm_integer_divide(tuuvm_test_context, INTEGER("-1"), INTEGER("1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("-1"), tuuvm_integer_divide(tuuvm_test_context, INTEGER("1"), INTEGER("-1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("2"), tuuvm_integer_divide(tuuvm_test_context, INTEGER("2"), INTEGER("-1")));

        // Simple divisions
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("3"), tuuvm_integer_divide(tuuvm_test_context, INTEGER("6"), INTEGER("2")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("2"), tuuvm_integer_divide(tuuvm_test_context, INTEGER("6"), INTEGER("3")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("7"), tuuvm_integer_divide(tuuvm_test_context, INTEGER("49"), INTEGER("7")));
    }

    TEST_CASE_WITH_FIXTURE(Factorial, TuuvmCore)
    {
        // Neutral element
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("1"), tuuvm_integer_factorial(tuuvm_test_context, INTEGER("0")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("1"), tuuvm_integer_factorial(tuuvm_test_context, INTEGER("1")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("2"), tuuvm_integer_factorial(tuuvm_test_context, INTEGER("2")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("6"), tuuvm_integer_factorial(tuuvm_test_context, INTEGER("3")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("24"), tuuvm_integer_factorial(tuuvm_test_context, INTEGER("4")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("120"), tuuvm_integer_factorial(tuuvm_test_context, INTEGER("5")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("720"), tuuvm_integer_factorial(tuuvm_test_context, INTEGER("6")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("5040"), tuuvm_integer_factorial(tuuvm_test_context, INTEGER("7")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("40320"), tuuvm_integer_factorial(tuuvm_test_context, INTEGER("8")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("362880"), tuuvm_integer_factorial(tuuvm_test_context, INTEGER("9")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("3628800"), tuuvm_integer_factorial(tuuvm_test_context, INTEGER("10")));

        TEST_ASSERT_INTEGER_EQUALS(INTEGER("2432902008176640000"), tuuvm_integer_factorial(tuuvm_test_context, INTEGER("20")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("265252859812191058636308480000000"), tuuvm_integer_factorial(tuuvm_test_context, INTEGER("30")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("815915283247897734345611269596115894272000000000"), tuuvm_integer_factorial(tuuvm_test_context, INTEGER("40")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("30414093201713378043612608166064768844377641568960512000000000000"), tuuvm_integer_factorial(tuuvm_test_context, INTEGER("50")));
        TEST_ASSERT_INTEGER_EQUALS(INTEGER("93326215443944152681699238856266700490715968264381621468592963895217599993229915608941463976156518286253697920827223758251185210916864000000000000000000000000"), tuuvm_integer_factorial(tuuvm_test_context, INTEGER("100")));
    }

}
