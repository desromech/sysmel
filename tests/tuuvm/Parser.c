#include "TestMacros.h"
#include "tuuvm/ast.h"
#include "tuuvm/arraySlice.h"
#include "tuuvm/parser.h"
#include "tuuvm/string.h"

TEST_SUITE(Parser)
{
    TEST_CASE_WITH_FIXTURE(EmptyString, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_parser_parseCString(tuuvm_test_context, "", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(0, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));
    }

    TEST_CASE_WITH_FIXTURE(SingleLineComment, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_parser_parseCString(tuuvm_test_context, "## Comment", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(0, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));
    }

    TEST_CASE_WITH_FIXTURE(MultiLineComment, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_parser_parseCString(tuuvm_test_context, "#* \nMulti line comment \n *#", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(0, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));
    }

    TEST_CASE_WITH_FIXTURE(IdentifierReference, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_parser_parseCString(tuuvm_test_context, "identifier", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));

        tuuvm_tuple_t node = tuuvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(tuuvm_astNode_isIdentifierReferenceNode(tuuvm_test_context, node));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "identifier"), tuuvm_astIdentifierReferenceNode_getValue(node));
    }

    TEST_CASE_WITH_FIXTURE(LiteralInteger, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_parser_parseCString(tuuvm_test_context, "1234", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));

        tuuvm_tuple_t node = tuuvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(tuuvm_astNode_isLiteralNode(tuuvm_test_context, node));
    }

    TEST_CASE_WITH_FIXTURE(LiteralSymbol, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_parser_parseCString(tuuvm_test_context, "#test", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));

        tuuvm_tuple_t node = tuuvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(tuuvm_astNode_isLiteralNode(tuuvm_test_context, node));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "test"), tuuvm_astLiteralNode_getValue(node));
    }

    TEST_CASE_WITH_FIXTURE(UnexpandedNullaryApplication, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_parser_parseCString(tuuvm_test_context, "(function)", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));

        tuuvm_tuple_t node = tuuvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(tuuvm_astNode_isUnexpandedApplicationNode(tuuvm_test_context, node));

        tuuvm_tuple_t functionOrMacroExpression = tuuvm_astUnexpandedApplicationNode_getFunctionOrMacroExpression(node);
        TEST_ASSERT(tuuvm_astNode_isIdentifierReferenceNode(tuuvm_test_context, functionOrMacroExpression));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "function"), tuuvm_astIdentifierReferenceNode_getValue(functionOrMacroExpression));

        tuuvm_tuple_t arguments = tuuvm_astUnexpandedApplicationNode_getArguments(node);
        TEST_ASSERT_EQUALS(0, tuuvm_arraySlice_getSize(arguments));
    }

    TEST_CASE_WITH_FIXTURE(UnexpandedNullaryApplication2, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_parser_parseCString(tuuvm_test_context, "[function]", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));

        tuuvm_tuple_t node = tuuvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(tuuvm_astNode_isUnexpandedApplicationNode(tuuvm_test_context, node));

        tuuvm_tuple_t functionOrMacroExpression = tuuvm_astUnexpandedApplicationNode_getFunctionOrMacroExpression(node);
        TEST_ASSERT(tuuvm_astNode_isIdentifierReferenceNode(tuuvm_test_context, functionOrMacroExpression));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "function"), tuuvm_astIdentifierReferenceNode_getValue(functionOrMacroExpression));

        tuuvm_tuple_t arguments = tuuvm_astUnexpandedApplicationNode_getArguments(node);
        TEST_ASSERT_EQUALS(0, tuuvm_arraySlice_getSize(arguments));
    }

    TEST_CASE_WITH_FIXTURE(UnexpandedNullaryApplication3, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_parser_parseCString(tuuvm_test_context, "{function}", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));

        tuuvm_tuple_t node = tuuvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(tuuvm_astNode_isUnexpandedApplicationNode(tuuvm_test_context, node));

        tuuvm_tuple_t functionOrMacroExpression = tuuvm_astUnexpandedApplicationNode_getFunctionOrMacroExpression(node);
        TEST_ASSERT(tuuvm_astNode_isIdentifierReferenceNode(tuuvm_test_context, functionOrMacroExpression));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "function"), tuuvm_astIdentifierReferenceNode_getValue(functionOrMacroExpression));

        tuuvm_tuple_t arguments = tuuvm_astUnexpandedApplicationNode_getArguments(node);
        TEST_ASSERT_EQUALS(0, tuuvm_arraySlice_getSize(arguments));
    }

    TEST_CASE_WITH_FIXTURE(UnexpandedUnaryApplication, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_parser_parseCString(tuuvm_test_context, "(function first)", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));

        tuuvm_tuple_t node = tuuvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(tuuvm_astNode_isUnexpandedApplicationNode(tuuvm_test_context, node));

        tuuvm_tuple_t functionOrMacroExpression = tuuvm_astUnexpandedApplicationNode_getFunctionOrMacroExpression(node);
        TEST_ASSERT(tuuvm_astNode_isIdentifierReferenceNode(tuuvm_test_context, functionOrMacroExpression));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "function"), tuuvm_astIdentifierReferenceNode_getValue(functionOrMacroExpression));

        tuuvm_tuple_t arguments = tuuvm_astUnexpandedApplicationNode_getArguments(node);
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(arguments));

        tuuvm_tuple_t argument = tuuvm_arraySlice_at(arguments, 0);
        TEST_ASSERT(tuuvm_astNode_isIdentifierReferenceNode(tuuvm_test_context, argument));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "first"), tuuvm_astIdentifierReferenceNode_getValue(argument));
    }

    TEST_CASE_WITH_FIXTURE(UnexpandedUnaryApplication2, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_parser_parseCString(tuuvm_test_context, "[function first]", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));

        tuuvm_tuple_t node = tuuvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(tuuvm_astNode_isUnexpandedApplicationNode(tuuvm_test_context, node));

        tuuvm_tuple_t functionOrMacroExpression = tuuvm_astUnexpandedApplicationNode_getFunctionOrMacroExpression(node);
        TEST_ASSERT(tuuvm_astNode_isIdentifierReferenceNode(tuuvm_test_context, functionOrMacroExpression));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "function"), tuuvm_astIdentifierReferenceNode_getValue(functionOrMacroExpression));

        tuuvm_tuple_t arguments = tuuvm_astUnexpandedApplicationNode_getArguments(node);
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(arguments));

        tuuvm_tuple_t argument = tuuvm_arraySlice_at(arguments, 0);
        TEST_ASSERT(tuuvm_astNode_isIdentifierReferenceNode(tuuvm_test_context, argument));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "first"), tuuvm_astIdentifierReferenceNode_getValue(argument));
    }

    TEST_CASE_WITH_FIXTURE(UnexpandedUnaryApplication3, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_parser_parseCString(tuuvm_test_context, "{function first}", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));

        tuuvm_tuple_t node = tuuvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(tuuvm_astNode_isUnexpandedApplicationNode(tuuvm_test_context, node));

        tuuvm_tuple_t functionOrMacroExpression = tuuvm_astUnexpandedApplicationNode_getFunctionOrMacroExpression(node);
        TEST_ASSERT(tuuvm_astNode_isIdentifierReferenceNode(tuuvm_test_context, functionOrMacroExpression));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "function"), tuuvm_astIdentifierReferenceNode_getValue(functionOrMacroExpression));

        tuuvm_tuple_t arguments = tuuvm_astUnexpandedApplicationNode_getArguments(node);
        TEST_ASSERT_EQUALS(1, tuuvm_arraySlice_getSize(arguments));

        tuuvm_tuple_t argument = tuuvm_arraySlice_at(arguments, 0);
        TEST_ASSERT(tuuvm_astNode_isIdentifierReferenceNode(tuuvm_test_context, argument));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "first"), tuuvm_astIdentifierReferenceNode_getValue(argument));
    }

    TEST_CASE_WITH_FIXTURE(UnexpandedBinaryApplication, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_parser_parseCString(tuuvm_test_context, "(function first second)", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));

        tuuvm_tuple_t node = tuuvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(tuuvm_astNode_isUnexpandedApplicationNode(tuuvm_test_context, node));

        tuuvm_tuple_t functionOrMacroExpression = tuuvm_astUnexpandedApplicationNode_getFunctionOrMacroExpression(node);
        TEST_ASSERT(tuuvm_astNode_isIdentifierReferenceNode(tuuvm_test_context, functionOrMacroExpression));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "function"), tuuvm_astIdentifierReferenceNode_getValue(functionOrMacroExpression));

        tuuvm_tuple_t arguments = tuuvm_astUnexpandedApplicationNode_getArguments(node);
        TEST_ASSERT_EQUALS(2, tuuvm_arraySlice_getSize(arguments));

        {
            tuuvm_tuple_t argument = tuuvm_arraySlice_at(arguments, 0);
            TEST_ASSERT(tuuvm_astNode_isIdentifierReferenceNode(tuuvm_test_context, argument));
            TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "first"), tuuvm_astIdentifierReferenceNode_getValue(argument));
        }

        {
            tuuvm_tuple_t argument = tuuvm_arraySlice_at(arguments, 1);
            TEST_ASSERT(tuuvm_astNode_isIdentifierReferenceNode(tuuvm_test_context, argument));
            TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "second"), tuuvm_astIdentifierReferenceNode_getValue(argument));
        }
    }

    TEST_CASE_WITH_FIXTURE(UnexpandedBinaryApplication2, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_parser_parseCString(tuuvm_test_context, "[function first second]", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));

        tuuvm_tuple_t node = tuuvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(tuuvm_astNode_isUnexpandedApplicationNode(tuuvm_test_context, node));

        tuuvm_tuple_t functionOrMacroExpression = tuuvm_astUnexpandedApplicationNode_getFunctionOrMacroExpression(node);
        TEST_ASSERT(tuuvm_astNode_isIdentifierReferenceNode(tuuvm_test_context, functionOrMacroExpression));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "function"), tuuvm_astIdentifierReferenceNode_getValue(functionOrMacroExpression));

        tuuvm_tuple_t arguments = tuuvm_astUnexpandedApplicationNode_getArguments(node);
        TEST_ASSERT_EQUALS(2, tuuvm_arraySlice_getSize(arguments));

        {
            tuuvm_tuple_t argument = tuuvm_arraySlice_at(arguments, 0);
            TEST_ASSERT(tuuvm_astNode_isIdentifierReferenceNode(tuuvm_test_context, argument));
            TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "first"), tuuvm_astIdentifierReferenceNode_getValue(argument));
        }

        {
            tuuvm_tuple_t argument = tuuvm_arraySlice_at(arguments, 1);
            TEST_ASSERT(tuuvm_astNode_isIdentifierReferenceNode(tuuvm_test_context, argument));
            TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "second"), tuuvm_astIdentifierReferenceNode_getValue(argument));
        }
    }

    TEST_CASE_WITH_FIXTURE(UnexpandedBinaryApplication2, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_parser_parseCString(tuuvm_test_context, "{function first second}", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));

        tuuvm_tuple_t node = tuuvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(tuuvm_astNode_isUnexpandedApplicationNode(tuuvm_test_context, node));

        tuuvm_tuple_t functionOrMacroExpression = tuuvm_astUnexpandedApplicationNode_getFunctionOrMacroExpression(node);
        TEST_ASSERT(tuuvm_astNode_isIdentifierReferenceNode(tuuvm_test_context, functionOrMacroExpression));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "function"), tuuvm_astIdentifierReferenceNode_getValue(functionOrMacroExpression));

        tuuvm_tuple_t arguments = tuuvm_astUnexpandedApplicationNode_getArguments(node);
        TEST_ASSERT_EQUALS(2, tuuvm_arraySlice_getSize(arguments));

        {
            tuuvm_tuple_t argument = tuuvm_arraySlice_at(arguments, 0);
            TEST_ASSERT(tuuvm_astNode_isIdentifierReferenceNode(tuuvm_test_context, argument));
            TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "first"), tuuvm_astIdentifierReferenceNode_getValue(argument));
        }

        {
            tuuvm_tuple_t argument = tuuvm_arraySlice_at(arguments, 1);
            TEST_ASSERT(tuuvm_astNode_isIdentifierReferenceNode(tuuvm_test_context, argument));
            TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "second"), tuuvm_astIdentifierReferenceNode_getValue(argument));
        }
    }
}
