#include "tuuvm/interpreter.h"
#include "tuuvm/environment.h"
#include "tuuvm/array.h"
#include "tuuvm/arraySlice.h"
#include "tuuvm/ast.h"
#include "tuuvm/assert.h"
#include "tuuvm/function.h"
#include "tuuvm/errors.h"
#include "tuuvm/macro.h"
#include "tuuvm/parser.h"
#include "tuuvm/type.h"
#include "internal/context.h"

TUUVM_API tuuvm_tuple_t tuuvm_interpreter_analyzeASTWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t astNode, tuuvm_tuple_t environment)
{
    tuuvm_tuple_t function = tuuvm_type_getAstNodeAnalysisFunction(tuuvm_tuple_getType(context, astNode));
    if(!function)
        tuuvm_error("Cannot analyze non AST node tuple.");

    return tuuvm_function_apply2(context, function, astNode, environment);
}

TUUVM_API tuuvm_tuple_t tuuvm_interpreter_evaluateASTWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t astNode, tuuvm_tuple_t environment)
{
    tuuvm_tuple_t function = tuuvm_type_getAstNodeEvaluationFunction(tuuvm_tuple_getType(context, astNode));
    if(!function)
        tuuvm_error("Cannot evaluate non AST node tuple.");

    return tuuvm_function_apply2(context, function, astNode, environment);
}

TUUVM_API tuuvm_tuple_t tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t astNode, tuuvm_tuple_t environment)
{
    tuuvm_tuple_t function = tuuvm_type_getAstNodeAnalysisAndEvaluationFunction(tuuvm_tuple_getType(context, astNode));
    if(!function)
        return tuuvm_interpreter_evaluateASTWithEnvironment(context, tuuvm_interpreter_analyzeASTWithEnvironment(context, astNode, environment), environment);

    return tuuvm_function_apply2(context, function, astNode, environment);
}

TUUVM_API tuuvm_tuple_t tuuvm_interpreter_analyzeAndEvaluateCStringWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t environment, const char *sourceCodeText, const char *sourceCodeName)
{
    return tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, tuuvm_parser_parseCString(context, sourceCodeText, sourceCodeName), environment);
}

static tuuvm_tuple_t tuuvm_astSequenceNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t node = arguments[0];
    tuuvm_tuple_t environment = arguments[1];

    tuuvm_astSequenceNode_t *sequenceNode = (tuuvm_astSequenceNode_t*)node;
    tuuvm_tuple_t expressions = sequenceNode->expressions;
    size_t expressionCount = tuuvm_arraySlice_getSize(expressions);
    if(expressionCount == 0)
        return node;
    
    tuuvm_tuple_t analyzedExpressions = tuuvm_arraySlice_createWithArrayOfSize(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        tuuvm_tuple_t expression = tuuvm_arraySlice_at(expressions, i);
        tuuvm_tuple_t analyzedExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, expression, environment);
        tuuvm_arraySlice_atPut(analyzedExpressions, i, analyzedExpression);
    }

    return tuuvm_astSequenceNode_create(context, sequenceNode->super.sourcePosition, analyzedExpressions);
}

static tuuvm_tuple_t tuuvm_astSequenceNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t node = arguments[0];
    tuuvm_tuple_t environment = arguments[1];

    tuuvm_tuple_t result = TUUVM_VOID_TUPLE;

    tuuvm_tuple_t expressions = ((tuuvm_astSequenceNode_t*)node)->expressions;
    size_t expressionCount = tuuvm_arraySlice_getSize(expressions);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        tuuvm_tuple_t expression = tuuvm_arraySlice_at(expressions, i);
        result = tuuvm_interpreter_evaluateASTWithEnvironment(context, expression, environment);
    }

    return result;
}

static tuuvm_tuple_t tuuvm_astSequenceNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t node = arguments[0];
    tuuvm_tuple_t environment = arguments[1];

    tuuvm_tuple_t result = TUUVM_VOID_TUPLE;

    tuuvm_tuple_t expressions = ((tuuvm_astSequenceNode_t*)node)->expressions;
    size_t expressionCount = tuuvm_arraySlice_getSize(expressions);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        tuuvm_tuple_t expression = tuuvm_arraySlice_at(expressions, i);
        result = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, expression, environment);
    }

    return result;
}

