#include "tuuvm/interpreter.h"
#include "tuuvm/environment.h"
#include "tuuvm/array.h"
#include "tuuvm/arraySlice.h"
#include "tuuvm/arrayList.h"
#include "tuuvm/ast.h"
#include "tuuvm/assert.h"
#include "tuuvm/function.h"
#include "tuuvm/gc.h"
#include "tuuvm/errors.h"
#include "tuuvm/macro.h"
#include "tuuvm/parser.h"
#include "tuuvm/string.h"
#include "tuuvm/sourceCode.h"
#include "tuuvm/stackFrame.h"
#include "tuuvm/type.h"
#include "internal/context.h"
#include <stdio.h>

#define tuuvm_gc_safepoint(x) false

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
    struct {
        tuuvm_tuple_t environment;
        tuuvm_tuple_t sourceCode;
    } gcFrame = {
        .environment = environment,
        .sourceCode = sourceCode
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    tuuvm_tuple_t result = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, tuuvm_parser_parseSourceCode(context, gcFrame.sourceCode), gcFrame.environment);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return result;
}

TUUVM_API tuuvm_tuple_t tuuvm_interpreter_analyzeAndEvaluateCStringWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t environment, const char *sourceCodeText, const char *sourceCodeName)
{
    struct {
        tuuvm_tuple_t environment;
    } gcFrame = {
        .environment = environment,
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    tuuvm_tuple_t result = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, tuuvm_parser_parseCString(context, sourceCodeText, sourceCodeName), environment);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return result;
}

TUUVM_API tuuvm_tuple_t tuuvm_interpreter_analyzeAndEvaluateStringWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t sourceCodeText, tuuvm_tuple_t sourceCodeName)
{
    struct {
        tuuvm_tuple_t environment;
    } gcFrame = {
        .environment = environment,
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    tuuvm_tuple_t result = tuuvm_interpreter_analyzeAndEvaluateSourceCodeWithEnvironment(context, environment, tuuvm_sourceCode_create(context, sourceCodeText, sourceCodeName));
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return result;
}

static tuuvm_tuple_t tuuvm_astSequenceNode_primitiveMacro(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *macroContext = &arguments[0];
    tuuvm_tuple_t *bodyNodes = &arguments[1];

    tuuvm_tuple_t sourcePosition = tuuvm_macroContext_getSourcePosition(*macroContext);
    return tuuvm_astSequenceNode_create(context, sourcePosition, *bodyNodes);
}

static tuuvm_tuple_t tuuvm_astSequenceNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astSequenceNode_t **sequenceNode = (tuuvm_astSequenceNode_t**)node;

    struct {
        tuuvm_astSequenceNode_t *analyzedSequenceNode;
        tuuvm_tuple_t expressions;
        tuuvm_tuple_t analyzedExpressions;
        tuuvm_tuple_t expression;
        tuuvm_tuple_t analyzedExpression;
    } gcFrame = {};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.expressions = (*sequenceNode)->expressions;
    size_t expressionCount = tuuvm_arraySlice_getSize(gcFrame.expressions);
    if(expressionCount == 0)
    {
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return *node;
    }

    gcFrame.analyzedSequenceNode = (tuuvm_astSequenceNode_t *)tuuvm_context_shallowCopy(context, *node);
    
    gcFrame.analyzedExpressions = tuuvm_arraySlice_createWithArrayOfSize(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expression = tuuvm_arraySlice_at(gcFrame.expressions, i);
        gcFrame.analyzedExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.expression, *environment);
        tuuvm_arraySlice_atPut(gcFrame.analyzedExpressions, i, gcFrame.analyzedExpression);
    }

    gcFrame.analyzedSequenceNode->expressions = gcFrame.analyzedExpressions;
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.analyzedSequenceNode;
}

static tuuvm_tuple_t tuuvm_astSequenceNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_tuple_t result = TUUVM_VOID_TUPLE;

    tuuvm_astSequenceNode_t **sequenceNode = (tuuvm_astSequenceNode_t**)node;

    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*sequenceNode)->super.sourcePosition);

    size_t expressionCount = tuuvm_arraySlice_getSize((*sequenceNode)->expressions);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        tuuvm_tuple_t expression = tuuvm_arraySlice_at((*sequenceNode)->expressions, i);
        result = tuuvm_interpreter_evaluateASTWithEnvironment(context, expression, *environment);
    }

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);

    return result;
}

