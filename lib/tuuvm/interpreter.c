#include "tuuvm/interpreter.h"
#include "tuuvm/environment.h"
#include "tuuvm/array.h"
#include "tuuvm/arraySlice.h"
#include "tuuvm/arrayList.h"
#include "tuuvm/ast.h"
#include "tuuvm/assert.h"
#include "tuuvm/function.h"
#include "tuuvm/errors.h"
#include "tuuvm/macro.h"
#include "tuuvm/parser.h"
#include "tuuvm/string.h"
#include "tuuvm/sourceCode.h"
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

TUUVM_API tuuvm_tuple_t tuuvm_interpreter_analyzeAndEvaluateSourceCodeWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t sourceCode)
{
    return tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, tuuvm_parser_parseSourceCode(context, sourceCode), environment);
}

TUUVM_API tuuvm_tuple_t tuuvm_interpreter_analyzeAndEvaluateCStringWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t environment, const char *sourceCodeText, const char *sourceCodeName)
{
    return tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, tuuvm_parser_parseCString(context, sourceCodeText, sourceCodeName), environment);
}

TUUVM_API tuuvm_tuple_t tuuvm_interpreter_analyzeAndEvaluateStringWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t sourceCodeText, tuuvm_tuple_t sourceCodeName)
{
    return tuuvm_interpreter_analyzeAndEvaluateSourceCodeWithEnvironment(context, environment, tuuvm_sourceCode_create(context, sourceCodeText, sourceCodeName));
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

    sequenceNode = (tuuvm_astSequenceNode_t *)tuuvm_context_shallowCopy(context, (tuuvm_tuple_t)sequenceNode);
    
    tuuvm_tuple_t analyzedExpressions = tuuvm_arraySlice_createWithArrayOfSize(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        tuuvm_tuple_t expression = tuuvm_arraySlice_at(expressions, i);
        tuuvm_tuple_t analyzedExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, expression, environment);
        tuuvm_arraySlice_atPut(analyzedExpressions, i, analyzedExpression);
    }

    sequenceNode->expressions = analyzedExpressions;
    return (tuuvm_tuple_t)sequenceNode;
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

static tuuvm_tuple_t tuuvm_astLambdaNode_parseArgumentsNodes(tuuvm_context_t *context, tuuvm_tuple_t argumentsNode)
{
    tuuvm_tuple_t argumentList = tuuvm_arrayList_create(context);
    size_t argumentNodeCount = tuuvm_arraySlice_getSize(argumentsNode);
    for(size_t i = 0; i < argumentNodeCount; ++i)
    {
        tuuvm_tuple_t argumentNode = tuuvm_arraySlice_at(argumentsNode, i);
        if(!tuuvm_astNode_isIdentifierReferenceNode(context, argumentNode))
            tuuvm_error("Invalid argument definition node.");

        tuuvm_arrayList_add(context, argumentList, tuuvm_astIdentifierReferenceNode_getValue(argumentNode));
    }

    return tuuvm_arrayList_asArraySlice(context, argumentList);
}

static tuuvm_tuple_t tuuvm_astLambdaNode_primitiveMacro(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) tuuvm_error_argumentCountMismatch(3, argumentCount);

    tuuvm_tuple_t macroContext = arguments[0];
    tuuvm_tuple_t argumentsSExpressionNode = arguments[1];
    tuuvm_tuple_t bodyNodes = arguments[2];

    if(!tuuvm_astNode_isUnexpandedSExpressionNode(context, argumentsSExpressionNode))
        tuuvm_error("Expected a S-Expression with the arguments node.");

    tuuvm_tuple_t argumentsNode = tuuvm_astUnexpandedSExpressionNode_getElements(argumentsSExpressionNode);
    tuuvm_tuple_t sourcePosition = tuuvm_macroContext_getSourcePosition(macroContext);
    tuuvm_tuple_t argumentsArraySlice = tuuvm_astLambdaNode_parseArgumentsNodes(context, argumentsNode);
    tuuvm_tuple_t bodySequence = tuuvm_astSequenceNode_create(context, sourcePosition, bodyNodes);
    return tuuvm_astLambdaNode_create(context, sourcePosition, tuuvm_tuple_size_encode(context, TUUVM_FUNCTION_FLAGS_NONE), argumentsArraySlice, bodySequence);
}

