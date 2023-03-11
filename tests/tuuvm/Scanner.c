#include "TestMacros.h"
#include "tuuvm/scanner.h"
#include "tuuvm/arraySlice.h"
#include "tuuvm/token.h"
#include "tuuvm/string.h"

TEST_SUITE(Scanner)
{
    TEST_CASE_WITH_FIXTURE(EmptyString, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "", "test", "tlisp");
        TEST_ASSERT_EQUALS(0, tuuvm_arraySlice_getSize(tokenList));
    }

    TEST_CASE_WITH_FIXTURE(SingleLineComment, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "## Single line comment", "test", "tlisp");
        TEST_ASSERT_EQUALS(0, tuuvm_arraySlice_getSize(tokenList));
    }

    TEST_CASE_WITH_FIXTURE(MultiLineComment, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "#*\n* Multi line comment\n*\n*#", "test", "tlisp");
        TEST_ASSERT_EQUALS(0, tuuvm_arraySlice_getSize(tokenList));
    }

    TEST_CASE_WITH_FIXTURE(Identifier, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "helloWorld1234", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_IDENTIFIER, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "helloWorld1234"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(ScopedIdentifier, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "Scope::helloWorld1234", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_IDENTIFIER, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "Scope::helloWorld1234"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(ScopedIdentifier2, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "Scope::SubScope::helloWorld1234", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_IDENTIFIER, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "Scope::SubScope::helloWorld1234"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(Operator, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "+", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_OPERATOR, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "+"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(Operator2, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "||", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_OPERATOR, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "||"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(ScopedOperator, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "Scope::+", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_OPERATOR, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "Scope::+"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(ScopedOperator2, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "Scope::SubScope::+", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_OPERATOR, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "Scope::SubScope::+"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(Keyword, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "test:", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_KEYWORD, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "test:"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(ScopedKeyword, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "Scope::test:", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_KEYWORD, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "Scope::test:"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(MultiKeyword, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "test:second:", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_MULTI_KEYWORD, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "test:second:"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(ScopedMultiKeyword, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "Scope::test:second:", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_MULTI_KEYWORD, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "Scope::test:second:"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(IdentifierSymbol, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "#helloWorld1234", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_SYMBOL, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "helloWorld1234"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(OperatorSymbol, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "#+", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_SYMBOL, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "+"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(Integer, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "12345", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_INTEGER, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(Float, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "12345.0", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_FLOAT, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(Character, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "'T'", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_CHARACTER, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS('T', tuuvm_tuple_char32_decode(tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 0))));
    }

    TEST_CASE_WITH_FIXTURE(String, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "\"Hello World\\r\\n\"", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_STRING, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT(tuuvm_string_equals(tuuvm_string_createWithCString(tuuvm_test_context, "Hello World\r\n"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 0))));
    }

    TEST_CASE_WITH_FIXTURE(StringSymbol, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "#\"Hello World\\r\\n\"", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_SYMBOL, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT(tuuvm_string_equals(tuuvm_string_createWithCString(tuuvm_test_context, "Hello World\r\n"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 0))));
    }

    TEST_CASE_WITH_FIXTURE(Delimiters, TuuvmCore)
    {
        tuuvm_tuple_t tokenList = tuuvm_scanner_scanCString(tuuvm_test_context, "() [] {} : :: `' `` `, `@ | .. ... ; * < >", "test", "tlisp");
        TEST_ASSERT_EQUALS(20, tuuvm_arraySlice_getSize(tokenList));
        
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_LPARENT, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "("), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 0)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_RPARENT, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 1)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, ")"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 1)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_LBRACKET, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 2)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "["), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 2)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_RBRACKET, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 3)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "]"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 3)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_LCBRACKET, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 4)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "{"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 4)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_RCBRACKET, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 5)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "}"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 5)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_COLON, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 6)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, ":"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 6)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_COLON_COLON, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 7)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "::"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 7)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_QUOTE, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 8)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "`'"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 8)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_QUASI_QUOTE, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 9)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "``"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 9)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_QUASI_UNQUOTE, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 10)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "`,"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 10)));
        
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_SPLICE, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 11)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "`@"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 11)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_BAR, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 12)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "|"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 12)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_DOT, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 13)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "."), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 13)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_DOT, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 14)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "."), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 14)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_ELLIPSIS, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 15)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "..."), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 15)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_SEMICOLON, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 16)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, ";"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 16)));
 
        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_STAR, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 17)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "*"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 17)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_LESS_THAN, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 18)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "<"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 18)));

        TEST_ASSERT_EQUALS(TUUVM_TOKEN_KIND_GREATER_THAN, tuuvm_token_getKind(tuuvm_arraySlice_at(tokenList, 19)));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, ">"), tuuvm_token_getValue(tuuvm_arraySlice_at(tokenList, 19)));
    }
}