static tuuvm_tuple_t tuuvm_astSequenceNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_tuple_t result = TUUVM_VOID_TUPLE;

    tuuvm_astSequenceNode_t **sequenceNode = (tuuvm_astSequenceNode_t**)node;
    size_t expressionCount = tuuvm_arraySlice_getSize((*sequenceNode)->expressions);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        tuuvm_tuple_t expression = tuuvm_arraySlice_at((*sequenceNode)->expressions, i);
        result = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, expression, *environment);
    }

    return result;
}

static tuuvm_tuple_t tuuvm_astLambdaNode_parseArgumentsNodes(tuuvm_context_t *context, tuuvm_tuple_t unsafeArgumentsNode)
{
    struct {
        tuuvm_tuple_t argumentsNode;
        tuuvm_tuple_t argumentList;
        tuuvm_tuple_t argumentNode;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.argumentsNode = unsafeArgumentsNode;
    gcFrame.argumentList = tuuvm_arrayList_create(context);
    size_t argumentNodeCount = tuuvm_arraySlice_getSize(gcFrame.argumentsNode);
    for(size_t i = 0; i < argumentNodeCount; ++i)
    {
        gcFrame.argumentNode = tuuvm_arraySlice_at(gcFrame.argumentsNode, i);
        if(!tuuvm_astNode_isIdentifierReferenceNode(context, gcFrame.argumentNode))
            tuuvm_error("Invalid argument definition node.");

        tuuvm_arrayList_add(context, gcFrame.argumentList, tuuvm_astIdentifierReferenceNode_getValue(gcFrame.argumentNode));
    }

    tuuvm_tuple_t result = tuuvm_arrayList_asArraySlice(context, gcFrame.argumentList);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return result;
}

static tuuvm_tuple_t tuuvm_astLambdaNode_primitiveMacro(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) tuuvm_error_argumentCountMismatch(3, argumentCount);

    tuuvm_tuple_t *macroContext = &arguments[0];
    tuuvm_tuple_t *argumentsSExpressionNode = &arguments[1];
    tuuvm_tuple_t *bodyNodes = &arguments[2];

    struct {
        tuuvm_tuple_t argumentsNode;
        tuuvm_tuple_t sourcePosition;
        tuuvm_tuple_t argumentsArraySlice;
        tuuvm_tuple_t bodySequence;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    if(!tuuvm_astNode_isUnexpandedSExpressionNode(context, *argumentsSExpressionNode))
        tuuvm_error("Expected a S-Expression with the arguments node.");

    gcFrame.argumentsNode = tuuvm_astUnexpandedSExpressionNode_getElements(*argumentsSExpressionNode);
    gcFrame.sourcePosition = tuuvm_macroContext_getSourcePosition(*macroContext);
    gcFrame.argumentsArraySlice = tuuvm_astLambdaNode_parseArgumentsNodes(context, gcFrame.argumentsNode);
    gcFrame.bodySequence = tuuvm_astSequenceNode_create(context, gcFrame.sourcePosition, *bodyNodes);
    tuuvm_tuple_t result = tuuvm_astLambdaNode_create(context, gcFrame.sourcePosition, tuuvm_tuple_size_encode(context, TUUVM_FUNCTION_FLAGS_NONE), gcFrame.argumentsArraySlice, gcFrame.bodySequence);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return result;
}

static tuuvm_tuple_t tuuvm_astLambdaNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_astLambdaNode_t *lambdaNode;
        tuuvm_tuple_t lambdaAnalysisEnvironment;
        tuuvm_tuple_t argumentsArraySlice;
        tuuvm_tuple_t bodySequence;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.lambdaNode = (tuuvm_astLambdaNode_t*)tuuvm_context_shallowCopy(context, *node);

    gcFrame.lambdaAnalysisEnvironment = tuuvm_environment_create(context, *environment);
    gcFrame.lambdaNode->body = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.lambdaNode->body, gcFrame.lambdaAnalysisEnvironment);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.lambdaNode;
}

static tuuvm_tuple_t tuuvm_astLambdaNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astLambdaNode_t **lambdaNode = (tuuvm_astLambdaNode_t**)node;

    return tuuvm_function_createClosureAST(context, (*lambdaNode)->super.sourcePosition, (*lambdaNode)->flags, *environment, (*lambdaNode)->arguments, (*lambdaNode)->body);
}

static tuuvm_tuple_t tuuvm_astLambdaNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_tuple_t lambdaAnalysisEnvironment;
        tuuvm_tuple_t analyzedBody;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    tuuvm_astLambdaNode_t **lambdaNode = (tuuvm_astLambdaNode_t**)node;

    gcFrame.lambdaAnalysisEnvironment = tuuvm_environment_create(context, *environment);
    gcFrame.analyzedBody = tuuvm_interpreter_analyzeASTWithEnvironment(context, (*lambdaNode)->body, gcFrame.lambdaAnalysisEnvironment);

    tuuvm_tuple_t result = tuuvm_function_createClosureAST(context, (*lambdaNode)->super.sourcePosition, (*lambdaNode)->flags, *environment, (*lambdaNode)->arguments, gcFrame.analyzedBody);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return result;
}

