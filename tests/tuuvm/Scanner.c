#include "TestMacros.h"
#include "tuuvm/scanner.h"
#include "tuuvm/arraySlice.h"
#include "tuuvm/token.h"

TEST_SUITE(Scanner)
{
    TEST_CASE_WITH_FIXTURE(EmptyString, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "", "test");
        TEST_ASSERT_EQUALS(0, tuuvm_arraySlice_getSize(tokenList));
    }

    TEST_CASE_WITH_FIXTURE(SingleLineComment, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "## Single line comment", "test");
        TEST_ASSERT_EQUALS(0, tuuvm_arraySlice_getSize(tokenList));
    }

    TEST_CASE_WITH_FIXTURE(MultiLineComment, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "#*\n* Multi line comment\n*\n*#", "test");
        TEST_ASSERT_EQUALS(0, tuuvm_arraySlice_getSize(tokenList));
    }

    TEST_CASE_WITH_FIXTURE(Identifier, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "helloWorld1234", "test");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_IDENTIFIER, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(Keyword, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "test:", "test");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_KEYWORD, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(MultiKeyword, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "test:second:", "test");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_MULTI_KEYWORD, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(IdentifierSymbol, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "#helloWorld1234", "test");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_SYMBOL, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(Integer, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "12345", "test");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_INTEGER, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(Float, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "12345.0", "test");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_FLOAT, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(Delimiters, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "() [] {} : :: `' `` `, `@ |", "test");
        TEST_ASSERT_EQUALS(13, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_LPARENT, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_RPARENT, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 1)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_LBRACKET, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 2)));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_RBRACKET, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 3)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_LCBRACKET, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 4)));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_RCBRACKET, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 5)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_COLON, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 6)));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_COLON_COLON, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 7)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_QUOTE, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 8)));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_QUASI_QUOTE, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 9)));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_QUASI_UNQUOTE, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 10)));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_SPLICE, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 11)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_BAR, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 12)));
    }
}
