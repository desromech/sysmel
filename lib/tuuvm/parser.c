#include "tuuvm/parser.h"
#include "tuuvm/ast.h"
#include "tuuvm/scanner.h"
#include "tuuvm/sourceCode.h"
#include "tuuvm/sourcePosition.h"

TUUVM_API tuuvm_tuple_t tuuvm_parser_parseTokens(tuuvm_context_t *context, tuuvm_tuple_t sourceCode, tuuvm_tuple_t tokenSequence)
{
    return TUUVM_NULL_TUPLE;
}

TUUVM_API tuuvm_tuple_t tuuvm_parser_parseCString(tuuvm_context_t *context, const char *sourceCodeText, const char *sourceCodeName)
{
    tuuvm_tuple_t sourceCode = tuuvm_sourceCode_createWithCStrings(context, sourceCodeText, sourceCodeName);
    tuuvm_tuple_t tokenSequence = tuuvm_scanner_scan(context, sourceCode);
    return tuuvm_parser_parseTokens(context, sourceCode, tokenSequence);
}