static tuuvm_tuple_t tuuvm_astLiteralNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];

    return *node;
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

    tuuvm_tuple_t *macroContext = &arguments[0];
    tuuvm_tuple_t *nameOrLambdaSignature = &arguments[1];
    tuuvm_tuple_t *valueOrBodyNodes = &arguments[2];

    struct {
        tuuvm_tuple_t nameNode;
        tuuvm_tuple_t valueNode;
        tuuvm_tuple_t sourcePosition;
        tuuvm_tuple_t lambdaSignatureElements;
        tuuvm_tuple_t argumentsNode;
        tuuvm_tuple_t arguments;
        tuuvm_tuple_t bodySequence;
        tuuvm_tuple_t nameExpression;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sourcePosition = tuuvm_macroContext_getSourcePosition(*macroContext);

    if(tuuvm_astNode_isIdentifierReferenceNode(context, *nameOrLambdaSignature))
    {
        if(tuuvm_arraySlice_getSize(*valueOrBodyNodes) != 1)
            tuuvm_error("Expected a single value for a local define.");

        gcFrame.nameNode = *nameOrLambdaSignature;
        gcFrame.valueNode = tuuvm_arraySlice_at(*valueOrBodyNodes, 0);
    }
    else if(tuuvm_astNode_isUnexpandedSExpressionNode(context, *nameOrLambdaSignature))
    {
        gcFrame.lambdaSignatureElements = tuuvm_astUnexpandedSExpressionNode_getElements(*nameOrLambdaSignature);
        if(tuuvm_arraySlice_getSize(gcFrame.lambdaSignatureElements) < 1)
            tuuvm_error("Expected function definition requires a name.");

        gcFrame.nameNode = tuuvm_arraySlice_at(gcFrame.lambdaSignatureElements, 0);
        if(!tuuvm_astNode_isIdentifierReferenceNode(context, gcFrame.nameNode))
            tuuvm_error("Expected an identifier reference node for the name.");

        gcFrame.argumentsNode = tuuvm_arraySlice_fromOffset(context, gcFrame.lambdaSignatureElements, 1);
        gcFrame.arguments = tuuvm_astLambdaNode_parseArgumentsNodes(context, gcFrame.argumentsNode);
        gcFrame.bodySequence = tuuvm_astSequenceNode_create(context, gcFrame.sourcePosition, *valueOrBodyNodes);
        gcFrame.valueNode = tuuvm_astLambdaNode_create(context, gcFrame.sourcePosition, tuuvm_tuple_size_encode(context, TUUVM_FUNCTION_FLAGS_NONE), gcFrame.arguments, gcFrame.bodySequence);
    }
    else
    {
        tuuvm_error("Invalid usage of (define)");
    }

    gcFrame.nameExpression = tuuvm_astLiteralNode_create(context, tuuvm_astNode_getSourcePosition(gcFrame.nameNode), tuuvm_astIdentifierReferenceNode_getValue(gcFrame.nameNode));
    tuuvm_tuple_t result = tuuvm_astLocalDefinitionNode_create(context, gcFrame.sourcePosition, gcFrame.nameExpression, gcFrame.valueNode);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return result;
}

static tuuvm_tuple_t tuuvm_astLocalDefinitionNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_astLocalDefinitionNode_t *localDefinitionNode;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.localDefinitionNode = (tuuvm_astLocalDefinitionNode_t*)tuuvm_context_shallowCopy(context, *node);
    gcFrame.localDefinitionNode->nameExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.localDefinitionNode->nameExpression, *environment);
    gcFrame.localDefinitionNode->valueExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.localDefinitionNode->valueExpression, *environment);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.localDefinitionNode;
}

static tuuvm_tuple_t tuuvm_astLocalDefinitionNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_tuple_t name;
        tuuvm_tuple_t value;
    } gcFrame = {};

    tuuvm_astLocalDefinitionNode_t **localDefinitionNode = (tuuvm_astLocalDefinitionNode_t**)node;

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*localDefinitionNode)->super.sourcePosition);

    gcFrame.name = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*localDefinitionNode)->nameExpression, *environment);
    gcFrame.value = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*localDefinitionNode)->valueExpression, *environment);
    tuuvm_environment_setNewSymbolBinding(context, *environment, gcFrame.name, gcFrame.value);
    //TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.value;
}

