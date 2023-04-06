#include "TestMacros.h"
#include "sysbvm/scanner.h"
#include "sysbvm/arraySlice.h"
#include "sysbvm/token.h"
#include "sysbvm/string.h"

TEST_SUITE(Scanner)
{
    TEST_CASE_WITH_FIXTURE(EmptyString, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "", "test", "tlisp");
        TEST_ASSERT_EQUALS(0, sysbvm_arraySlice_getSize(tokenList));
    }

    TEST_CASE_WITH_FIXTURE(SingleLineComment, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "## Single line comment", "test", "tlisp");
        TEST_ASSERT_EQUALS(0, sysbvm_arraySlice_getSize(tokenList));
    }

    TEST_CASE_WITH_FIXTURE(MultiLineComment, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "#*\n* Multi line comment\n*\n*#", "test", "tlisp");
        TEST_ASSERT_EQUALS(0, sysbvm_arraySlice_getSize(tokenList));
    }

    TEST_CASE_WITH_FIXTURE(Identifier, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "helloWorld1234", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, sysbvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_IDENTIFIER, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "helloWorld1234"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(ScopedIdentifier, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "Scope::helloWorld1234", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, sysbvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_IDENTIFIER, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "Scope::helloWorld1234"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(ScopedIdentifier2, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "Scope::SubScope::helloWorld1234", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, sysbvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_IDENTIFIER, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "Scope::SubScope::helloWorld1234"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(Operator, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "+", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, sysbvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_OPERATOR, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "+"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(Operator2, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "||", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, sysbvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_OPERATOR, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "||"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(ScopedOperator, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "Scope::+", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, sysbvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_OPERATOR, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "Scope::+"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(ScopedOperator2, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "Scope::SubScope::+", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, sysbvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_OPERATOR, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "Scope::SubScope::+"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(Keyword, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "test:", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, sysbvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_KEYWORD, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "test:"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(ScopedKeyword, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "Scope::test:", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, sysbvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_KEYWORD, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "Scope::test:"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(MultiKeyword, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "test:second:", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, sysbvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_MULTI_KEYWORD, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "test:second:"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(ScopedMultiKeyword, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "Scope::test:second:", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, sysbvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_MULTI_KEYWORD, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "Scope::test:second:"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(IdentifierSymbol, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "#helloWorld1234", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, sysbvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_SYMBOL, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "helloWorld1234"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(OperatorSymbol, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "#+", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, sysbvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_SYMBOL, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "+"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(Integer, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "12345", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, sysbvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_INTEGER, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
    }


    TEST_CASE_WITH_FIXTURE(Integer2, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "12_345", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, sysbvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_INTEGER, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(IntegerWithRadix, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "16rFF00", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, sysbvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_INTEGER, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(IntegerWithRadix2, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "16rFF_00", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, sysbvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_INTEGER, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(Float, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "12345.0", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, sysbvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_FLOAT, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
    }

    TEST_CASE_WITH_FIXTURE(Character, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "'T'", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, sysbvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_CHARACTER, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS('T', sysbvm_tuple_char32_decode(sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 0))));
    }

    TEST_CASE_WITH_FIXTURE(String, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "\"Hello \\\"World\\r\\n\"", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, sysbvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_STRING, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT(sysbvm_string_equals(sysbvm_string_createWithCString(sysbvm_test_context, "Hello \"World\r\n"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 0))));
    }

    TEST_CASE_WITH_FIXTURE(StringSymbol, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "#\"Hello \\\"World\\r\\n\"", "test", "tlisp");
        TEST_ASSERT_EQUALS(1, sysbvm_arraySlice_getSize(tokenList));
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_SYMBOL, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT(sysbvm_string_equals(sysbvm_string_createWithCString(sysbvm_test_context, "Hello \"World\r\n"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 0))));
    }

    TEST_CASE_WITH_FIXTURE(Delimiters, TuuvmCore)
    {
        sysbvm_tuple_t tokenList = sysbvm_scanner_scanCString(sysbvm_test_context, "() [] {} : :: `' `` `, `@ | .. ... ; * < >", "test", "tlisp");
        TEST_ASSERT_EQUALS(20, sysbvm_arraySlice_getSize(tokenList));
        
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_LPARENT, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 0)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "("), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 0)));

        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_RPARENT, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 1)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, ")"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 1)));

        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_LBRACKET, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 2)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "["), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 2)));

        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_RBRACKET, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 3)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "]"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 3)));

        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_LCBRACKET, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 4)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "{"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 4)));

        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_RCBRACKET, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 5)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "}"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 5)));

        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_COLON, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 6)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, ":"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 6)));

        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_COLON_COLON, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 7)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "::"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 7)));

        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_QUOTE, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 8)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "`'"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 8)));

        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_QUASI_QUOTE, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 9)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "``"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 9)));

        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_QUASI_UNQUOTE, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 10)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "`,"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 10)));
        
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_SPLICE, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 11)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "`@"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 11)));

        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_BAR, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 12)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "|"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 12)));

        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_DOT, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 13)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "."), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 13)));

        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_DOT, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 14)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "."), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 14)));

        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_ELLIPSIS, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 15)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "..."), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 15)));

        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_SEMICOLON, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 16)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, ";"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 16)));
 
        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_STAR, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 17)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "*"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 17)));

        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_LESS_THAN, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 18)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "<"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 18)));

        TEST_ASSERT_EQUALS(SYSBVM_TOKEN_KIND_GREATER_THAN, sysbvm_token_getKind(sysbvm_arraySlice_at(tokenList, 19)));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, ">"), sysbvm_token_getValue(sysbvm_arraySlice_at(tokenList, 19)));
    }
}