static tuuvm_tuple_t tuuvm_astLiteralNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t node = arguments[0];

    return node;
}

static tuuvm_tuple_t tuuvm_astLiteralNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t node = arguments[0];

    return ((tuuvm_astLiteralNode_t*)node)->value;
}

static tuuvm_tuple_t tuuvm_astIdentifierReferenceNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t node = arguments[0];
    tuuvm_tuple_t environment = arguments[1];

    tuuvm_astIdentifierReferenceNode_t *referenceNode = (tuuvm_astIdentifierReferenceNode_t*)node;
    tuuvm_tuple_t binding;

    // Attempt to replace the symbol with its binding, if it exists.
    if(tuuvm_environment_lookSymbolRecursively(context, environment, referenceNode->value, &binding))
        return tuuvm_astLiteralNode_create(context, referenceNode->super.sourcePosition, referenceNode->value);

    return node;
}

static tuuvm_tuple_t tuuvm_astIdentifierReferenceNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t node = arguments[0];
    tuuvm_tuple_t environment = arguments[1];

    tuuvm_astIdentifierReferenceNode_t *referenceNode = (tuuvm_astIdentifierReferenceNode_t*)node;
    tuuvm_tuple_t binding;

    if(tuuvm_environment_lookSymbolRecursively(context, environment, referenceNode->value, &binding))
        return binding;

    tuuvm_error("Failed to find symbol binding");
    return TUUVM_NULL_TUPLE;
}

static tuuvm_tuple_t tuuvm_astUnexpandedApplicationNode_expandNodeWithMacro(tuuvm_context_t *context, tuuvm_tuple_t node, tuuvm_tuple_t macro)
{
    tuuvm_astUnexpandedApplicationNode_t *unexpandedNode = (tuuvm_astUnexpandedApplicationNode_t*)node;

    TUUVM_ASSERT(tuuvm_function_isMacro(context, macro));
    bool isVariadic = tuuvm_function_isVariadic(context, macro);
    size_t expectedArgumentCount = tuuvm_function_getArgumentCount(context, macro);
    size_t applicationArgumentCount = tuuvm_arraySlice_getSize(unexpandedNode->arguments);
    tuuvm_tuple_t applicationArguments[TUUVM_MAX_FUNCTION_ARGUMENTS];

    if(isVariadic)
    {
        TUUVM_ASSERT(expectedArgumentCount >= 2);
        size_t directArgumentCount = expectedArgumentCount - /* Macro context */ 1 - /* Variadic argument */ 1;
        if(applicationArgumentCount < directArgumentCount)
            tuuvm_error("Macro application is missing required arguments.");
        else if(directArgumentCount + 1> TUUVM_MAX_FUNCTION_ARGUMENTS)
            tuuvm_error("Macro application direct arguments exceeds the max argument count.");

        applicationArguments[0] = tuuvm_macroContext_create(context, node, unexpandedNode->super.sourcePosition);
        for(size_t i = 0; i < directArgumentCount; ++i)
            applicationArguments[i + 1] = tuuvm_arraySlice_at(unexpandedNode->arguments, i);

        applicationArguments[expectedArgumentCount - 1] = tuuvm_arraySlice_fromOffset(context, unexpandedNode->arguments, directArgumentCount);
    }
    else
    {
        TUUVM_ASSERT(expectedArgumentCount >= 1);
        size_t directArgumentCount = expectedArgumentCount - 1;
        if(applicationArgumentCount != directArgumentCount)
            tuuvm_error("Macro application exceeds the max argument count.");
        else if(expectedArgumentCount > TUUVM_MAX_FUNCTION_ARGUMENTS)
            tuuvm_error("Macro application exceeds the max argument count.");

        applicationArguments[0] = tuuvm_macroContext_create(context, node, unexpandedNode->super.sourcePosition);
        for(size_t i = 0; i < directArgumentCount; ++i)
            applicationArguments[i + 1] = tuuvm_arraySlice_at(unexpandedNode->arguments, i);
    }

    return tuuvm_function_apply(context, macro, expectedArgumentCount, applicationArguments);
}

