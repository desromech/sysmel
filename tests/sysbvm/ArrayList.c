#include "TestMacros.h"
#include "sysbvm/arrayList.h"
#include "sysbvm/arraySlice.h"

TEST_SUITE(ArrayList)
{
    TEST_CASE_WITH_FIXTURE(Empty, TuuvmCore)
    {
        TEST_ASSERT(sysbvm_tuple_isNonNullPointer(sysbvm_arrayList_create(sysbvm_test_context)));
        TEST_ASSERT_EQUALS(0, sysbvm_arrayList_getSize(sysbvm_arrayList_create(sysbvm_test_context)));
        TEST_ASSERT_EQUALS(0, sysbvm_arrayList_getCapacity(sysbvm_arrayList_create(sysbvm_test_context)));
    }

    TEST_CASE_WITH_FIXTURE(Add1, TuuvmCore)
    {
        sysbvm_tuple_t arrayList = sysbvm_arrayList_create(sysbvm_test_context);
        TEST_ASSERT(sysbvm_tuple_isNonNullPointer(arrayList));
        TEST_ASSERT_EQUALS(0, sysbvm_arrayList_getSize(sysbvm_arrayList_create(sysbvm_test_context)));
        TEST_ASSERT_EQUALS(0, sysbvm_arrayList_getCapacity(sysbvm_arrayList_create(sysbvm_test_context)));

        sysbvm_arrayList_add(sysbvm_test_context, arrayList, sysbvm_tuple_uint8_encode(1));
        TEST_ASSERT_EQUALS(1, sysbvm_arrayList_getSize(arrayList));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(1), sysbvm_arrayList_at(arrayList, 0));
    }

    TEST_CASE_WITH_FIXTURE(Add2, TuuvmCore)
    {
        sysbvm_tuple_t arrayList = sysbvm_arrayList_create(sysbvm_test_context);
        TEST_ASSERT(sysbvm_tuple_isNonNullPointer(arrayList));
        TEST_ASSERT_EQUALS(0, sysbvm_arrayList_getSize(sysbvm_arrayList_create(sysbvm_test_context)));
        TEST_ASSERT_EQUALS(0, sysbvm_arrayList_getCapacity(sysbvm_arrayList_create(sysbvm_test_context)));

        sysbvm_arrayList_add(sysbvm_test_context, arrayList, sysbvm_tuple_uint8_encode(1));
        sysbvm_arrayList_add(sysbvm_test_context, arrayList, sysbvm_tuple_uint8_encode(2));
        TEST_ASSERT_EQUALS(2, sysbvm_arrayList_getSize(arrayList));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(1), sysbvm_arrayList_at(arrayList, 0));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(2), sysbvm_arrayList_at(arrayList, 1));
    }

    TEST_CASE_WITH_FIXTURE(Add3, TuuvmCore)
    {
        sysbvm_tuple_t arrayList = sysbvm_arrayList_create(sysbvm_test_context);
        TEST_ASSERT(sysbvm_tuple_isNonNullPointer(arrayList));
        TEST_ASSERT_EQUALS(0, sysbvm_arrayList_getSize(sysbvm_arrayList_create(sysbvm_test_context)));
        TEST_ASSERT_EQUALS(0, sysbvm_arrayList_getCapacity(sysbvm_arrayList_create(sysbvm_test_context)));

        sysbvm_arrayList_add(sysbvm_test_context, arrayList, sysbvm_tuple_uint8_encode(1));
        sysbvm_arrayList_add(sysbvm_test_context, arrayList, sysbvm_tuple_uint8_encode(2));
        sysbvm_arrayList_add(sysbvm_test_context, arrayList, sysbvm_tuple_uint8_encode(5));
        TEST_ASSERT_EQUALS(3, sysbvm_arrayList_getSize(arrayList));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(1), sysbvm_arrayList_at(arrayList, 0));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(2), sysbvm_arrayList_at(arrayList, 1));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(5), sysbvm_arrayList_at(arrayList, 2));
    }

    TEST_CASE_WITH_FIXTURE(AsArraySlice, TuuvmCore)
    {
        sysbvm_tuple_t arrayList = sysbvm_arrayList_create(sysbvm_test_context);
        TEST_ASSERT(sysbvm_tuple_isNonNullPointer(arrayList));
        TEST_ASSERT_EQUALS(0, sysbvm_arrayList_getSize(sysbvm_arrayList_create(sysbvm_test_context)));
        TEST_ASSERT_EQUALS(0, sysbvm_arrayList_getCapacity(sysbvm_arrayList_create(sysbvm_test_context)));

        sysbvm_arrayList_add(sysbvm_test_context, arrayList, sysbvm_tuple_uint8_encode(1));
        sysbvm_arrayList_add(sysbvm_test_context, arrayList, sysbvm_tuple_uint8_encode(2));
        sysbvm_arrayList_add(sysbvm_test_context, arrayList, sysbvm_tuple_uint8_encode(5));
        TEST_ASSERT_EQUALS(3, sysbvm_arrayList_getSize(arrayList));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(1), sysbvm_arrayList_at(arrayList, 0));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(2), sysbvm_arrayList_at(arrayList, 1));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(5), sysbvm_arrayList_at(arrayList, 2));

        sysbvm_tuple_t arraySlice = sysbvm_arrayList_asArraySlice(sysbvm_test_context, arrayList);
        TEST_ASSERT(sysbvm_tuple_isNonNullPointer(arraySlice));
        TEST_ASSERT_EQUALS(3, sysbvm_arraySlice_getSize(arraySlice));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(1), sysbvm_arraySlice_at(arraySlice, 0));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(2), sysbvm_arraySlice_at(arraySlice, 1));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(5), sysbvm_arraySlice_at(arraySlice, 2));
    }

}
