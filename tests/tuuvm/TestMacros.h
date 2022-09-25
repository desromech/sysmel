#ifndef TUUVM_TEST_MACROS_H
#define TUUVM_TEST_MACROS_H
 
#include <stdio.h>

#define TEST_SUITE(testSuiteName) \
void TestSuite_##testSuiteName(void)

#define TEST_SUITE_FIXTURE_INITIALIZE(testSuiteFixtureName) \
void testSuiteFixtureName ## _initialize(void)

#define TEST_SUITE_FIXTURE_SHUTDOWN(testSuiteFixtureName) \
void testSuiteFixtureName ## _shutdown(void)

#define TEST_SUITE_WITH_FIXTURE(testSuiteName, testSuiteFixtureName) \
void TestSuite_##testSuiteName##_withFixture(void); \
void TestSuite_##testSuiteName(void) { \
    testSuiteFixtureName ## _initialize(); \
    TestSuite_##testSuiteName##_withFixture(); \
    testSuiteFixtureName ## _shutdown(); \
} \
void TestSuite_##testSuiteName##_withFixture(void)

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