static tuuvm_tuple_t tuuvm_astLocalDefinitionNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_tuple_t name;
        tuuvm_tuple_t value;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    tuuvm_astLocalDefinitionNode_t **localDefinitionNode = (tuuvm_astLocalDefinitionNode_t**)node;
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*localDefinitionNode)->super.sourcePosition);

    gcFrame.name = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*localDefinitionNode)->nameExpression, *environment);
    gcFrame.value = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*localDefinitionNode)->valueExpression, *environment);
    tuuvm_environment_setNewSymbolBinding(context, *environment, gcFrame.name, gcFrame.value);
    //TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.value;
}

static tuuvm_tuple_t tuuvm_astIdentifierReferenceNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astIdentifierReferenceNode_t **referenceNode = (tuuvm_astIdentifierReferenceNode_t**)node;
    tuuvm_tuple_t binding;

    // Attempt to replace the symbol with its binding, if it exists.
    if(tuuvm_environment_lookSymbolRecursively(context, *environment, (*referenceNode)->value, &binding))
        return tuuvm_astLiteralNode_create(context, (*referenceNode)->super.sourcePosition, binding);

    return *node;
}

static tuuvm_tuple_t tuuvm_astIdentifierReferenceNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astIdentifierReferenceNode_t **referenceNode = (tuuvm_astIdentifierReferenceNode_t**)node;
    tuuvm_tuple_t binding;

    if(tuuvm_environment_lookSymbolRecursively(context, *environment, (*referenceNode)->value, &binding))
        return binding;

    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*referenceNode)->super.sourcePosition);

    tuuvm_error("Failed to find symbol binding");
    return TUUVM_NULL_TUPLE;
}

static tuuvm_tuple_t tuuvm_astIfNode_primitiveMacro(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 4) tuuvm_error_argumentCountMismatch(4, argumentCount);

    tuuvm_tuple_t *macroContext = &arguments[0];
    tuuvm_tuple_t *conditionNode = &arguments[1];
    tuuvm_tuple_t *trueExpressionNode = &arguments[2];
    tuuvm_tuple_t *falseExpressionNode = &arguments[3];

    tuuvm_tuple_t sourcePosition = tuuvm_macroContext_getSourcePosition(*macroContext);

    return tuuvm_astIfNode_create(context, sourcePosition, *conditionNode, *trueExpressionNode, *falseExpressionNode);
}

static tuuvm_tuple_t tuuvm_astIfNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_astIfNode_t *ifNode;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.ifNode = (tuuvm_astIfNode_t*)tuuvm_context_shallowCopy(context, *node);
    gcFrame.ifNode->conditionExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.ifNode->conditionExpression, *environment);
    gcFrame.ifNode->trueExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.ifNode->trueExpression, *environment);
    gcFrame.ifNode->falseExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.ifNode->falseExpression, *environment);
    return (tuuvm_tuple_t)gcFrame.ifNode;
}

static tuuvm_tuple_t tuuvm_astIfNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astIfNode_t **ifNode = (tuuvm_astIfNode_t**)node;
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*ifNode)->super.sourcePosition);
    tuuvm_tuple_t condition = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*ifNode)->conditionExpression, *environment);
    if(condition == TUUVM_TRUE_TUPLE)
    {
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        return tuuvm_interpreter_evaluateASTWithEnvironment(context, (*ifNode)->trueExpression, *environment);
    }
    else
    {
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        return tuuvm_interpreter_evaluateASTWithEnvironment(context, (*ifNode)->falseExpression, *environment);
    }
}

static tuuvm_tuple_t tuuvm_astIfNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astIfNode_t **ifNode = (tuuvm_astIfNode_t**)node;
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*ifNode)->super.sourcePosition);
    tuuvm_tuple_t condition = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*ifNode)->conditionExpression, *environment);
    if(condition == TUUVM_TRUE_TUPLE)
    {
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        return tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*ifNode)->trueExpression, *environment);
    }
    else
    {
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        return tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*ifNode)->falseExpression, *environment);
    }
}

