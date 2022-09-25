#include "TestMacros.h"
#include <tuuvm/string.h>

TEST_SUITE(String)
{
    TEST_CASE_WITH_FIXTURE(CreateWithString, TuuvmCore)
    {
        tuuvm_tuple_t string = tuuvm_string_createWithString(tuuvm_test_context, 2, "ABC");
        TEST_ASSERT(tuuvm_tuple_isBytes(string));
        TEST_ASSERT_EQUALS(2, tuuvm_tuple_getSizeInBytes(string));
    }

    TEST_CASE_WITH_FIXTURE(CreateWithCString, TuuvmCore)
    {
        tuuvm_tuple_t string = tuuvm_string_createWithCString(tuuvm_test_context, "Hello World\n");
        TEST_ASSERT(tuuvm_tuple_isBytes(string));
        TEST_ASSERT_EQUALS(12, tuuvm_tuple_getSizeInBytes(string));
    }
}


TEST_SUITE(Symbol)
{
    TEST_CASE_WITH_FIXTURE(InternWithString, TuuvmCore)
    {
        tuuvm_tuple_t symbol = tuuvm_symbol_internWithString(tuuvm_test_context, 2, "ABC");
        TEST_ASSERT(tuuvm_tuple_isBytes(symbol));
        TEST_ASSERT_EQUALS(2, tuuvm_tuple_getSizeInBytes(symbol));

        tuuvm_tuple_t secondSymbol = tuuvm_symbol_internWithString(tuuvm_test_context, 2, "ABC");
        TEST_ASSERT(tuuvm_tuple_isBytes(secondSymbol));
        TEST_ASSERT_EQUALS(2, tuuvm_tuple_getSizeInBytes(secondSymbol));

        TEST_ASSERT_EQUALS(symbol, secondSymbol);
    }

    TEST_CASE_WITH_FIXTURE(InternWithCString, TuuvmCore)
    {
        tuuvm_tuple_t symbol = tuuvm_symbol_internWithCString(tuuvm_test_context, "Hello World\n");
        TEST_ASSERT(tuuvm_tuple_isBytes(symbol));
        TEST_ASSERT_EQUALS(12, tuuvm_tuple_getSizeInBytes(symbol));

        tuuvm_tuple_t secondSymbol = tuuvm_symbol_internWithCString(tuuvm_test_context, "Hello World\n");
        TEST_ASSERT(tuuvm_tuple_isBytes(secondSymbol));
        TEST_ASSERT_EQUALS(12, tuuvm_tuple_getSizeInBytes(secondSymbol));

        TEST_ASSERT_EQUALS(symbol, secondSymbol);
    }
}