static tuuvm_tuple_t tuuvm_astLambdaNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t node = arguments[0];
    tuuvm_tuple_t environment = arguments[1];

    tuuvm_astLambdaNode_t *lambdaNode = (tuuvm_astLambdaNode_t*)tuuvm_context_shallowCopy(context, node);

    tuuvm_tuple_t lambdaAnalysisEnvironment = tuuvm_environment_create(context, environment);
    lambdaNode->body = tuuvm_interpreter_analyzeASTWithEnvironment(context, lambdaNode->body, lambdaAnalysisEnvironment);
    return (tuuvm_tuple_t)lambdaNode;
}

static tuuvm_tuple_t tuuvm_astLambdaNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t node = arguments[0];
    tuuvm_tuple_t environment = arguments[1];

    tuuvm_astLambdaNode_t *lambdaNode = (tuuvm_astLambdaNode_t*)node;

    return tuuvm_function_createClosureAST(context, lambdaNode->super.sourcePosition, lambdaNode->flags, environment, lambdaNode->arguments, lambdaNode->body);
}

static tuuvm_tuple_t tuuvm_astLambdaNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t node = arguments[0];
    tuuvm_tuple_t environment = arguments[1];

    tuuvm_astLambdaNode_t *lambdaNode = (tuuvm_astLambdaNode_t*)node;

    tuuvm_tuple_t lambdaAnalysisEnvironment = tuuvm_environment_create(context, environment);
    tuuvm_tuple_t analyzedBody = tuuvm_interpreter_analyzeASTWithEnvironment(context, lambdaNode->body, lambdaAnalysisEnvironment);

    return tuuvm_function_createClosureAST(context, lambdaNode->super.sourcePosition, lambdaNode->flags, environment, lambdaNode->arguments, analyzedBody);
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

static tuuvm_tuple_t tuuvm_astLocalDefinitionNode_primitiveMacro(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) tuuvm_error_argumentCountMismatch(3, argumentCount);

    tuuvm_tuple_t macroContext = arguments[0];
    tuuvm_tuple_t nameOrLambdaSignature = arguments[1];
    tuuvm_tuple_t valueOrBodyNodes = arguments[2];

    tuuvm_tuple_t nameNode = TUUVM_NULL_TUPLE;
    tuuvm_tuple_t valueNode = TUUVM_NULL_TUPLE;
    tuuvm_tuple_t sourcePosition = tuuvm_macroContext_getSourcePosition(macroContext);

    if(tuuvm_astNode_isIdentifierReferenceNode(context, nameOrLambdaSignature))
    {
        if(tuuvm_arraySlice_getSize(valueOrBodyNodes) != 1)
            tuuvm_error("Expected a single value for a local define.");

        nameNode = nameOrLambdaSignature;
        valueNode = tuuvm_arraySlice_at(valueOrBodyNodes, 0);
    }
    else if(tuuvm_astNode_isUnexpandedSExpressionNode(context, nameOrLambdaSignature))
    {
        tuuvm_tuple_t lambdaSignatureElements = tuuvm_astUnexpandedSExpressionNode_getElements(nameOrLambdaSignature);
        if(tuuvm_arraySlice_getSize(lambdaSignatureElements) < 1)
            tuuvm_error("Expected function definition requires a name.");

        nameNode = tuuvm_arraySlice_at(lambdaSignatureElements, 0);
        if(!tuuvm_astNode_isIdentifierReferenceNode(context, nameNode))
            tuuvm_error("Expected an identifier reference node for the name.");

        tuuvm_tuple_t argumentsNode = tuuvm_arraySlice_fromOffset(context, lambdaSignatureElements, 1);
        tuuvm_tuple_t arguments = tuuvm_astLambdaNode_parseArgumentsNodes(context, argumentsNode);
        tuuvm_tuple_t bodySequence = tuuvm_astSequenceNode_create(context, sourcePosition, valueOrBodyNodes);
        valueNode = tuuvm_astLambdaNode_create(context, sourcePosition, tuuvm_tuple_size_encode(context, TUUVM_FUNCTION_FLAGS_NONE), arguments, bodySequence);
    }
    else
    {
        tuuvm_error("Invalid usage of (define)");
    }

    tuuvm_tuple_t nameExpression = tuuvm_astLiteralNode_create(context, tuuvm_astNode_getSourcePosition(nameNode), tuuvm_astIdentifierReferenceNode_getValue(nameNode));
    return tuuvm_astLocalDefinitionNode_create(context, sourcePosition, nameExpression, valueNode);
}