static tuuvm_tuple_t tuuvm_astUnexpandedApplicationNode_expandNodeWithMacro(tuuvm_context_t *context, tuuvm_tuple_t *node, tuuvm_tuple_t *macro)
{
    tuuvm_astUnexpandedApplicationNode_t **unexpandedNode = (tuuvm_astUnexpandedApplicationNode_t**)node;

    size_t applicationArgumentCount = tuuvm_arraySlice_getSize((*unexpandedNode)->arguments);

    tuuvm_functionCallFrameStack_t callFrameStack = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    tuuvm_functionCallFrameStack_begin(context, &callFrameStack, *macro, 1 + applicationArgumentCount);
    tuuvm_functionCallFrameStack_push(&callFrameStack, (*unexpandedNode)->super.sourcePosition);
    for(size_t i = 0; i < applicationArgumentCount; ++i)
        tuuvm_functionCallFrameStack_push(&callFrameStack, tuuvm_arraySlice_at((*unexpandedNode)->arguments, i));

    tuuvm_tuple_t result = tuuvm_functionCallFrameStack_finish(context, &callFrameStack);
    TUUVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    return result;
}

static tuuvm_tuple_t tuuvm_astUnexpandedApplicationNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astUnexpandedApplicationNode_t **unexpandedNode = (tuuvm_astUnexpandedApplicationNode_t**)node;

    struct {
        tuuvm_tuple_t macro;
        tuuvm_tuple_t expansionResult;
        tuuvm_tuple_t applicationNode;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*unexpandedNode)->super.sourcePosition);

    tuuvm_tuple_t functionOrMacroExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, (*unexpandedNode)->functionOrMacroExpression, *environment);

    // Is this a macro?
    bool isMacro = tuuvm_astNode_isMacroExpression(context, functionOrMacroExpression);
    if(isMacro)
    {
        gcFrame.macro = tuuvm_interpreter_evaluateASTWithEnvironment(context, functionOrMacroExpression, *environment);
        gcFrame.expansionResult = tuuvm_astUnexpandedApplicationNode_expandNodeWithMacro(context, node, &gcFrame.macro);
        //TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.expansionResult, *environment);
    }

    // Convert into application node and then analyze it.
    gcFrame.applicationNode = tuuvm_astFunctionApplicationNode_create(context, (*unexpandedNode)->super.sourcePosition, functionOrMacroExpression, (*unexpandedNode)->arguments);
    //TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.applicationNode, *environment);
}

static tuuvm_tuple_t tuuvm_astUnexpandedApplicationNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astUnexpandedApplicationNode_t **unexpandedNode = (tuuvm_astUnexpandedApplicationNode_t**)node;

    struct {
        tuuvm_tuple_t functionOrMacro;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.functionOrMacro = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*unexpandedNode)->functionOrMacroExpression, *environment);
    bool isMacro = tuuvm_function_isMacro(context, gcFrame.functionOrMacro);

    if(isMacro)
    {
        tuuvm_tuple_t expansionResult = tuuvm_astUnexpandedApplicationNode_expandNodeWithMacro(context, node, &gcFrame.functionOrMacro);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, expansionResult, *environment);
    }

    tuuvm_functionCallFrameStack_t callFrameStack = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    size_t applicationArgumentCount = tuuvm_arraySlice_getSize((*unexpandedNode)->arguments);
    tuuvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.functionOrMacro, applicationArgumentCount);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        tuuvm_tuple_t argumentNode = tuuvm_arraySlice_at((*unexpandedNode)->arguments, i);
        tuuvm_functionCallFrameStack_push(&callFrameStack, tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, argumentNode, *environment));
    }

    tuuvm_tuple_t result = tuuvm_functionCallFrameStack_finish(context, &callFrameStack);
    //TUUVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return result;
}

static tuuvm_tuple_t tuuvm_astUnexpandedSExpressionNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astUnexpandedSExpressionNode_t **unexpandedSExpressionNode = (tuuvm_astUnexpandedSExpressionNode_t**)node;

    struct {
        tuuvm_tuple_t array;
        tuuvm_tuple_t literalNode;
        tuuvm_tuple_t functionOrMacroExpression;
        tuuvm_tuple_t argumentExpressions;
        tuuvm_tuple_t unexpandedApplicationNode;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    size_t elementCount = tuuvm_arraySlice_getSize((*unexpandedSExpressionNode)->elements);
    if(elementCount == 0)
    {
        // Empty array.
        gcFrame.array = tuuvm_array_create(context, 0);
        gcFrame.literalNode = tuuvm_astLiteralNode_create(context, (*unexpandedSExpressionNode)->super.sourcePosition, gcFrame.array);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.literalNode, *environment);
    }
    else
    {
        // Unexpanded application node.
        gcFrame.functionOrMacroExpression = tuuvm_arraySlice_at((*unexpandedSExpressionNode)->elements, 0);
        gcFrame.argumentExpressions = tuuvm_arraySlice_fromOffset(context, (*unexpandedSExpressionNode)->elements, 1);
        gcFrame.unexpandedApplicationNode = tuuvm_astUnexpandedApplicationNode_create(context, (*unexpandedSExpressionNode)->super.sourcePosition, gcFrame.functionOrMacroExpression, gcFrame.argumentExpressions);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.unexpandedApplicationNode, *environment);
    }
}

