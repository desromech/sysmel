#include "TestMacros.h"
#include <tuuvm/string.h>

TEST_SUITE(String)
{
    TEST_CASE_WITH_FIXTURE(FromCString, TuuvmCore)
    {
        tuuvm_tuple_t helloWorld = tuuvm_string_createWithCString(tuuvm_test_context, "Hello World\n");
        TEST_ASSERT(tuuvm_tuple_isBytes(helloWorld));
        TEST_ASSERT_EQUALS(13, tuuvm_tuple_getSizeInBytes(helloWorld));
    }

}