static tuuvm_tuple_t tuuvm_astLocalDefinitionNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t node = arguments[0];
    tuuvm_tuple_t environment = arguments[1];

    tuuvm_astLocalDefinitionNode_t *localDefinitionNode = (tuuvm_astLocalDefinitionNode_t*)tuuvm_context_shallowCopy(context, node);
    localDefinitionNode->nameExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, localDefinitionNode->nameExpression, environment);
    localDefinitionNode->valueExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, localDefinitionNode->valueExpression, environment);
    return (tuuvm_tuple_t)localDefinitionNode;
}

static tuuvm_tuple_t tuuvm_astLocalDefinitionNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t node = arguments[0];
    tuuvm_tuple_t environment = arguments[1];

    tuuvm_astLocalDefinitionNode_t *localDefinitionNode = (tuuvm_astLocalDefinitionNode_t*)node;
    tuuvm_tuple_t name = tuuvm_interpreter_evaluateASTWithEnvironment(context, localDefinitionNode->nameExpression, environment);
    tuuvm_tuple_t value = tuuvm_interpreter_evaluateASTWithEnvironment(context, localDefinitionNode->valueExpression, environment);
    tuuvm_environment_setSymbolBinding(context, environment, name, value);
    return value;
}

static tuuvm_tuple_t tuuvm_astLocalDefinitionNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t node = arguments[0];
    tuuvm_tuple_t environment = arguments[1];

    tuuvm_astLocalDefinitionNode_t *localDefinitionNode = (tuuvm_astLocalDefinitionNode_t*)node;
    tuuvm_tuple_t name = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, localDefinitionNode->nameExpression, environment);
    tuuvm_tuple_t value = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, localDefinitionNode->valueExpression, environment);
    tuuvm_environment_setSymbolBinding(context, environment, name, value);
    return value;
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
        return tuuvm_astLiteralNode_create(context, referenceNode->super.sourcePosition, binding);

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

void tuuvm_astInterpreter_setupASTInterpreter(tuuvm_context_t *context)
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

    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "lambda"), tuuvm_function_createPrimitive(context, 3, TUUVM_FUNCTION_FLAGS_MACRO | TUUVM_FUNCTION_FLAGS_VARIADIC, NULL, tuuvm_astLambdaNode_primitiveMacro));
    tuuvm_type_setAstNodeAnalysisFunction(context->roots.astLambdaNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astLambdaNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context->roots.astLambdaNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astLambdaNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context->roots.astLambdaNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astLambdaNode_primitiveAnalyzeAndEvaluate));

    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "define"), tuuvm_function_createPrimitive(context, 3, TUUVM_FUNCTION_FLAGS_MACRO | TUUVM_FUNCTION_FLAGS_VARIADIC, NULL, tuuvm_astLocalDefinitionNode_primitiveMacro));
    tuuvm_type_setAstNodeAnalysisFunction(context->roots.astLocalDefinitionNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astLocalDefinitionNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context->roots.astLocalDefinitionNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astLocalDefinitionNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context->roots.astLocalDefinitionNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astLocalDefinitionNode_primitiveAnalyzeAndEvaluate));
}
