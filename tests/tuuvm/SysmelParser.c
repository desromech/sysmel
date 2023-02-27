#include "TestMacros.h"
#include "tuuvm/ast.h"
#include "tuuvm/arraySlice.h"
#include "tuuvm/sysmelParser.h"
#include "tuuvm/string.h"

TEST_SUITE(SysmelParser)
{
    TEST_CASE_WITH_FIXTURE(EmptyString, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_sysmelParser_parseCString(tuuvm_test_context, "", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(0, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));
    }

    TEST_CASE_WITH_FIXTURE(SingleLineComment, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_sysmelParser_parseCString(tuuvm_test_context, "## Comment", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(0, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));
    }

    TEST_CASE_WITH_FIXTURE(MultiLineComment, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_sysmelParser_parseCString(tuuvm_test_context, "#* \nMulti line comment \n *#", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(0, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));
    }

    TEST_CASE_WITH_FIXTURE(IdentifierReference, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_sysmelParser_parseCString(tuuvm_test_context, "identifier", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));

        tuuvm_tuple_t node = tuuvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(tuuvm_astNode_isIdentifierReferenceNode(tuuvm_test_context, node));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "identifier"), tuuvm_astIdentifierReferenceNode_getValue(node));
    }

    TEST_CASE_WITH_FIXTURE(LiteralInteger, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_sysmelParser_parseCString(tuuvm_test_context, "1234", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));

        tuuvm_tuple_t node = tuuvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(tuuvm_astNode_isLiteralNode(tuuvm_test_context, node));
    }

    TEST_CASE_WITH_FIXTURE(LiteralSymbol, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_sysmelParser_parseCString(tuuvm_test_context, "#test", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));

        tuuvm_tuple_t node = tuuvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(tuuvm_astNode_isLiteralNode(tuuvm_test_context, node));
        TEST_ASSERT_EQUALS(tuuvm_symbol_internWithCString(tuuvm_test_context, "test"), tuuvm_astLiteralNode_getValue(node));
    }

    TEST_CASE_WITH_FIXTURE(LexicalBlock, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_sysmelParser_parseCString(tuuvm_test_context, "{}", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));

        tuuvm_tuple_t node = tuuvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(tuuvm_astNode_isLexicalBlockNode(tuuvm_test_context, node));
    }

    TEST_CASE_WITH_FIXTURE(MessageWithoutReceiver, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_sysmelParser_parseCString(tuuvm_test_context, "let: #x with: 42", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));

        tuuvm_tuple_t node = tuuvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(tuuvm_astNode_isUnexpandedApplicationNode(tuuvm_test_context, node));
    }

    TEST_CASE_WITH_FIXTURE(KeywordMessage, TuuvmCore)
    {
        tuuvm_tuple_t sequenceNode = tuuvm_sysmelParser_parseCString(tuuvm_test_context, "a perform: #yourself", "test");
        TEST_ASSERT(tuuvm_astNode_isSequenceNode(tuuvm_test_context, sequenceNode));
        TEST_ASSERT_EQUALS(1, tuuvm_astSequenceNode_getExpressionCount(sequenceNode));

        tuuvm_tuple_t node = tuuvm_astSequenceNode_getExpressionAt(sequenceNode, 0);
        TEST_ASSERT(tuuvm_astNode_isMessageSendNode(tuuvm_test_context, node));
    }

}