static tuuvm_tuple_t tuuvm_astUnexpandedSExpressionNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astUnexpandedSExpressionNode_t **unexpandedSExpressionNode = (tuuvm_astUnexpandedSExpressionNode_t**)node;

    struct {
        tuuvm_tuple_t functionOrMacroExpression;
        tuuvm_tuple_t argumentExpressions;
        tuuvm_tuple_t unexpandedApplicationNode;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    size_t elementCount = tuuvm_arraySlice_getSize((*unexpandedSExpressionNode)->elements);
    if(elementCount == 0)
    {
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return tuuvm_array_create(context, 0);
    }
    else
    {
        // Unexpanded application node.
        gcFrame.functionOrMacroExpression = tuuvm_arraySlice_at((*unexpandedSExpressionNode)->elements, 0);
        gcFrame.argumentExpressions = tuuvm_arraySlice_fromOffset(context, (*unexpandedSExpressionNode)->elements, 1);
        gcFrame.unexpandedApplicationNode = tuuvm_astUnexpandedApplicationNode_create(context, (*unexpandedSExpressionNode)->super.sourcePosition, gcFrame.functionOrMacroExpression, gcFrame.argumentExpressions);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.unexpandedApplicationNode, *environment);
    }
}

static tuuvm_tuple_t tuuvm_astFunctionApplicationNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_astFunctionApplicationNode_t *applicationNode;
        tuuvm_tuple_t analyzedArguments;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.applicationNode = (tuuvm_astFunctionApplicationNode_t*)tuuvm_context_shallowCopy(context, *node);
    gcFrame.applicationNode->functionExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.applicationNode->functionExpression, *environment);

    size_t applicationArgumentCount = tuuvm_arraySlice_getSize(gcFrame.applicationNode->arguments);
    gcFrame.analyzedArguments = tuuvm_array_create(context, applicationArgumentCount);
    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        tuuvm_tuple_t argumentNode = tuuvm_arraySlice_at(gcFrame.applicationNode->arguments, i);
        tuuvm_array_atPut(gcFrame.analyzedArguments, i, tuuvm_interpreter_analyzeASTWithEnvironment(context, argumentNode, *environment));
    }

    gcFrame.applicationNode->arguments = tuuvm_array_asArraySlice(context, gcFrame.analyzedArguments);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.applicationNode;
}

static tuuvm_tuple_t tuuvm_astFunctionApplicationNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astFunctionApplicationNode_t **applicationNode = (tuuvm_astFunctionApplicationNode_t**)node;

    tuuvm_functionCallFrameStack_t callFrameStack = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*applicationNode)->super.sourcePosition);

    size_t applicationArgumentCount = tuuvm_arraySlice_getSize((*applicationNode)->arguments);
    tuuvm_functionCallFrameStack_begin(context, &callFrameStack, tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*applicationNode)->functionExpression, *environment), applicationArgumentCount);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        tuuvm_tuple_t argumentNode = tuuvm_arraySlice_at((*applicationNode)->arguments, i);
        tuuvm_functionCallFrameStack_push(&callFrameStack, tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, argumentNode, *environment));
    }

    tuuvm_tuple_t result = tuuvm_functionCallFrameStack_finish(context, &callFrameStack);
    //TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    return result;
}

static tuuvm_tuple_t tuuvm_astFunctionApplicationNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astFunctionApplicationNode_t **applicationNode = (tuuvm_astFunctionApplicationNode_t**)node;

    tuuvm_functionCallFrameStack_t callFrameStack = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*applicationNode)->super.sourcePosition);

    size_t applicationArgumentCount = tuuvm_arraySlice_getSize((*applicationNode)->arguments);
    tuuvm_functionCallFrameStack_begin(context, &callFrameStack, tuuvm_interpreter_evaluateASTWithEnvironment(context, (*applicationNode)->functionExpression, *environment), applicationArgumentCount);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        tuuvm_tuple_t argumentNode = tuuvm_arraySlice_at((*applicationNode)->arguments, i);
        tuuvm_functionCallFrameStack_push(&callFrameStack, tuuvm_interpreter_evaluateASTWithEnvironment(context, argumentNode, *environment));
    }

    tuuvm_tuple_t result = tuuvm_functionCallFrameStack_finish(context, &callFrameStack);
    //TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    return result;
}

