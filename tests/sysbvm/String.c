#include "TestMacros.h"
#include "sysbvm/string.h"

TEST_SUITE(String)
{
    TEST_CASE_WITH_FIXTURE(CreateWithString, TuuvmCore)
    {
        sysbvm_tuple_t string = sysbvm_string_createWithString(sysbvm_test_context, 2, "ABC");
        TEST_ASSERT(sysbvm_tuple_isBytes(string));
        TEST_ASSERT_EQUALS(2, sysbvm_tuple_getSizeInBytes(string));
    }

    TEST_CASE_WITH_FIXTURE(CreateWithCString, TuuvmCore)
    {
        sysbvm_tuple_t string = sysbvm_string_createWithCString(sysbvm_test_context, "Hello World\n");
        TEST_ASSERT(sysbvm_tuple_isBytes(string));
        TEST_ASSERT_EQUALS(12, sysbvm_tuple_getSizeInBytes(string));
    }
}


TEST_SUITE(StringSymbol)
{
    TEST_CASE_WITH_FIXTURE(InternWithString, TuuvmCore)
    {
        sysbvm_tuple_t symbol = sysbvm_symbol_internWithString(sysbvm_test_context, 2, "ABC");
        TEST_ASSERT(sysbvm_tuple_isBytes(symbol));
        TEST_ASSERT_EQUALS(2, sysbvm_tuple_getSizeInBytes(symbol));

        sysbvm_tuple_t secondSymbol = sysbvm_symbol_internWithString(sysbvm_test_context, 2, "ABC");
        TEST_ASSERT(sysbvm_tuple_isBytes(secondSymbol));
        TEST_ASSERT_EQUALS(2, sysbvm_tuple_getSizeInBytes(secondSymbol));

        TEST_ASSERT_EQUALS(symbol, secondSymbol);
    }

    TEST_CASE_WITH_FIXTURE(InternWithCString, TuuvmCore)
    {
        sysbvm_tuple_t symbol = sysbvm_symbol_internWithCString(sysbvm_test_context, "Hello World\n");
        TEST_ASSERT(sysbvm_tuple_isBytes(symbol));
        TEST_ASSERT_EQUALS(12, sysbvm_tuple_getSizeInBytes(symbol));

        sysbvm_tuple_t secondSymbol = sysbvm_symbol_internWithCString(sysbvm_test_context, "Hello World\n");
        TEST_ASSERT(sysbvm_tuple_isBytes(secondSymbol));
        TEST_ASSERT_EQUALS(12, sysbvm_tuple_getSizeInBytes(secondSymbol));

        TEST_ASSERT_EQUALS(symbol, secondSymbol);
    }
}