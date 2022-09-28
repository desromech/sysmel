#include "TestMacros.h"
#include "tuuvm/tuple.h"

TEST_SUITE(Immediate)
{
    TEST_CASE_WITH_FIXTURE(Char8, TuuvmCore)
    {
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
    }

    TEST_CASE_WITH_FIXTURE(UInt8, TuuvmCore)
    {
        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_uint8_encode(0)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_UINT8, tuuvm_tuple_uint8_encode(0) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_uint8_encode(1)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_UINT8, tuuvm_tuple_uint8_encode(1) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_uint8_encode(2)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_UINT8, tuuvm_tuple_uint8_encode(2) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_uint8_encode(255)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_UINT8, tuuvm_tuple_uint8_encode(255) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, tuuvm_tuple_uint8_decode(tuuvm_tuple_uint8_encode(0)));
        TEST_ASSERT_EQUALS(1, tuuvm_tuple_uint8_decode(tuuvm_tuple_uint8_encode(1)));
        TEST_ASSERT_EQUALS(2, tuuvm_tuple_uint8_decode(tuuvm_tuple_uint8_encode(2)));
        TEST_ASSERT_EQUALS(255, tuuvm_tuple_uint8_decode(tuuvm_tuple_uint8_encode(255)));
    }

    TEST_CASE_WITH_FIXTURE(Int8, TuuvmCore)
    {
        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int8_encode(0)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT8, tuuvm_tuple_int8_encode(0) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int8_encode(1)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT8, tuuvm_tuple_int8_encode(1) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int8_encode(2)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT8, tuuvm_tuple_int8_encode(2) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int8_encode(-1)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT8, tuuvm_tuple_int8_encode(-1) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int8_encode(-2)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT8, tuuvm_tuple_int8_encode(-2) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, tuuvm_tuple_int8_decode(tuuvm_tuple_int8_encode(0)));
        TEST_ASSERT_EQUALS(1, tuuvm_tuple_int8_decode(tuuvm_tuple_int8_encode(1)));
        TEST_ASSERT_EQUALS(2, tuuvm_tuple_int8_decode(tuuvm_tuple_int8_encode(2)));
        TEST_ASSERT_EQUALS(-1, tuuvm_tuple_int8_decode(tuuvm_tuple_int8_encode(-1)));
        TEST_ASSERT_EQUALS(-2, tuuvm_tuple_int8_decode(tuuvm_tuple_int8_encode(-2)));
    }

    TEST_CASE_WITH_FIXTURE(Char16, TuuvmCore)
    {
        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_char16_encode(0)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_CHAR16, tuuvm_tuple_char16_encode(0) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_char16_encode(1)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_CHAR16, tuuvm_tuple_char16_encode(1) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_char16_encode(2)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_CHAR16, tuuvm_tuple_char16_encode(2) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_char16_encode(255)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_CHAR16, tuuvm_tuple_char16_encode(255) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_char16_encode(65535)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_CHAR16, tuuvm_tuple_char16_encode(65535) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, tuuvm_tuple_char16_decode(tuuvm_tuple_char16_encode(0)));
        TEST_ASSERT_EQUALS(1, tuuvm_tuple_char16_decode(tuuvm_tuple_char16_encode(1)));
        TEST_ASSERT_EQUALS(2, tuuvm_tuple_char16_decode(tuuvm_tuple_char16_encode(2)));
        TEST_ASSERT_EQUALS(255, tuuvm_tuple_char16_decode(tuuvm_tuple_char16_encode(255)));
        TEST_ASSERT_EQUALS(65535, tuuvm_tuple_char16_decode(tuuvm_tuple_char16_encode(65535)));
    }

    TEST_CASE_WITH_FIXTURE(UInt16, TuuvmCore)
    {
        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_uint16_encode(0)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_UINT16, tuuvm_tuple_uint16_encode(0) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_uint16_encode(1)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_UINT16, tuuvm_tuple_uint16_encode(1) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_uint16_encode(2)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_UINT16, tuuvm_tuple_uint16_encode(2) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_uint16_encode(255)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_UINT16, tuuvm_tuple_uint16_encode(255) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_uint16_encode(65535)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_UINT16, tuuvm_tuple_uint16_encode(65535) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, tuuvm_tuple_uint16_decode(tuuvm_tuple_uint16_encode(0)));
        TEST_ASSERT_EQUALS(1, tuuvm_tuple_uint16_decode(tuuvm_tuple_uint16_encode(1)));
        TEST_ASSERT_EQUALS(2, tuuvm_tuple_uint16_decode(tuuvm_tuple_uint16_encode(2)));
        TEST_ASSERT_EQUALS(255, tuuvm_tuple_uint16_decode(tuuvm_tuple_uint16_encode(255)));
        TEST_ASSERT_EQUALS(65535, tuuvm_tuple_uint16_decode(tuuvm_tuple_uint16_encode(65535)));
    }

    TEST_CASE_WITH_FIXTURE(Int16, TuuvmCore)
    {
        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int16_encode(0)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT16, tuuvm_tuple_int16_encode(0) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int16_encode(1)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT16, tuuvm_tuple_int16_encode(1) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int16_encode(2)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT16, tuuvm_tuple_int16_encode(2) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int16_encode(-1)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT16, tuuvm_tuple_int16_encode(-1) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int16_encode(-2)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT16, tuuvm_tuple_int16_encode(-2) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, tuuvm_tuple_int16_decode(tuuvm_tuple_int16_encode(0)));
        TEST_ASSERT_EQUALS(1, tuuvm_tuple_int16_decode(tuuvm_tuple_int16_encode(1)));
        TEST_ASSERT_EQUALS(2, tuuvm_tuple_int16_decode(tuuvm_tuple_int16_encode(2)));
        TEST_ASSERT_EQUALS(255, tuuvm_tuple_int16_decode(tuuvm_tuple_int16_encode(255)));
        TEST_ASSERT_EQUALS(-1, tuuvm_tuple_int16_decode(tuuvm_tuple_int16_encode(-1)));
        TEST_ASSERT_EQUALS(-2, tuuvm_tuple_int16_decode(tuuvm_tuple_int16_encode(-2)));
    }

    TEST_CASE_WITH_FIXTURE(Char32, TuuvmCore)
    {
        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_char32_encode(tuuvm_test_context, 0)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_CHAR32, tuuvm_tuple_char32_encode(tuuvm_test_context, 0) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_char32_encode(tuuvm_test_context, 1)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_CHAR32, tuuvm_tuple_char32_encode(tuuvm_test_context, 1) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_char32_encode(tuuvm_test_context, 2)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_CHAR32, tuuvm_tuple_char32_encode(tuuvm_test_context, 2) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_char32_encode(tuuvm_test_context, 255)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_CHAR32, tuuvm_tuple_char32_encode(tuuvm_test_context, 255) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_char32_encode(tuuvm_test_context, 65535)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_CHAR32, tuuvm_tuple_char32_encode(tuuvm_test_context, 65535) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_char32_encode(tuuvm_test_context, 4294967295)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_CHAR32, tuuvm_tuple_char32_encode(tuuvm_test_context, 4294967295) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, tuuvm_tuple_char32_decode(tuuvm_tuple_char32_encode(tuuvm_test_context, 0)));
        TEST_ASSERT_EQUALS(1, tuuvm_tuple_char32_decode(tuuvm_tuple_char32_encode(tuuvm_test_context, 1)));
        TEST_ASSERT_EQUALS(2, tuuvm_tuple_char32_decode(tuuvm_tuple_char32_encode(tuuvm_test_context, 2)));
        TEST_ASSERT_EQUALS(255, tuuvm_tuple_char32_decode(tuuvm_tuple_char32_encode(tuuvm_test_context, 255)));
        TEST_ASSERT_EQUALS(65535, tuuvm_tuple_char32_decode(tuuvm_tuple_char32_encode(tuuvm_test_context, 65535)));
        TEST_ASSERT_EQUALS(4294967295, tuuvm_tuple_char32_decode(tuuvm_tuple_char32_encode(tuuvm_test_context, 4294967295)));
    }

    TEST_CASE_WITH_FIXTURE(UInt32, TuuvmCore)
    {
        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_uint32_encode(tuuvm_test_context, 0)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_UINT32, tuuvm_tuple_uint32_encode(tuuvm_test_context, 0) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_uint32_encode(tuuvm_test_context, 1)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_UINT32, tuuvm_tuple_uint32_encode(tuuvm_test_context, 1) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_uint32_encode(tuuvm_test_context, 2)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_UINT32, tuuvm_tuple_uint32_encode(tuuvm_test_context, 2) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_uint32_encode(tuuvm_test_context, 255)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_UINT32, tuuvm_tuple_uint32_encode(tuuvm_test_context, 255) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_uint32_encode(tuuvm_test_context, 65535)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_UINT32, tuuvm_tuple_uint32_encode(tuuvm_test_context, 65535) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_uint32_encode(tuuvm_test_context, 4294967295)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_UINT32, tuuvm_tuple_uint32_encode(tuuvm_test_context, 4294967295) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, tuuvm_tuple_uint32_decode(tuuvm_tuple_uint32_encode(tuuvm_test_context, 0)));
        TEST_ASSERT_EQUALS(1, tuuvm_tuple_uint32_decode(tuuvm_tuple_uint32_encode(tuuvm_test_context, 1)));
        TEST_ASSERT_EQUALS(2, tuuvm_tuple_uint32_decode(tuuvm_tuple_uint32_encode(tuuvm_test_context, 2)));
        TEST_ASSERT_EQUALS(255, tuuvm_tuple_uint32_decode(tuuvm_tuple_uint32_encode(tuuvm_test_context, 255)));
        TEST_ASSERT_EQUALS(65535, tuuvm_tuple_uint32_decode(tuuvm_tuple_uint32_encode(tuuvm_test_context, 65535)));
        TEST_ASSERT_EQUALS(4294967295, tuuvm_tuple_uint32_decode(tuuvm_tuple_uint32_encode(tuuvm_test_context, 4294967295)));
    }

    TEST_CASE_WITH_FIXTURE(Int32, TuuvmCore)
    {
        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int32_encode(tuuvm_test_context, 0)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT32, tuuvm_tuple_int32_encode(tuuvm_test_context, 0) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int32_encode(tuuvm_test_context, 1)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT32, tuuvm_tuple_int32_encode(tuuvm_test_context, 1) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int32_encode(tuuvm_test_context, 2)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT32, tuuvm_tuple_int32_encode(tuuvm_test_context, 2) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int32_encode(tuuvm_test_context, 255)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT32, tuuvm_tuple_int32_encode(tuuvm_test_context, 255) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int32_encode(tuuvm_test_context, 65535)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT32, tuuvm_tuple_int32_encode(tuuvm_test_context, 65535) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int32_encode(tuuvm_test_context, -1)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT32, tuuvm_tuple_int32_encode(tuuvm_test_context, -1) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int32_encode(tuuvm_test_context, -65535)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT32, tuuvm_tuple_int32_encode(tuuvm_test_context, -65535) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, tuuvm_tuple_int32_decode(tuuvm_tuple_int32_encode(tuuvm_test_context, 0)));
        TEST_ASSERT_EQUALS(1, tuuvm_tuple_int32_decode(tuuvm_tuple_int32_encode(tuuvm_test_context, 1)));
        TEST_ASSERT_EQUALS(2, tuuvm_tuple_int32_decode(tuuvm_tuple_int32_encode(tuuvm_test_context, 2)));
        TEST_ASSERT_EQUALS(255, tuuvm_tuple_int32_decode(tuuvm_tuple_int32_encode(tuuvm_test_context, 255)));
        TEST_ASSERT_EQUALS(65535, tuuvm_tuple_int32_decode(tuuvm_tuple_int32_encode(tuuvm_test_context, 65535)));
        TEST_ASSERT_EQUALS(-1, tuuvm_tuple_int32_decode(tuuvm_tuple_int32_encode(tuuvm_test_context, -1)));
        TEST_ASSERT_EQUALS(-2, tuuvm_tuple_int32_decode(tuuvm_tuple_int32_encode(tuuvm_test_context, -2)));
        TEST_ASSERT_EQUALS(-255, tuuvm_tuple_int32_decode(tuuvm_tuple_int32_encode(tuuvm_test_context, -255)));
        TEST_ASSERT_EQUALS(-65535, tuuvm_tuple_int32_decode(tuuvm_tuple_int32_encode(tuuvm_test_context, -65535)));
    }

    TEST_CASE_WITH_FIXTURE(UInt64, TuuvmCore)
    {
        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_uint64_encode(tuuvm_test_context, 0)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_UINT64, tuuvm_tuple_uint64_encode(tuuvm_test_context, 0) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_uint64_encode(tuuvm_test_context, 1)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_UINT64, tuuvm_tuple_uint64_encode(tuuvm_test_context, 1) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_uint64_encode(tuuvm_test_context, 2)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_UINT64, tuuvm_tuple_uint64_encode(tuuvm_test_context, 2) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_uint64_encode(tuuvm_test_context, 255)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_UINT64, tuuvm_tuple_uint64_encode(tuuvm_test_context, 255) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_uint64_encode(tuuvm_test_context, 65535)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_UINT64, tuuvm_tuple_uint64_encode(tuuvm_test_context, 65535) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_uint64_encode(tuuvm_test_context, 4294967295)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_UINT64, tuuvm_tuple_uint64_encode(tuuvm_test_context, 4294967295) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, tuuvm_tuple_uint64_decode(tuuvm_tuple_uint64_encode(tuuvm_test_context, 0)));
        TEST_ASSERT_EQUALS(1, tuuvm_tuple_uint64_decode(tuuvm_tuple_uint64_encode(tuuvm_test_context, 1)));
        TEST_ASSERT_EQUALS(2, tuuvm_tuple_uint64_decode(tuuvm_tuple_uint64_encode(tuuvm_test_context, 2)));
        TEST_ASSERT_EQUALS(255, tuuvm_tuple_uint64_decode(tuuvm_tuple_uint64_encode(tuuvm_test_context, 255)));
        TEST_ASSERT_EQUALS(65535, tuuvm_tuple_uint64_decode(tuuvm_tuple_uint64_encode(tuuvm_test_context, 65535)));
        TEST_ASSERT_EQUALS(4294967295, tuuvm_tuple_uint64_decode(tuuvm_tuple_uint64_encode(tuuvm_test_context, 4294967295)));
    }

    TEST_CASE_WITH_FIXTURE(Int64, TuuvmCore)
    {
        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int64_encode(tuuvm_test_context, 0)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT64, tuuvm_tuple_int64_encode(tuuvm_test_context, 0) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int64_encode(tuuvm_test_context, 1)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT64, tuuvm_tuple_int64_encode(tuuvm_test_context, 1) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int64_encode(tuuvm_test_context, 2)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT64, tuuvm_tuple_int64_encode(tuuvm_test_context, 2) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int64_encode(tuuvm_test_context, 255)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT64, tuuvm_tuple_int64_encode(tuuvm_test_context, 255) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int64_encode(tuuvm_test_context, 65535)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT64, tuuvm_tuple_int64_encode(tuuvm_test_context, 65535) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int64_encode(tuuvm_test_context, -1)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT64, tuuvm_tuple_int64_encode(tuuvm_test_context, -1) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int64_encode(tuuvm_test_context, -65535)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT64, tuuvm_tuple_int64_encode(tuuvm_test_context, -65535) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(tuuvm_tuple_isImmediate(tuuvm_tuple_int64_encode(tuuvm_test_context, -4294967295)));
        TEST_ASSERT_EQUALS(TUUVM_TUPLE_TAG_INT64, tuuvm_tuple_int64_encode(tuuvm_test_context, -4294967295) & TUUVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, tuuvm_tuple_int64_decode(tuuvm_tuple_int64_encode(tuuvm_test_context, 0)));
        TEST_ASSERT_EQUALS(1, tuuvm_tuple_int64_decode(tuuvm_tuple_int64_encode(tuuvm_test_context, 1)));
        TEST_ASSERT_EQUALS(2, tuuvm_tuple_int64_decode(tuuvm_tuple_int64_encode(tuuvm_test_context, 2)));
        TEST_ASSERT_EQUALS(255, tuuvm_tuple_int64_decode(tuuvm_tuple_int64_encode(tuuvm_test_context, 255)));
        TEST_ASSERT_EQUALS(65535, tuuvm_tuple_int64_decode(tuuvm_tuple_int64_encode(tuuvm_test_context, 65535)));
        TEST_ASSERT_EQUALS(4294967295, tuuvm_tuple_int64_decode(tuuvm_tuple_int64_encode(tuuvm_test_context, 4294967295)));
        TEST_ASSERT_EQUALS(-1, tuuvm_tuple_int64_decode(tuuvm_tuple_int64_encode(tuuvm_test_context, -1)));
        TEST_ASSERT_EQUALS(-2, tuuvm_tuple_int64_decode(tuuvm_tuple_int64_encode(tuuvm_test_context, -2)));
        TEST_ASSERT_EQUALS(-255, tuuvm_tuple_int64_decode(tuuvm_tuple_int64_encode(tuuvm_test_context, -255)));
        TEST_ASSERT_EQUALS(-65535, tuuvm_tuple_int64_decode(tuuvm_tuple_int64_encode(tuuvm_test_context, -65535)));
        TEST_ASSERT_EQUALS(-4294967295, tuuvm_tuple_int64_decode(tuuvm_tuple_int64_encode(tuuvm_test_context, -4294967295)));
    }
}
