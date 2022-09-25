#ifndef TUUVM_TEST_MACROS_H
#define TUUVM_TEST_MACROS_H
 
#include <stdio.h>

#define TEST_SUITE(testSuiteName) \
void TestSuite_##testSuiteName()

extern int tuuvm_test_currentHasError;
extern int tuuvm_test_runCount;
extern int tuuvm_test_errorCount;

#define RUN_TEST_SUITE(testSuiteName) \
    tuuvm_test_currentHasError = 0; \
    ++tuuvm_test_runCount; \
    TestSuite_##testSuiteName()

#define INCREMENT_TEST_ERROR_COUNT() \
    if(!tuuvm_test_currentHasError) { \
        ++tuuvm_test_errorCount; \
        tuuvm_test_currentHasError = 1; \
    }
#define TEST_ASSERT(expression) \
    if(!(expression)) { \
        printf("Assertion failure: " #expression); \
        INCREMENT_TEST_ERROR_COUNT() \
    }

#endif //TUUVM_TEST_MACROS_H