static tuuvm_tuple_t tuuvm_astUnexpandedApplicationNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t node = arguments[0];
    tuuvm_tuple_t environment = arguments[1];

    tuuvm_astUnexpandedApplicationNode_t *unexpandedNode = (tuuvm_astUnexpandedApplicationNode_t*)node;
    tuuvm_tuple_t functionOrMacroExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, unexpandedNode->functionOrMacroExpression, environment);

    // Is this a macro?
    bool isMacro = tuuvm_astNode_isMacroExpression(context, functionOrMacroExpression);
    if(isMacro)
    {
        tuuvm_tuple_t macro = tuuvm_interpreter_evaluateASTWithEnvironment(context, functionOrMacroExpression, environment);
        tuuvm_tuple_t expansionResult = tuuvm_astUnexpandedApplicationNode_expandNodeWithMacro(context, node, macro);
        return tuuvm_interpreter_analyzeASTWithEnvironment(context, expansionResult, environment);
    }

    // Convert into application node and then analyze it.
    tuuvm_tuple_t applicationNode = tuuvm_astFunctionApplicationNode_create(context, unexpandedNode->super.sourcePosition, functionOrMacroExpression, unexpandedNode->arguments);
    return tuuvm_interpreter_analyzeASTWithEnvironment(context, applicationNode, environment);
}

static tuuvm_tuple_t tuuvm_astUnexpandedApplicationNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t node = arguments[0];
    tuuvm_tuple_t environment = arguments[1];

    tuuvm_astUnexpandedApplicationNode_t *unexpandedNode = (tuuvm_astUnexpandedApplicationNode_t*)node;
    tuuvm_tuple_t functionOrMacro = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, unexpandedNode->functionOrMacroExpression, environment);
    bool isMacro = tuuvm_function_isMacro(context, functionOrMacro);

    if(isMacro)
    {
        tuuvm_tuple_t expansionResult = tuuvm_astUnexpandedApplicationNode_expandNodeWithMacro(context, node, functionOrMacro);
        return tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, expansionResult, environment);
    }

    TUUVM_ASSERT(!tuuvm_function_isVariadic(context, functionOrMacro));
    tuuvm_tuple_t applicationArguments[TUUVM_MAX_FUNCTION_ARGUMENTS];
    size_t applicationArgumentCount = tuuvm_arraySlice_getSize(unexpandedNode->arguments);
    if(applicationArgumentCount > TUUVM_MAX_FUNCTION_ARGUMENTS)
        tuuvm_error("Function applicatio exceeds the max argument count.");

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        tuuvm_tuple_t argumentNode = tuuvm_arraySlice_at(unexpandedNode->arguments, i);
        applicationArguments[i] = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, argumentNode, environment);
    }

    return tuuvm_function_apply(context, functionOrMacro, applicationArgumentCount, applicationArguments);
}

static tuuvm_tuple_t tuuvm_astUnexpandedSExpressionNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t node = arguments[0];
    tuuvm_tuple_t environment = arguments[1];

    tuuvm_astUnexpandedSExpressionNode_t *unexpandedSExpressionNode = (tuuvm_astUnexpandedSExpressionNode_t*)node;
    size_t elementCount = tuuvm_arraySlice_getSize(unexpandedSExpressionNode->elements);
    if(elementCount == 0)
    {
        // Empty array.
        tuuvm_tuple_t array = tuuvm_array_create(context, 0);
        tuuvm_tuple_t literalNode = tuuvm_astLiteralNode_create(context, unexpandedSExpressionNode->super.sourcePosition, array);
        return tuuvm_interpreter_analyzeASTWithEnvironment(context, literalNode, environment);
    }
    else
    {
        // Unexpanded application node.
        tuuvm_tuple_t functionOrMacroExpression = tuuvm_arraySlice_at(unexpandedSExpressionNode->elements, 0);
        tuuvm_tuple_t argumentExpressions = tuuvm_arraySlice_fromOffset(context, unexpandedSExpressionNode->elements, 1);
        tuuvm_tuple_t unexpandedApplicationNode = tuuvm_astUnexpandedApplicationNode_create(context, unexpandedSExpressionNode->super.sourcePosition, functionOrMacroExpression, argumentExpressions);
        return tuuvm_interpreter_analyzeASTWithEnvironment(context, unexpandedApplicationNode, environment);
    }
}

