#include "TestMacros.h"
#include "sysbvm/ast.h"
#include "sysbvm/array.h"
#include "sysbvm/parser.h"
#include "sysbvm/string.h"

TEST_SUITE(Parser)
{
    TEST_CASE_WITH_FIXTURE(EmptyString, TuuvmCore)
    {
        sysbvm_tuple_t sequenceNode = sysbvm_parser_parseCString(sysbvm_test_context, "", "test");
        TEST_ASSERT(sysbvm_astNode_isSequenceNode(sysbvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(0, sysbvm_astSequenceNode_getExpressionCount(sequenceNode));
    }

    TEST_CASE_WITH_FIXTURE(SingleLineComment, TuuvmCore)
    {
        sysbvm_tuple_t sequenceNode = sysbvm_parser_parseCString(sysbvm_test_context, "## Comment", "test");
        TEST_ASSERT(sysbvm_astNode_isSequenceNode(sysbvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(0, sysbvm_astSequenceNode_getExpressionCount(sequenceNode));
    }

    TEST_CASE_WITH_FIXTURE(MultiLineComment, TuuvmCore)
    {
        sysbvm_tuple_t sequenceNode = sysbvm_parser_parseCString(sysbvm_test_context, "#* \nMulti line comment \n *#", "test");
        TEST_ASSERT(sysbvm_astNode_isSequenceNode(sysbvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(0, sysbvm_astSequenceNode_getExpressionCount(sequenceNode));
    }

    TEST_CASE_WITH_FIXTURE(IdentifierReference, TuuvmCore)
    {
        sysbvm_tuple_t sequenceNode = sysbvm_parser_parseCString(sysbvm_test_context, "identifier", "test");
        TEST_ASSERT(sysbvm_astNode_isSequenceNode(sysbvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, sysbvm_astSequenceNode_getExpressionCount(sequenceNode));

        sysbvm_tuple_t node = sysbvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(sysbvm_astNode_isIdentifierReferenceNode(sysbvm_test_context, node));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "identifier"), sysbvm_astIdentifierReferenceNode_getValue(node));
    }

    TEST_CASE_WITH_FIXTURE(LiteralInteger, TuuvmCore)
    {
        sysbvm_tuple_t sequenceNode = sysbvm_parser_parseCString(sysbvm_test_context, "1234", "test");
        TEST_ASSERT(sysbvm_astNode_isSequenceNode(sysbvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, sysbvm_astSequenceNode_getExpressionCount(sequenceNode));

        sysbvm_tuple_t node = sysbvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(sysbvm_astNode_isLiteralNode(sysbvm_test_context, node));
    }

    TEST_CASE_WITH_FIXTURE(LiteralSymbol, TuuvmCore)
    {
        sysbvm_tuple_t sequenceNode = sysbvm_parser_parseCString(sysbvm_test_context, "#test", "test");
        TEST_ASSERT(sysbvm_astNode_isSequenceNode(sysbvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, sysbvm_astSequenceNode_getExpressionCount(sequenceNode));

        sysbvm_tuple_t node = sysbvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(sysbvm_astNode_isLiteralNode(sysbvm_test_context, node));
        TEST_ASSERT_EQUALS(sysbvm_symbol_internWithCString(sysbvm_test_context, "test"), sysbvm_astLiteralNode_getValue(node));
    }

    TEST_CASE_WITH_FIXTURE(UnexpandedEmptySExpression, TuuvmCore)
    {
        sysbvm_tuple_t sequenceNode = sysbvm_parser_parseCString(sysbvm_test_context, "()", "test");
        TEST_ASSERT(sysbvm_astNode_isSequenceNode(sysbvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, sysbvm_astSequenceNode_getExpressionCount(sequenceNode));

        sysbvm_tuple_t node = sysbvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(sysbvm_astNode_isUnexpandedSExpressionNode(sysbvm_test_context, node));

        sysbvm_tuple_t elements = sysbvm_astUnexpandedSExpressionNode_getElements(node);
        TEST_ASSERT_EQUALS(0, sysbvm_array_getSize(elements));
    }

    TEST_CASE_WITH_FIXTURE(UnexpandedSExpression, TuuvmCore)
    {
        sysbvm_tuple_t sequenceNode = sysbvm_parser_parseCString(sysbvm_test_context, "(function)", "test");
        TEST_ASSERT(sysbvm_astNode_isSequenceNode(sysbvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, sysbvm_astSequenceNode_getExpressionCount(sequenceNode));

        sysbvm_tuple_t node = sysbvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(sysbvm_astNode_isUnexpandedSExpressionNode(sysbvm_test_context, node));

        sysbvm_tuple_t elements = sysbvm_astUnexpandedSExpressionNode_getElements(node);
        TEST_ASSERT_EQUALS(1, sysbvm_array_getSize(elements));
    }
}