static tuuvm_tuple_t tuuvm_astDoWhileContinueWithNode_primitiveMacro(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 4) tuuvm_error_argumentCountMismatch(4, argumentCount);

    tuuvm_tuple_t *macroContext = &arguments[0];
    tuuvm_tuple_t *bodyExpressionNode = &arguments[1];
    tuuvm_tuple_t *conditionNode = &arguments[2];
    tuuvm_tuple_t *continueExpressionNode = &arguments[3];

    tuuvm_tuple_t sourcePosition = tuuvm_macroContext_getSourcePosition(*macroContext);

    return tuuvm_astDoWhileContinueWithNode_create(context, sourcePosition, *bodyExpressionNode, *conditionNode, *continueExpressionNode);
}

static tuuvm_tuple_t tuuvm_astDoWhileContinueWithNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_astDoWhileContinueWithNode_t *doWhileNode;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.doWhileNode = (tuuvm_astDoWhileContinueWithNode_t*)tuuvm_context_shallowCopy(context, *node);
    gcFrame.doWhileNode->bodyExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.doWhileNode->bodyExpression, *environment);
    gcFrame.doWhileNode->conditionExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.doWhileNode->conditionExpression, *environment);
    gcFrame.doWhileNode->continueExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.doWhileNode->continueExpression, *environment);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.doWhileNode;
}

static tuuvm_tuple_t tuuvm_astDoWhileContinueWithNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astDoWhileContinueWithNode_t **doWhileNode = (tuuvm_astDoWhileContinueWithNode_t**)node;
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*doWhileNode)->super.sourcePosition);
    bool shouldContinue = false;
    do
    {
        tuuvm_interpreter_evaluateASTWithEnvironment(context, (*doWhileNode)->bodyExpression, *environment);
        shouldContinue = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*doWhileNode)->conditionExpression, *environment);
        if(shouldContinue)
            tuuvm_interpreter_evaluateASTWithEnvironment(context, (*doWhileNode)->continueExpression, *environment);
        tuuvm_gc_safepoint(context);
    } while (shouldContinue);
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);

    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_astDoWhileContinueWithNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    return tuuvm_interpreter_evaluateASTWithEnvironment(context, tuuvm_interpreter_analyzeASTWithEnvironment(context, *node, *environment), *environment);
}

static tuuvm_tuple_t tuuvm_astWhileContinueWithNode_primitiveMacro(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 4) tuuvm_error_argumentCountMismatch(4, argumentCount);

    tuuvm_tuple_t *macroContext = &arguments[0];
    tuuvm_tuple_t *conditionNode = &arguments[1];
    tuuvm_tuple_t *bodyExpressionNode = &arguments[2];
    tuuvm_tuple_t *continueExpressionNode = &arguments[3];

    tuuvm_tuple_t sourcePosition = tuuvm_macroContext_getSourcePosition(*macroContext);

    return tuuvm_astWhileContinueWithNode_create(context, sourcePosition, *conditionNode, *bodyExpressionNode, *continueExpressionNode);
}

static tuuvm_tuple_t tuuvm_astWhileContinueWithNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_astWhileContinueWithNode_t *whileNode;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.whileNode = (tuuvm_astWhileContinueWithNode_t*)tuuvm_context_shallowCopy(context, *node);
    gcFrame.whileNode->conditionExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.whileNode->conditionExpression, *environment);
    gcFrame.whileNode->bodyExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.whileNode->bodyExpression, *environment);
    gcFrame.whileNode->continueExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.whileNode->continueExpression, *environment);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.whileNode;
}

static tuuvm_tuple_t tuuvm_astWhileContinueWithNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astWhileContinueWithNode_t **whileNode = (tuuvm_astWhileContinueWithNode_t**)node;
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*whileNode)->super.sourcePosition);
    for(; tuuvm_interpreter_evaluateASTWithEnvironment(context, (*whileNode)->conditionExpression, *environment) == TUUVM_TRUE_TUPLE;
        tuuvm_interpreter_evaluateASTWithEnvironment(context, (*whileNode)->continueExpression, *environment), tuuvm_gc_safepoint(context))
    {
        tuuvm_interpreter_evaluateASTWithEnvironment(context, (*whileNode)->bodyExpression, *environment);
    }
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);

    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_astWhileContinueWithNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    return tuuvm_interpreter_evaluateASTWithEnvironment(context, tuuvm_interpreter_analyzeASTWithEnvironment(context, *node, *environment), *environment);
}

