#include "TestMacros.h"
#include "sysbvm/tuple.h"

TEST_SUITE(Immediate)
{
    TEST_CASE_WITH_FIXTURE(Char8, TuuvmCore)
    {
        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_char8_encode(0)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_CHAR8, sysbvm_tuple_char8_encode(0) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_char8_encode(1)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_CHAR8, sysbvm_tuple_char8_encode(1) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_char8_encode(2)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_CHAR8, sysbvm_tuple_char8_encode(2) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_char8_encode(255)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_CHAR8, sysbvm_tuple_char8_encode(255) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, sysbvm_tuple_char8_decode(sysbvm_tuple_char8_encode(0)));
        TEST_ASSERT_EQUALS(1, sysbvm_tuple_char8_decode(sysbvm_tuple_char8_encode(1)));
        TEST_ASSERT_EQUALS(2, sysbvm_tuple_char8_decode(sysbvm_tuple_char8_encode(2)));
        TEST_ASSERT_EQUALS(255, sysbvm_tuple_char8_decode(sysbvm_tuple_char8_encode(255)));
    }

    TEST_CASE_WITH_FIXTURE(UInt8, TuuvmCore)
    {
        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_uint8_encode(0)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_UINT8, sysbvm_tuple_uint8_encode(0) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_uint8_encode(1)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_UINT8, sysbvm_tuple_uint8_encode(1) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_uint8_encode(2)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_UINT8, sysbvm_tuple_uint8_encode(2) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_uint8_encode(255)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_UINT8, sysbvm_tuple_uint8_encode(255) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, sysbvm_tuple_uint8_decode(sysbvm_tuple_uint8_encode(0)));
        TEST_ASSERT_EQUALS(1, sysbvm_tuple_uint8_decode(sysbvm_tuple_uint8_encode(1)));
        TEST_ASSERT_EQUALS(2, sysbvm_tuple_uint8_decode(sysbvm_tuple_uint8_encode(2)));
        TEST_ASSERT_EQUALS(255, sysbvm_tuple_uint8_decode(sysbvm_tuple_uint8_encode(255)));
    }

    TEST_CASE_WITH_FIXTURE(Int8, TuuvmCore)
    {
        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int8_encode(0)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT8, sysbvm_tuple_int8_encode(0) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int8_encode(1)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT8, sysbvm_tuple_int8_encode(1) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int8_encode(2)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT8, sysbvm_tuple_int8_encode(2) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int8_encode(-1)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT8, sysbvm_tuple_int8_encode(-1) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int8_encode(-2)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT8, sysbvm_tuple_int8_encode(-2) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, sysbvm_tuple_int8_decode(sysbvm_tuple_int8_encode(0)));
        TEST_ASSERT_EQUALS(1, sysbvm_tuple_int8_decode(sysbvm_tuple_int8_encode(1)));
        TEST_ASSERT_EQUALS(2, sysbvm_tuple_int8_decode(sysbvm_tuple_int8_encode(2)));
        TEST_ASSERT_EQUALS(-1, sysbvm_tuple_int8_decode(sysbvm_tuple_int8_encode(-1)));
        TEST_ASSERT_EQUALS(-2, sysbvm_tuple_int8_decode(sysbvm_tuple_int8_encode(-2)));
    }

    TEST_CASE_WITH_FIXTURE(Char16, TuuvmCore)
    {
        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_char16_encode(0)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_CHAR16, sysbvm_tuple_char16_encode(0) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_char16_encode(1)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_CHAR16, sysbvm_tuple_char16_encode(1) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_char16_encode(2)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_CHAR16, sysbvm_tuple_char16_encode(2) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_char16_encode(255)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_CHAR16, sysbvm_tuple_char16_encode(255) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_char16_encode(65535)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_CHAR16, sysbvm_tuple_char16_encode(65535) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, sysbvm_tuple_char16_decode(sysbvm_tuple_char16_encode(0)));
        TEST_ASSERT_EQUALS(1, sysbvm_tuple_char16_decode(sysbvm_tuple_char16_encode(1)));
        TEST_ASSERT_EQUALS(2, sysbvm_tuple_char16_decode(sysbvm_tuple_char16_encode(2)));
        TEST_ASSERT_EQUALS(255, sysbvm_tuple_char16_decode(sysbvm_tuple_char16_encode(255)));
        TEST_ASSERT_EQUALS(65535, sysbvm_tuple_char16_decode(sysbvm_tuple_char16_encode(65535)));
    }

    TEST_CASE_WITH_FIXTURE(UInt16, TuuvmCore)
    {
        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_uint16_encode(0)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_UINT16, sysbvm_tuple_uint16_encode(0) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_uint16_encode(1)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_UINT16, sysbvm_tuple_uint16_encode(1) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_uint16_encode(2)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_UINT16, sysbvm_tuple_uint16_encode(2) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_uint16_encode(255)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_UINT16, sysbvm_tuple_uint16_encode(255) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_uint16_encode(65535)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_UINT16, sysbvm_tuple_uint16_encode(65535) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, sysbvm_tuple_uint16_decode(sysbvm_tuple_uint16_encode(0)));
        TEST_ASSERT_EQUALS(1, sysbvm_tuple_uint16_decode(sysbvm_tuple_uint16_encode(1)));
        TEST_ASSERT_EQUALS(2, sysbvm_tuple_uint16_decode(sysbvm_tuple_uint16_encode(2)));
        TEST_ASSERT_EQUALS(255, sysbvm_tuple_uint16_decode(sysbvm_tuple_uint16_encode(255)));
        TEST_ASSERT_EQUALS(65535, sysbvm_tuple_uint16_decode(sysbvm_tuple_uint16_encode(65535)));
    }

    TEST_CASE_WITH_FIXTURE(Int16, TuuvmCore)
    {
        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int16_encode(0)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT16, sysbvm_tuple_int16_encode(0) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int16_encode(1)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT16, sysbvm_tuple_int16_encode(1) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int16_encode(2)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT16, sysbvm_tuple_int16_encode(2) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int16_encode(-1)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT16, sysbvm_tuple_int16_encode(-1) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int16_encode(-2)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT16, sysbvm_tuple_int16_encode(-2) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, sysbvm_tuple_int16_decode(sysbvm_tuple_int16_encode(0)));
        TEST_ASSERT_EQUALS(1, sysbvm_tuple_int16_decode(sysbvm_tuple_int16_encode(1)));
        TEST_ASSERT_EQUALS(2, sysbvm_tuple_int16_decode(sysbvm_tuple_int16_encode(2)));
        TEST_ASSERT_EQUALS(255, sysbvm_tuple_int16_decode(sysbvm_tuple_int16_encode(255)));
        TEST_ASSERT_EQUALS(-1, sysbvm_tuple_int16_decode(sysbvm_tuple_int16_encode(-1)));
        TEST_ASSERT_EQUALS(-2, sysbvm_tuple_int16_decode(sysbvm_tuple_int16_encode(-2)));
    }

    TEST_CASE_WITH_FIXTURE(Char32, TuuvmCore)
    {
        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_char32_encode(sysbvm_test_context, 0)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_CHAR32, sysbvm_tuple_char32_encode(sysbvm_test_context, 0) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_char32_encode(sysbvm_test_context, 1)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_CHAR32, sysbvm_tuple_char32_encode(sysbvm_test_context, 1) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_char32_encode(sysbvm_test_context, 2)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_CHAR32, sysbvm_tuple_char32_encode(sysbvm_test_context, 2) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_char32_encode(sysbvm_test_context, 255)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_CHAR32, sysbvm_tuple_char32_encode(sysbvm_test_context, 255) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_char32_encode(sysbvm_test_context, 65535)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_CHAR32, sysbvm_tuple_char32_encode(sysbvm_test_context, 65535) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_char32_encode(sysbvm_test_context, 4294967295)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_CHAR32, sysbvm_tuple_char32_encode(sysbvm_test_context, 4294967295) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, sysbvm_tuple_char32_decode(sysbvm_tuple_char32_encode(sysbvm_test_context, 0)));
        TEST_ASSERT_EQUALS(1, sysbvm_tuple_char32_decode(sysbvm_tuple_char32_encode(sysbvm_test_context, 1)));
        TEST_ASSERT_EQUALS(2, sysbvm_tuple_char32_decode(sysbvm_tuple_char32_encode(sysbvm_test_context, 2)));
        TEST_ASSERT_EQUALS(255, sysbvm_tuple_char32_decode(sysbvm_tuple_char32_encode(sysbvm_test_context, 255)));
        TEST_ASSERT_EQUALS(65535, sysbvm_tuple_char32_decode(sysbvm_tuple_char32_encode(sysbvm_test_context, 65535)));
        TEST_ASSERT_EQUALS(4294967295, sysbvm_tuple_char32_decode(sysbvm_tuple_char32_encode(sysbvm_test_context, 4294967295)));
    }

    TEST_CASE_WITH_FIXTURE(UInt32, TuuvmCore)
    {
        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_uint32_encode(sysbvm_test_context, 0)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_UINT32, sysbvm_tuple_uint32_encode(sysbvm_test_context, 0) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_uint32_encode(sysbvm_test_context, 1)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_UINT32, sysbvm_tuple_uint32_encode(sysbvm_test_context, 1) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_uint32_encode(sysbvm_test_context, 2)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_UINT32, sysbvm_tuple_uint32_encode(sysbvm_test_context, 2) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_uint32_encode(sysbvm_test_context, 255)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_UINT32, sysbvm_tuple_uint32_encode(sysbvm_test_context, 255) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_uint32_encode(sysbvm_test_context, 65535)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_UINT32, sysbvm_tuple_uint32_encode(sysbvm_test_context, 65535) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_uint32_encode(sysbvm_test_context, 4294967295)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_UINT32, sysbvm_tuple_uint32_encode(sysbvm_test_context, 4294967295) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, sysbvm_tuple_uint32_decode(sysbvm_tuple_uint32_encode(sysbvm_test_context, 0)));
        TEST_ASSERT_EQUALS(1, sysbvm_tuple_uint32_decode(sysbvm_tuple_uint32_encode(sysbvm_test_context, 1)));
        TEST_ASSERT_EQUALS(2, sysbvm_tuple_uint32_decode(sysbvm_tuple_uint32_encode(sysbvm_test_context, 2)));
        TEST_ASSERT_EQUALS(255, sysbvm_tuple_uint32_decode(sysbvm_tuple_uint32_encode(sysbvm_test_context, 255)));
        TEST_ASSERT_EQUALS(65535, sysbvm_tuple_uint32_decode(sysbvm_tuple_uint32_encode(sysbvm_test_context, 65535)));
        TEST_ASSERT_EQUALS(4294967295, sysbvm_tuple_uint32_decode(sysbvm_tuple_uint32_encode(sysbvm_test_context, 4294967295)));
    }

    TEST_CASE_WITH_FIXTURE(Int32, TuuvmCore)
    {
        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int32_encode(sysbvm_test_context, 0)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT32, sysbvm_tuple_int32_encode(sysbvm_test_context, 0) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int32_encode(sysbvm_test_context, 1)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT32, sysbvm_tuple_int32_encode(sysbvm_test_context, 1) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int32_encode(sysbvm_test_context, 2)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT32, sysbvm_tuple_int32_encode(sysbvm_test_context, 2) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int32_encode(sysbvm_test_context, 255)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT32, sysbvm_tuple_int32_encode(sysbvm_test_context, 255) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int32_encode(sysbvm_test_context, 65535)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT32, sysbvm_tuple_int32_encode(sysbvm_test_context, 65535) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int32_encode(sysbvm_test_context, -1)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT32, sysbvm_tuple_int32_encode(sysbvm_test_context, -1) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int32_encode(sysbvm_test_context, -65535)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT32, sysbvm_tuple_int32_encode(sysbvm_test_context, -65535) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, sysbvm_tuple_int32_decode(sysbvm_tuple_int32_encode(sysbvm_test_context, 0)));
        TEST_ASSERT_EQUALS(1, sysbvm_tuple_int32_decode(sysbvm_tuple_int32_encode(sysbvm_test_context, 1)));
        TEST_ASSERT_EQUALS(2, sysbvm_tuple_int32_decode(sysbvm_tuple_int32_encode(sysbvm_test_context, 2)));
        TEST_ASSERT_EQUALS(255, sysbvm_tuple_int32_decode(sysbvm_tuple_int32_encode(sysbvm_test_context, 255)));
        TEST_ASSERT_EQUALS(65535, sysbvm_tuple_int32_decode(sysbvm_tuple_int32_encode(sysbvm_test_context, 65535)));
        TEST_ASSERT_EQUALS(-1, sysbvm_tuple_int32_decode(sysbvm_tuple_int32_encode(sysbvm_test_context, -1)));
        TEST_ASSERT_EQUALS(-2, sysbvm_tuple_int32_decode(sysbvm_tuple_int32_encode(sysbvm_test_context, -2)));
        TEST_ASSERT_EQUALS(-255, sysbvm_tuple_int32_decode(sysbvm_tuple_int32_encode(sysbvm_test_context, -255)));
        TEST_ASSERT_EQUALS(-65535, sysbvm_tuple_int32_decode(sysbvm_tuple_int32_encode(sysbvm_test_context, -65535)));
    }

    TEST_CASE_WITH_FIXTURE(UInt64, TuuvmCore)
    {
        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_uint64_encode(sysbvm_test_context, 0)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_UINT64, sysbvm_tuple_uint64_encode(sysbvm_test_context, 0) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_uint64_encode(sysbvm_test_context, 1)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_UINT64, sysbvm_tuple_uint64_encode(sysbvm_test_context, 1) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_uint64_encode(sysbvm_test_context, 2)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_UINT64, sysbvm_tuple_uint64_encode(sysbvm_test_context, 2) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_uint64_encode(sysbvm_test_context, 255)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_UINT64, sysbvm_tuple_uint64_encode(sysbvm_test_context, 255) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_uint64_encode(sysbvm_test_context, 65535)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_UINT64, sysbvm_tuple_uint64_encode(sysbvm_test_context, 65535) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_uint64_encode(sysbvm_test_context, 4294967295)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_UINT64, sysbvm_tuple_uint64_encode(sysbvm_test_context, 4294967295) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, sysbvm_tuple_uint64_decode(sysbvm_tuple_uint64_encode(sysbvm_test_context, 0)));
        TEST_ASSERT_EQUALS(1, sysbvm_tuple_uint64_decode(sysbvm_tuple_uint64_encode(sysbvm_test_context, 1)));
        TEST_ASSERT_EQUALS(2, sysbvm_tuple_uint64_decode(sysbvm_tuple_uint64_encode(sysbvm_test_context, 2)));
        TEST_ASSERT_EQUALS(255, sysbvm_tuple_uint64_decode(sysbvm_tuple_uint64_encode(sysbvm_test_context, 255)));
        TEST_ASSERT_EQUALS(65535, sysbvm_tuple_uint64_decode(sysbvm_tuple_uint64_encode(sysbvm_test_context, 65535)));
        TEST_ASSERT_EQUALS(4294967295, sysbvm_tuple_uint64_decode(sysbvm_tuple_uint64_encode(sysbvm_test_context, 4294967295)));
    }

    TEST_CASE_WITH_FIXTURE(Int64, TuuvmCore)
    {
        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int64_encode(sysbvm_test_context, 0)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT64, sysbvm_tuple_int64_encode(sysbvm_test_context, 0) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int64_encode(sysbvm_test_context, 1)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT64, sysbvm_tuple_int64_encode(sysbvm_test_context, 1) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int64_encode(sysbvm_test_context, 2)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT64, sysbvm_tuple_int64_encode(sysbvm_test_context, 2) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int64_encode(sysbvm_test_context, 255)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT64, sysbvm_tuple_int64_encode(sysbvm_test_context, 255) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int64_encode(sysbvm_test_context, 65535)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT64, sysbvm_tuple_int64_encode(sysbvm_test_context, 65535) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int64_encode(sysbvm_test_context, -1)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT64, sysbvm_tuple_int64_encode(sysbvm_test_context, -1) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int64_encode(sysbvm_test_context, -65535)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT64, sysbvm_tuple_int64_encode(sysbvm_test_context, -65535) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT(sysbvm_tuple_isImmediate(sysbvm_tuple_int64_encode(sysbvm_test_context, -4294967295ll)));
        TEST_ASSERT_EQUALS(SYSBVM_TUPLE_TAG_INT64, sysbvm_tuple_int64_encode(sysbvm_test_context, -4294967295ll) & SYSBVM_TUPLE_TAG_BIT_MASK);

        TEST_ASSERT_EQUALS(0, sysbvm_tuple_int64_decode(sysbvm_tuple_int64_encode(sysbvm_test_context, 0)));
        TEST_ASSERT_EQUALS(1, sysbvm_tuple_int64_decode(sysbvm_tuple_int64_encode(sysbvm_test_context, 1)));
        TEST_ASSERT_EQUALS(2, sysbvm_tuple_int64_decode(sysbvm_tuple_int64_encode(sysbvm_test_context, 2)));
        TEST_ASSERT_EQUALS(255, sysbvm_tuple_int64_decode(sysbvm_tuple_int64_encode(sysbvm_test_context, 255)));
        TEST_ASSERT_EQUALS(65535, sysbvm_tuple_int64_decode(sysbvm_tuple_int64_encode(sysbvm_test_context, 65535)));
        TEST_ASSERT_EQUALS(4294967295, sysbvm_tuple_int64_decode(sysbvm_tuple_int64_encode(sysbvm_test_context, 4294967295)));
        TEST_ASSERT_EQUALS(-1, sysbvm_tuple_int64_decode(sysbvm_tuple_int64_encode(sysbvm_test_context, -1)));
        TEST_ASSERT_EQUALS(-2, sysbvm_tuple_int64_decode(sysbvm_tuple_int64_encode(sysbvm_test_context, -2)));
        TEST_ASSERT_EQUALS(-255, sysbvm_tuple_int64_decode(sysbvm_tuple_int64_encode(sysbvm_test_context, -255)));
        TEST_ASSERT_EQUALS(-65535, sysbvm_tuple_int64_decode(sysbvm_tuple_int64_encode(sysbvm_test_context, -65535)));
        TEST_ASSERT_EQUALS(-4294967295ll, sysbvm_tuple_int64_decode(sysbvm_tuple_int64_encode(sysbvm_test_context, -4294967295ll)));
    }
}
