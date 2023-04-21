#include "TestMacros.h"
#include "sysbvm/orderedCollection.h"
#include "sysbvm/arraySlice.h"

TEST_SUITE(OrderedCollection)
{
    TEST_CASE_WITH_FIXTURE(Empty, TuuvmCore)
    {
        TEST_ASSERT(sysbvm_tuple_isNonNullPointer(sysbvm_orderedCollection_create(sysbvm_test_context)));
        TEST_ASSERT_EQUALS(0, sysbvm_orderedCollection_getSize(sysbvm_orderedCollection_create(sysbvm_test_context)));
        TEST_ASSERT_EQUALS(0, sysbvm_orderedCollection_getCapacity(sysbvm_orderedCollection_create(sysbvm_test_context)));
    }

    TEST_CASE_WITH_FIXTURE(Add1, TuuvmCore)
    {
        sysbvm_tuple_t orderedCollection = sysbvm_orderedCollection_create(sysbvm_test_context);
        TEST_ASSERT(sysbvm_tuple_isNonNullPointer(orderedCollection));
        TEST_ASSERT_EQUALS(0, sysbvm_orderedCollection_getSize(sysbvm_orderedCollection_create(sysbvm_test_context)));
        TEST_ASSERT_EQUALS(0, sysbvm_orderedCollection_getCapacity(sysbvm_orderedCollection_create(sysbvm_test_context)));

        sysbvm_orderedCollection_add(sysbvm_test_context, orderedCollection, sysbvm_tuple_uint8_encode(1));
        TEST_ASSERT_EQUALS(1, sysbvm_orderedCollection_getSize(orderedCollection));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(1), sysbvm_orderedCollection_at(orderedCollection, 0));
    }

    TEST_CASE_WITH_FIXTURE(Add2, TuuvmCore)
    {
        sysbvm_tuple_t orderedCollection = sysbvm_orderedCollection_create(sysbvm_test_context);
        TEST_ASSERT(sysbvm_tuple_isNonNullPointer(orderedCollection));
        TEST_ASSERT_EQUALS(0, sysbvm_orderedCollection_getSize(sysbvm_orderedCollection_create(sysbvm_test_context)));
        TEST_ASSERT_EQUALS(0, sysbvm_orderedCollection_getCapacity(sysbvm_orderedCollection_create(sysbvm_test_context)));

        sysbvm_orderedCollection_add(sysbvm_test_context, orderedCollection, sysbvm_tuple_uint8_encode(1));
        sysbvm_orderedCollection_add(sysbvm_test_context, orderedCollection, sysbvm_tuple_uint8_encode(2));
        TEST_ASSERT_EQUALS(2, sysbvm_orderedCollection_getSize(orderedCollection));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(1), sysbvm_orderedCollection_at(orderedCollection, 0));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(2), sysbvm_orderedCollection_at(orderedCollection, 1));
    }

    TEST_CASE_WITH_FIXTURE(Add3, TuuvmCore)
    {
        sysbvm_tuple_t orderedCollection = sysbvm_orderedCollection_create(sysbvm_test_context);
        TEST_ASSERT(sysbvm_tuple_isNonNullPointer(orderedCollection));
        TEST_ASSERT_EQUALS(0, sysbvm_orderedCollection_getSize(sysbvm_orderedCollection_create(sysbvm_test_context)));
        TEST_ASSERT_EQUALS(0, sysbvm_orderedCollection_getCapacity(sysbvm_orderedCollection_create(sysbvm_test_context)));

        sysbvm_orderedCollection_add(sysbvm_test_context, orderedCollection, sysbvm_tuple_uint8_encode(1));
        sysbvm_orderedCollection_add(sysbvm_test_context, orderedCollection, sysbvm_tuple_uint8_encode(2));
        sysbvm_orderedCollection_add(sysbvm_test_context, orderedCollection, sysbvm_tuple_uint8_encode(5));
        TEST_ASSERT_EQUALS(3, sysbvm_orderedCollection_getSize(orderedCollection));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(1), sysbvm_orderedCollection_at(orderedCollection, 0));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(2), sysbvm_orderedCollection_at(orderedCollection, 1));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(5), sysbvm_orderedCollection_at(orderedCollection, 2));
    }

    TEST_CASE_WITH_FIXTURE(AsArraySlice, TuuvmCore)
    {
        sysbvm_tuple_t orderedCollection = sysbvm_orderedCollection_create(sysbvm_test_context);
        TEST_ASSERT(sysbvm_tuple_isNonNullPointer(orderedCollection));
        TEST_ASSERT_EQUALS(0, sysbvm_orderedCollection_getSize(sysbvm_orderedCollection_create(sysbvm_test_context)));
        TEST_ASSERT_EQUALS(0, sysbvm_orderedCollection_getCapacity(sysbvm_orderedCollection_create(sysbvm_test_context)));

        sysbvm_orderedCollection_add(sysbvm_test_context, orderedCollection, sysbvm_tuple_uint8_encode(1));
        sysbvm_orderedCollection_add(sysbvm_test_context, orderedCollection, sysbvm_tuple_uint8_encode(2));
        sysbvm_orderedCollection_add(sysbvm_test_context, orderedCollection, sysbvm_tuple_uint8_encode(5));
        TEST_ASSERT_EQUALS(3, sysbvm_orderedCollection_getSize(orderedCollection));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(1), sysbvm_orderedCollection_at(orderedCollection, 0));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(2), sysbvm_orderedCollection_at(orderedCollection, 1));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(5), sysbvm_orderedCollection_at(orderedCollection, 2));

        sysbvm_tuple_t arraySlice = sysbvm_orderedCollection_asArraySlice(sysbvm_test_context, orderedCollection);
        TEST_ASSERT(sysbvm_tuple_isNonNullPointer(arraySlice));
        TEST_ASSERT_EQUALS(3, sysbvm_arraySlice_getSize(arraySlice));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(1), sysbvm_arraySlice_at(arraySlice, 0));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(2), sysbvm_arraySlice_at(arraySlice, 1));
        TEST_ASSERT_EQUALS(sysbvm_tuple_uint8_encode(5), sysbvm_arraySlice_at(arraySlice, 2));
    }

}
