#include "TestMacros.h"
#include "tuuvm/arrayList.h"
#include "tuuvm/arraySlice.h"

TEST_SUITE(ArrayList)
{
    TEST_CASE_WITH_FIXTURE(Empty, TuuvmCore)
    {
        TEST_ASSERT(tuuvm_tuple_isNonNullPointer(tuuvm_arrayList_create(tuuvm_test_context)));
        TEST_ASSERT_EQUALS(0, tuuvm_arrayList_getSize(tuuvm_arrayList_create(tuuvm_test_context)));
        TEST_ASSERT_EQUALS(0, tuuvm_arrayList_getCapacity(tuuvm_arrayList_create(tuuvm_test_context)));
    }

    TEST_CASE_WITH_FIXTURE(Add1, TuuvmCore)
    {
        tuuvm_tuple_t arrayList = tuuvm_arrayList_create(tuuvm_test_context);
        TEST_ASSERT(tuuvm_tuple_isNonNullPointer(arrayList));
        TEST_ASSERT_EQUALS(0, tuuvm_arrayList_getSize(tuuvm_arrayList_create(tuuvm_test_context)));
        TEST_ASSERT_EQUALS(0, tuuvm_arrayList_getCapacity(tuuvm_arrayList_create(tuuvm_test_context)));

        tuuvm_arrayList_add(tuuvm_test_context, arrayList, tuuvm_tuple_uint8_encode(1));
        TEST_ASSERT_EQUALS(1, tuuvm_arrayList_getSize(arrayList));
        TEST_ASSERT_EQUALS(tuuvm_tuple_uint8_encode(1), tuuvm_arrayList_at(arrayList, 0));
    }

    TEST_CASE_WITH_FIXTURE(Add2, TuuvmCore)
    {
        tuuvm_tuple_t arrayList = tuuvm_arrayList_create(tuuvm_test_context);
        TEST_ASSERT(tuuvm_tuple_isNonNullPointer(arrayList));
        TEST_ASSERT_EQUALS(0, tuuvm_arrayList_getSize(tuuvm_arrayList_create(tuuvm_test_context)));
        TEST_ASSERT_EQUALS(0, tuuvm_arrayList_getCapacity(tuuvm_arrayList_create(tuuvm_test_context)));

        tuuvm_arrayList_add(tuuvm_test_context, arrayList, tuuvm_tuple_uint8_encode(1));
        tuuvm_arrayList_add(tuuvm_test_context, arrayList, tuuvm_tuple_uint8_encode(2));
        TEST_ASSERT_EQUALS(2, tuuvm_arrayList_getSize(arrayList));
        TEST_ASSERT_EQUALS(tuuvm_tuple_uint8_encode(1), tuuvm_arrayList_at(arrayList, 0));
        TEST_ASSERT_EQUALS(tuuvm_tuple_uint8_encode(2), tuuvm_arrayList_at(arrayList, 1));
    }

    TEST_CASE_WITH_FIXTURE(Add3, TuuvmCore)
    {
        tuuvm_tuple_t arrayList = tuuvm_arrayList_create(tuuvm_test_context);
        TEST_ASSERT(tuuvm_tuple_isNonNullPointer(arrayList));
        TEST_ASSERT_EQUALS(0, tuuvm_arrayList_getSize(tuuvm_arrayList_create(tuuvm_test_context)));
        TEST_ASSERT_EQUALS(0, tuuvm_arrayList_getCapacity(tuuvm_arrayList_create(tuuvm_test_context)));

        tuuvm_arrayList_add(tuuvm_test_context, arrayList, tuuvm_tuple_uint8_encode(1));
        tuuvm_arrayList_add(tuuvm_test_context, arrayList, tuuvm_tuple_uint8_encode(2));
        tuuvm_arrayList_add(tuuvm_test_context, arrayList, tuuvm_tuple_uint8_encode(5));
        TEST_ASSERT_EQUALS(3, tuuvm_arrayList_getSize(arrayList));
        TEST_ASSERT_EQUALS(tuuvm_tuple_uint8_encode(1), tuuvm_arrayList_at(arrayList, 0));
        TEST_ASSERT_EQUALS(tuuvm_tuple_uint8_encode(2), tuuvm_arrayList_at(arrayList, 1));
        TEST_ASSERT_EQUALS(tuuvm_tuple_uint8_encode(5), tuuvm_arrayList_at(arrayList, 2));
    }

    TEST_CASE_WITH_FIXTURE(AsArraySlice, TuuvmCore)
    {
        tuuvm_tuple_t arrayList = tuuvm_arrayList_create(tuuvm_test_context);
        TEST_ASSERT(tuuvm_tuple_isNonNullPointer(arrayList));
        TEST_ASSERT_EQUALS(0, tuuvm_arrayList_getSize(tuuvm_arrayList_create(tuuvm_test_context)));
        TEST_ASSERT_EQUALS(0, tuuvm_arrayList_getCapacity(tuuvm_arrayList_create(tuuvm_test_context)));

        tuuvm_arrayList_add(tuuvm_test_context, arrayList, tuuvm_tuple_uint8_encode(1));
        tuuvm_arrayList_add(tuuvm_test_context, arrayList, tuuvm_tuple_uint8_encode(2));
        tuuvm_arrayList_add(tuuvm_test_context, arrayList, tuuvm_tuple_uint8_encode(5));
        TEST_ASSERT_EQUALS(3, tuuvm_arrayList_getSize(arrayList));
        TEST_ASSERT_EQUALS(tuuvm_tuple_uint8_encode(1), tuuvm_arrayList_at(arrayList, 0));
        TEST_ASSERT_EQUALS(tuuvm_tuple_uint8_encode(2), tuuvm_arrayList_at(arrayList, 1));
        TEST_ASSERT_EQUALS(tuuvm_tuple_uint8_encode(5), tuuvm_arrayList_at(arrayList, 2));

        tuuvm_tuple_t arraySlice = tuuvm_arrayList_asArraySlice(tuuvm_test_context, arrayList);
        TEST_ASSERT(tuuvm_tuple_isNonNullPointer(arraySlice));
        TEST_ASSERT_EQUALS(3, tuuvm_arraySlice_getSize(arraySlice));
        TEST_ASSERT_EQUALS(tuuvm_tuple_uint8_encode(1), tuuvm_arraySlice_at(arraySlice, 0));
        TEST_ASSERT_EQUALS(tuuvm_tuple_uint8_encode(2), tuuvm_arraySlice_at(arraySlice, 1));
        TEST_ASSERT_EQUALS(tuuvm_tuple_uint8_encode(5), tuuvm_arraySlice_at(arraySlice, 2));
    }

}