static tuuvm_tuple_t tuuvm_astUnexpandedSExpressionNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t node = arguments[0];
    tuuvm_tuple_t environment = arguments[1];

    tuuvm_astUnexpandedSExpressionNode_t *unexpandedSExpressionNode = (tuuvm_astUnexpandedSExpressionNode_t*)node;
    size_t elementCount = tuuvm_arraySlice_getSize(unexpandedSExpressionNode->elements);
    if(elementCount == 0)
    {
        return tuuvm_array_create(context, 0);
    }
    else
    {
        // Unexpanded application node.
        tuuvm_tuple_t functionOrMacroExpression = tuuvm_arraySlice_at(unexpandedSExpressionNode->elements, 0);
        tuuvm_tuple_t argumentExpressions = tuuvm_arraySlice_fromOffset(context, unexpandedSExpressionNode->elements, 1);
        tuuvm_tuple_t unexpandedApplicationNode = tuuvm_astUnexpandedApplicationNode_create(context, unexpandedSExpressionNode->super.sourcePosition, functionOrMacroExpression, argumentExpressions);
        return tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, unexpandedApplicationNode, environment);
    }
}

TUUVM_API tuuvm_tuple_t tuuvm_interpreter_applyClosureASTFunction(tuuvm_context_t *context, tuuvm_tuple_t function, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    tuuvm_closureASTFunction_t *closureASTFunction = (tuuvm_closureASTFunction_t*)function;

    size_t expectedArgumentCount = tuuvm_arraySlice_getSize(closureASTFunction->argumentSymbols);
    if(argumentCount != expectedArgumentCount)
        tuuvm_error_argumentCountMismatch(expectedArgumentCount, argumentCount);

    tuuvm_tuple_t applicationEnvironment = tuuvm_environment_create(context, closureASTFunction->closureEnvironment);
    for(size_t i = 0; i < argumentCount; ++i)
        tuuvm_environment_setSymbolBinding(context, applicationEnvironment, tuuvm_arraySlice_at(closureASTFunction->argumentSymbols, i), arguments[i]);

    return tuuvm_interpreter_evaluateASTWithEnvironment(context, closureASTFunction->body, applicationEnvironment);
}

void tuuvm_astInterpreter_setupTypes(tuuvm_context_t *context)
{
    tuuvm_type_setAstNodeAnalysisFunction(context->roots.astSequenceNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astSequenceNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context->roots.astSequenceNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astSequenceNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context->roots.astSequenceNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astSequenceNode_primitiveAnalyzeAndEvaluate));

    tuuvm_type_setAstNodeAnalysisFunction(context->roots.astLiteralNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astLiteralNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context->roots.astLiteralNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astLiteralNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context->roots.astLiteralNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astLiteralNode_primitiveEvaluate));

    tuuvm_type_setAstNodeAnalysisFunction(context->roots.astIdentifierReferenceNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astIdentifierReferenceNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context->roots.astIdentifierReferenceNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astIdentifierReferenceNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context->roots.astIdentifierReferenceNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astIdentifierReferenceNode_primitiveEvaluate));

    tuuvm_type_setAstNodeAnalysisFunction(context->roots.astUnexpandedApplicationNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astUnexpandedApplicationNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context->roots.astUnexpandedApplicationNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astUnexpandedApplicationNode_primitiveAnalyzeAndEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context->roots.astUnexpandedApplicationNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astUnexpandedApplicationNode_primitiveAnalyzeAndEvaluate));

    tuuvm_type_setAstNodeAnalysisFunction(context->roots.astUnexpandedSExpressionNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astUnexpandedSExpressionNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context->roots.astUnexpandedSExpressionNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astUnexpandedSExpressionNode_primitiveAnalyzeAndEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context->roots.astUnexpandedSExpressionNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astUnexpandedSExpressionNode_primitiveAnalyzeAndEvaluate));

}