TUUVM_API tuuvm_tuple_t tuuvm_interpreter_applyClosureASTFunction(tuuvm_context_t *context, tuuvm_tuple_t function, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    tuuvm_stackFrameFunctionActivationRecord_t functionActivationRecord = {
        .type = TUUVM_STACK_FRAME_RECORD_TYPE_FUNCTION_ACTIVATION,
        .function = function,
    };
    tuuvm_stackFrame_pushRecord((tuuvm_stackFrameRecord_t*)&functionActivationRecord);  
    tuuvm_closureASTFunction_t **closureASTFunction = (tuuvm_closureASTFunction_t**)&functionActivationRecord.function;
    
    size_t expectedArgumentCount = tuuvm_arraySlice_getSize((*closureASTFunction)->argumentSymbols);
    if(argumentCount != expectedArgumentCount)
        tuuvm_error_argumentCountMismatch(expectedArgumentCount, argumentCount);

    functionActivationRecord.applicationEnvironment = tuuvm_environment_create(context, (*closureASTFunction)->closureEnvironment);
    for(size_t i = 0; i < argumentCount; ++i)
        tuuvm_environment_setNewSymbolBinding(context, functionActivationRecord.applicationEnvironment, tuuvm_arraySlice_at((*closureASTFunction)->argumentSymbols, i), arguments[i]);

    tuuvm_gc_safepoint(context);
    tuuvm_tuple_t result = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*closureASTFunction)->body, functionActivationRecord.applicationEnvironment);
    tuuvm_stackFrame_popRecord((tuuvm_stackFrameRecord_t*)&functionActivationRecord);  
    return result;
}

void tuuvm_astInterpreter_setupASTInterpreter(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "begin", 2, TUUVM_FUNCTION_FLAGS_MACRO | TUUVM_FUNCTION_FLAGS_VARIADIC, NULL, tuuvm_astSequenceNode_primitiveMacro);
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

    tuuvm_type_setAstNodeAnalysisFunction(context->roots.astFunctionApplicationNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astFunctionApplicationNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context->roots.astFunctionApplicationNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astFunctionApplicationNode_primitiveAnalyzeAndEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context->roots.astFunctionApplicationNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astFunctionApplicationNode_primitiveEvaluate));

    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "lambda", 3, TUUVM_FUNCTION_FLAGS_MACRO | TUUVM_FUNCTION_FLAGS_VARIADIC, NULL, tuuvm_astLambdaNode_primitiveMacro);
    tuuvm_type_setAstNodeAnalysisFunction(context->roots.astLambdaNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astLambdaNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context->roots.astLambdaNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astLambdaNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context->roots.astLambdaNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astLambdaNode_primitiveAnalyzeAndEvaluate));

    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "define", 3, TUUVM_FUNCTION_FLAGS_MACRO | TUUVM_FUNCTION_FLAGS_VARIADIC, NULL, tuuvm_astLocalDefinitionNode_primitiveMacro);
    tuuvm_type_setAstNodeAnalysisFunction(context->roots.astLocalDefinitionNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astLocalDefinitionNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context->roots.astLocalDefinitionNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astLocalDefinitionNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context->roots.astLocalDefinitionNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astLocalDefinitionNode_primitiveAnalyzeAndEvaluate));

    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "if", 4, TUUVM_FUNCTION_FLAGS_MACRO, NULL, tuuvm_astIfNode_primitiveMacro);
    tuuvm_type_setAstNodeAnalysisFunction(context->roots.astIfNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astIfNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context->roots.astIfNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astIfNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context->roots.astIfNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astIfNode_primitiveAnalyzeAndEvaluate));

    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "doWhileContinueWith", 4, TUUVM_FUNCTION_FLAGS_MACRO, NULL, tuuvm_astDoWhileContinueWithNode_primitiveMacro);
    tuuvm_type_setAstNodeAnalysisFunction(context->roots.astDoWhileContinueWithNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astDoWhileContinueWithNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context->roots.astDoWhileContinueWithNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astDoWhileContinueWithNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context->roots.astDoWhileContinueWithNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astDoWhileContinueWithNode_primitiveAnalyzeAndEvaluate));

    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "whileContinueWith", 4, TUUVM_FUNCTION_FLAGS_MACRO, NULL, tuuvm_astWhileContinueWithNode_primitiveMacro);
    tuuvm_type_setAstNodeAnalysisFunction(context->roots.astWhileContinueWithNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astWhileContinueWithNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context->roots.astWhileContinueWithNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astWhileContinueWithNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context->roots.astWhileContinueWithNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_astWhileContinueWithNode_primitiveAnalyzeAndEvaluate));
}
