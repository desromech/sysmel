#include "TestMacros.h"
#include "tuuvm/tuple.h"

TEST_SUITE(Interpreter)
{
    TEST_CASE_WITH_FIXTURE(EmptyString, TuuvmCore)
    {
        /*
        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_char8_encode(0)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_CHAR8, tuuvm_tuple_char8_encode(0) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_char8_encode(1)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_CHAR8, tuuvm_tuple_char8_encode(1) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_char8_encode(2)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_CHAR8, tuuvm_tuple_char8_encode(2) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_char8_encode(255)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_CHAR8, tuuvm_tuple_char8_encode(255) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, tuuvm_tuple_char8_decode(tuuvm_tuple_char8_encode(0)));
        TEST_ASSERT_EQUALS(1, tuuvm_tuple_char8_decode(tuuvm_tuple_char8_encode(1)));
        TEST_ASSERT_EQUALS(2, tuuvm_tuple_char8_decode(tuuvm_tuple_char8_encode(2)));
        TEST_ASSERT_EQUALS(255, tuuvm_tuple_char8_decode(tuuvm_tuple_char8_encode(255)));
        */
    }
}
