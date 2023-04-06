#ifndef SYSBVM_TEST_MACROS_H
#define SYSBVM_TEST_MACROS_H
 
#include <stdio.h>
#include <sysbvm/context.h>

#define DECLARE_TEST_SUITE(testSuiteName) \
void TestSuite_##testSuiteName(void)

#define DECLARE_TEST_SUITE_FIXTURE(testSuiteFixtureName) \
void testSuiteFixtureName ## _initialize(void); \
void testSuiteFixtureName ## _shutdown(void) \

#define TEST_SUITE(testSuiteName) \
void TestSuite_##testSuiteName(void)

#define TEST_CASE_WITH_FIXTURE(testCaseName, testSuiteFixtureName) \
    sysbvm_test_currentTestCaseName = #testCaseName; \
    sysbvm_test_currentHasError = 0; \
    ++sysbvm_test_runCount; \
    printf("---- " #testCaseName "..."); \
    testSuiteFixtureName##_initialize(); \
    for(int testCaseName##_isRunning = 1; testCaseName ## _isRunning; testCaseName##_isRunning = 0, testSuiteFixtureName##_shutdown(), printf(sysbvm_test_currentHasError ? "" : " Success\n"))

#define TEST_SUITE_FIXTURE_INITIALIZE(testSuiteFixtureName) \
void testSuiteFixtureName ## _initialize(void)

#define TEST_SUITE_FIXTURE_SHUTDOWN(testSuiteFixtureName) \
void testSuiteFixtureName ## _shutdown(void)

extern const char *sysbvm_test_currentTestSuiteName;
extern const char *sysbvm_test_currentTestCaseName;
extern int sysbvm_test_currentHasError;
extern int sysbvm_test_runCount;
extern int sysbvm_test_errorCount;

#define RUN_TEST_SUITE(testSuiteName) \
    sysbvm_test_currentTestSuiteName = #testSuiteName; \
    printf("Test suite " #testSuiteName "...\n"); \
    TestSuite_##testSuiteName()

#define INCREMENT_TEST_ERROR_COUNT() \
    if(!sysbvm_test_currentHasError) { \
        ++sysbvm_test_errorCount; \
        sysbvm_test_currentHasError = 1; \
    }

#define TEST_LINE_TO_STRING_(x) #x
#define TEST_LINE_TO_STRING(x) TEST_LINE_TO_STRING_(x)

#define TEST_ASSERT_DESCRIPTION(expression, description) \
    if(!(expression)) { \
        if(!sysbvm_test_currentHasError) printf("\n"); \
        printf(__FILE__ ":" TEST_LINE_TO_STRING(__LINE__) ": " description "\n"); \
        INCREMENT_TEST_ERROR_COUNT() \
    }

#define TEST_ASSERT(expression) TEST_ASSERT_DESCRIPTION(expression, "Assertion failure: " #expression)
#define TEST_ASSERT_EQUALS(expected, gotten) TEST_ASSERT_DESCRIPTION((expected) == (gotten), "Obtained value is not equal to " #expected)

extern sysbvm_context_t *sysbvm_test_context;
DECLARE_TEST_SUITE_FIXTURE(TuuvmCore);

#endif //SYSBVM_TEST_MACROS_H
