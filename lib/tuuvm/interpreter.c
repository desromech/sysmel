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
#include "tuuvm/sysmelParser.h"
#include "tuuvm/type.h"
#include "internal/context.h"
#include <stdio.h>
#include <string.h>

//#define tuuvm_gc_safepoint(x) false

TUUVM_API tuuvm_tuple_t tuuvm_interpreter_analyzeASTWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t astNode, tuuvm_tuple_t environment)
{
    struct {
        tuuvm_tuple_t function;
    } gcFrame = {};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.function = tuuvm_type_getAstNodeAnalysisFunction(context, tuuvm_tuple_getType(context, astNode));
    if(!gcFrame.function)
        tuuvm_error("Cannot analyze non AST node tuple.");

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return tuuvm_function_apply2(context, gcFrame.function, astNode, environment);
}

TUUVM_API tuuvm_tuple_t tuuvm_interpreter_evaluateASTWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t astNode, tuuvm_tuple_t environment)
{
    struct {
        tuuvm_tuple_t function;
    } gcFrame = {};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.function = tuuvm_type_getAstNodeEvaluationFunction(context, tuuvm_tuple_getType(context, astNode));
    if(!gcFrame.function)
        tuuvm_error("Cannot evaluate non AST node tuple.");

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return tuuvm_function_apply2(context, gcFrame.function, astNode, environment);
}

TUUVM_API tuuvm_tuple_t tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t astNode, tuuvm_tuple_t environment)
{
    struct {
        tuuvm_tuple_t environment;
        tuuvm_tuple_t astNode;
        tuuvm_tuple_t analyzedAstNode;
        tuuvm_tuple_t function;
        tuuvm_tuple_t result;
    } gcFrame = {
        .environment = environment,
        .astNode = astNode,
    };

    gcFrame.function = tuuvm_type_getAstNodeAnalysisAndEvaluationFunction(context, tuuvm_tuple_getType(context, gcFrame.astNode));
    if(!gcFrame.function)
    {
        TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
        gcFrame.analyzedAstNode = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.astNode, gcFrame.environment);
        gcFrame.result = tuuvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.analyzedAstNode, gcFrame.environment);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.result;
    }

    return tuuvm_function_apply2(context, gcFrame.function, gcFrame.astNode, gcFrame.environment);
}

TUUVM_API tuuvm_tuple_t tuuvm_interpreter_analyzeAndEvaluateSourceCodeWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t sourceCode)
{
    struct {
        tuuvm_tuple_t environment;
        tuuvm_tuple_t sourceCode;
        tuuvm_tuple_t ast;
        tuuvm_tuple_t result;
    } gcFrame = {
        .environment = environment,
        .sourceCode = sourceCode
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    if(tuuvm_string_equalsCString(tuuvm_sourceCode_getLanguage(gcFrame.sourceCode), "sysmel"))
        gcFrame.ast = tuuvm_sysmelParser_parseSourceCode(context, gcFrame.sourceCode);
    else
        gcFrame.ast = tuuvm_parser_parseSourceCode(context, gcFrame.sourceCode);
    gcFrame.result = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.ast, gcFrame.environment);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

TUUVM_API tuuvm_tuple_t tuuvm_interpreter_analyzeAndEvaluateCStringWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t environment, const char *sourceCodeText, const char *sourceCodeName, const char *sourceCodeLanguage)
{
    struct {
        tuuvm_tuple_t environment;
        tuuvm_tuple_t astNode;
        tuuvm_tuple_t result;
    } gcFrame = {
        .environment = environment,
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    if(!strcmp(sourceCodeLanguage, "sysmel"))
        gcFrame.astNode = tuuvm_sysmelParser_parseCString(context, sourceCodeText, sourceCodeName);
    else
        gcFrame.astNode = tuuvm_parser_parseCString(context, sourceCodeText, sourceCodeName);
    gcFrame.result = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.astNode, gcFrame.environment);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

TUUVM_API tuuvm_tuple_t tuuvm_interpreter_analyzeAndEvaluateStringWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t sourceCodeText, tuuvm_tuple_t sourceCodeName, tuuvm_tuple_t sourceCodeLanguage)
{
    struct {
        tuuvm_tuple_t environment;
        tuuvm_tuple_t astNode;
        tuuvm_tuple_t result;
    } gcFrame = {
        .environment = environment,
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.astNode = tuuvm_sourceCode_create(context, sourceCodeText, sourceCodeName, sourceCodeLanguage);
    gcFrame.result = tuuvm_interpreter_analyzeAndEvaluateSourceCodeWithEnvironment(context, gcFrame.environment, gcFrame.astNode);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astSequenceNode_primitiveMacro(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *macroContext = &arguments[0];
    tuuvm_tuple_t *bodyNodes = &arguments[1];

    tuuvm_tuple_t sourcePosition = tuuvm_macroContext_getSourcePosition(*macroContext);
    tuuvm_tuple_t pragmas = tuuvm_arraySlice_createWithArrayOfSize(context, 0);
    return tuuvm_astSequenceNode_create(context, sourcePosition, pragmas, *bodyNodes);
}

static tuuvm_tuple_t tuuvm_astSequenceNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
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

        tuuvm_tuple_t elementValue;
        tuuvm_tuple_t elementType;
        tuuvm_tuple_t concretizeFunction;
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

        if(tuuvm_astNode_isLiteralNode(context, gcFrame.analyzedExpression))
        {
            gcFrame.elementValue = tuuvm_astLiteralNode_getValue(gcFrame.analyzedExpression);
            gcFrame.elementType = tuuvm_tuple_getType(context, gcFrame.elementValue);
            gcFrame.concretizeFunction = tuuvm_type_getAnalyzeConcreteSequenceElementWithEnvironmentFunction(context, gcFrame.elementType);
            if(gcFrame.concretizeFunction)
                gcFrame.analyzedExpression = tuuvm_function_apply3(context, gcFrame.concretizeFunction, gcFrame.elementType, gcFrame.analyzedExpression, *environment);
        }

        tuuvm_arraySlice_atPut(gcFrame.analyzedExpressions, i, gcFrame.analyzedExpression);
    }

    gcFrame.analyzedSequenceNode->expressions = gcFrame.analyzedExpressions;
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.analyzedSequenceNode;
}

static tuuvm_tuple_t tuuvm_astSequenceNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astSequenceNode_t **sequenceNode = (tuuvm_astSequenceNode_t**)node;

    struct {
        tuuvm_tuple_t expression;
        tuuvm_tuple_t result;
    } gcFrame = {
        .result = TUUVM_VOID_TUPLE,
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*sequenceNode)->super.sourcePosition);

    size_t expressionCount = tuuvm_arraySlice_getSize((*sequenceNode)->expressions);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expression = tuuvm_arraySlice_at((*sequenceNode)->expressions, i);
        gcFrame.result = tuuvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.expression, *environment);
    }

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);

    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astSequenceNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_tuple_t result;
        tuuvm_tuple_t expression;
        tuuvm_tuple_t elementType;
        tuuvm_tuple_t concretizeFunction;
    } gcFrame = {
        .result = TUUVM_VOID_TUPLE
    };
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    tuuvm_astSequenceNode_t **sequenceNode = (tuuvm_astSequenceNode_t**)node;
    size_t expressionCount = tuuvm_arraySlice_getSize((*sequenceNode)->expressions);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expression = tuuvm_arraySlice_at((*sequenceNode)->expressions, i);
        gcFrame.result = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.expression, *environment);
        
        gcFrame.elementType = tuuvm_tuple_getType(context, gcFrame.result);
        gcFrame.concretizeFunction = tuuvm_type_getAnalyzeAndEvaluateConcreteSequenceElementWithEnvironmentFunction(context, gcFrame.elementType);
        if(gcFrame.concretizeFunction)
            gcFrame.result = tuuvm_function_apply3(context, gcFrame.concretizeFunction, gcFrame.elementType, gcFrame.result, *environment);
    }

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astLambdaNode_parseArgumentsNodes(tuuvm_context_t *context, tuuvm_tuple_t unsafeArgumentsNode, bool *hasVariadicArguments)
{
    struct {
        tuuvm_tuple_t argumentsNode;
        tuuvm_tuple_t argumentList;

        tuuvm_tuple_t unparsedArgumentNode;
        tuuvm_tuple_t isForAll;
        tuuvm_tuple_t nameExpression;
        tuuvm_tuple_t typeExpression;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.argumentsNode = unsafeArgumentsNode;
    gcFrame.argumentList = tuuvm_arrayList_create(context);
    size_t argumentNodeCount = tuuvm_arraySlice_getSize(gcFrame.argumentsNode);
    *hasVariadicArguments = false;
    for(size_t i = 0; i < argumentNodeCount; ++i)
    {
        gcFrame.unparsedArgumentNode = tuuvm_arraySlice_at(gcFrame.argumentsNode, i);
        if(tuuvm_astNode_isIdentifierReferenceNode(context, gcFrame.unparsedArgumentNode))
        {
            if(tuuvm_astIdentifierReferenceNode_isEllipsis(gcFrame.unparsedArgumentNode))
            {
                if(i + 1 != argumentNodeCount)
                    tuuvm_error("Ellipsis can only be present at the end.");
                else if(i == 0)
                    tuuvm_error("Ellipsis cannot be the first argument.");

                *hasVariadicArguments = true;
                break;
            }

            gcFrame.isForAll = TUUVM_FALSE_TUPLE;
            gcFrame.nameExpression = tuuvm_astLiteralNode_create(context, tuuvm_astNode_getSourcePosition(gcFrame.unparsedArgumentNode), tuuvm_astIdentifierReferenceNode_getValue(gcFrame.unparsedArgumentNode));
            gcFrame.typeExpression = TUUVM_NULL_TUPLE;

        }
        else
        {
            tuuvm_error("Invalid argument definition node.");
        }

        tuuvm_arrayList_add(context, gcFrame.argumentList, tuuvm_astArgumentNode_create(context, tuuvm_astNode_getSourcePosition(gcFrame.unparsedArgumentNode), gcFrame.isForAll, gcFrame.nameExpression, gcFrame.typeExpression));
    }

    tuuvm_tuple_t result = tuuvm_arrayList_asArraySlice(context, gcFrame.argumentList);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return result;
}

static tuuvm_tuple_t tuuvm_astLambdaNode_primitiveMacro(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
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

    bool hasVariadicArguments = false;
    gcFrame.argumentsNode = tuuvm_astUnexpandedSExpressionNode_getElements(*argumentsSExpressionNode);
    gcFrame.sourcePosition = tuuvm_macroContext_getSourcePosition(*macroContext);
    gcFrame.argumentsArraySlice = tuuvm_astLambdaNode_parseArgumentsNodes(context, gcFrame.argumentsNode, &hasVariadicArguments);
    tuuvm_tuple_t pragmas = tuuvm_arraySlice_createWithArrayOfSize(context, 0);
    gcFrame.bodySequence = tuuvm_astSequenceNode_create(context, gcFrame.sourcePosition, pragmas, *bodyNodes);
    tuuvm_tuple_t result = tuuvm_astLambdaNode_create(context, gcFrame.sourcePosition,
        tuuvm_tuple_size_encode(context, hasVariadicArguments ? TUUVM_FUNCTION_FLAGS_VARIADIC : TUUVM_FUNCTION_FLAGS_NONE),
        tuuvm_tuple_size_encode(context, tuuvm_arraySlice_getSize(gcFrame.argumentsArraySlice)),
        gcFrame.argumentsArraySlice, TUUVM_NULL_TUPLE, gcFrame.bodySequence);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return result;
}


static tuuvm_tuple_t tuuvm_astArgumentNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_astArgumentNode_t *argumentNode;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.argumentNode = (tuuvm_astArgumentNode_t*)tuuvm_context_shallowCopy(context, *node);
    if(gcFrame.argumentNode->name)
        gcFrame.argumentNode->name = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.argumentNode->name, *environment);
    if(gcFrame.argumentNode->type)
        gcFrame.argumentNode->type = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.argumentNode->type, *environment);

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.argumentNode;
}

static tuuvm_tuple_t tuuvm_astLambdaNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_astLambdaNode_t *lambdaNode;
        tuuvm_tuple_t lambdaAnalysisEnvironment;
        tuuvm_tuple_t argumentNode;
        tuuvm_tuple_t argumentsNode;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.lambdaNode = (tuuvm_astLambdaNode_t*)tuuvm_context_shallowCopy(context, *node);
    gcFrame.lambdaAnalysisEnvironment = tuuvm_environment_create(context, *environment);

    size_t lambdaArgumentCount = 0;
    size_t argumentNodeCount = tuuvm_arraySlice_getSize(gcFrame.lambdaNode->arguments);
    gcFrame.argumentsNode = tuuvm_arraySlice_createWithArrayOfSize(context, argumentNodeCount);
    for(size_t i = 0; i < argumentNodeCount; ++i)
    {
        gcFrame.argumentNode = tuuvm_arraySlice_at(gcFrame.lambdaNode->arguments, i);
        gcFrame.argumentNode = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.argumentNode, *environment);

        if(!tuuvm_astArgumentNode_isForAll(gcFrame.argumentNode))
            ++lambdaArgumentCount;

        tuuvm_arraySlice_atPut(gcFrame.argumentsNode, i, gcFrame.argumentNode);
    }

    gcFrame.lambdaNode->argumentCount = tuuvm_tuple_size_encode(context,  lambdaArgumentCount);
    gcFrame.lambdaNode->arguments = gcFrame.argumentsNode;
    
    gcFrame.lambdaNode->body = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.lambdaNode->body, gcFrame.lambdaAnalysisEnvironment);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.lambdaNode;
}

static tuuvm_tuple_t tuuvm_astLambdaNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astLambdaNode_t **lambdaNode = (tuuvm_astLambdaNode_t**)node;

    return tuuvm_function_createClosureAST(context, (*lambdaNode)->super.sourcePosition, (*lambdaNode)->flags, (*lambdaNode)->argumentCount, *environment, (*lambdaNode)->arguments, (*lambdaNode)->resultType, (*lambdaNode)->body);
}

static tuuvm_tuple_t tuuvm_astLambdaNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_tuple_t lambdaAnalysisEnvironment;
        tuuvm_tuple_t analyzedArgumentCount;
        tuuvm_tuple_t analyzedArguments;
        tuuvm_tuple_t analyzedBody;
        tuuvm_tuple_t argumentNode;
        tuuvm_tuple_t result;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    tuuvm_astLambdaNode_t **lambdaNode = (tuuvm_astLambdaNode_t**)node;

    gcFrame.lambdaAnalysisEnvironment = tuuvm_environment_create(context, *environment);

    size_t lambdaArgumentCount = 0;
    size_t argumentNodeCount = tuuvm_arraySlice_getSize((*lambdaNode)->arguments);
    gcFrame.analyzedArguments = tuuvm_arraySlice_createWithArrayOfSize(context, argumentNodeCount);
    for(size_t i = 0; i < argumentNodeCount; ++i)
    {
        gcFrame.argumentNode = tuuvm_arraySlice_at((*lambdaNode)->arguments, i);
        gcFrame.argumentNode = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.argumentNode, *environment);

        if(!tuuvm_astArgumentNode_isForAll(gcFrame.argumentNode))
            ++lambdaArgumentCount;

        tuuvm_arraySlice_atPut(gcFrame.analyzedArguments, i, gcFrame.argumentNode);
    }

    gcFrame.analyzedArgumentCount = tuuvm_tuple_size_encode(context, lambdaArgumentCount);
    gcFrame.analyzedBody = tuuvm_interpreter_analyzeASTWithEnvironment(context, (*lambdaNode)->body, gcFrame.lambdaAnalysisEnvironment);

    gcFrame.result = tuuvm_function_createClosureAST(context, (*lambdaNode)->super.sourcePosition, (*lambdaNode)->flags, gcFrame.analyzedArgumentCount, *environment, gcFrame.analyzedArguments, (*lambdaNode)->resultType, gcFrame.analyzedBody);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astLiteralNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];

    return *node;
}

static tuuvm_tuple_t tuuvm_astLiteralNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t node = arguments[0];

    return ((tuuvm_astLiteralNode_t*)node)->value;
}

static tuuvm_tuple_t tuuvm_astLocalDefinitionNode_letWithPrimitiveMacro(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) tuuvm_error_argumentCountMismatch(3, argumentCount);

    tuuvm_tuple_t *macroContext = &arguments[0];
    tuuvm_tuple_t *name = &arguments[1];
    tuuvm_tuple_t *value = &arguments[2];

    tuuvm_tuple_t sourcePosition = tuuvm_macroContext_getSourcePosition(*macroContext);
    return tuuvm_astLocalDefinitionNode_create(context, sourcePosition, *name, TUUVM_NULL_TUPLE, *value);
}

static tuuvm_tuple_t tuuvm_astLocalDefinitionNode_primitiveMacro(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
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

        bool hasVariadicArguments = false;
        gcFrame.argumentsNode = tuuvm_arraySlice_fromOffset(context, gcFrame.lambdaSignatureElements, 1);
        gcFrame.arguments = tuuvm_astLambdaNode_parseArgumentsNodes(context, gcFrame.argumentsNode, &hasVariadicArguments);
        tuuvm_tuple_t pragmas = tuuvm_arraySlice_createWithArrayOfSize(context, 0);
        gcFrame.bodySequence = tuuvm_astSequenceNode_create(context, gcFrame.sourcePosition, pragmas, *valueOrBodyNodes);
        gcFrame.valueNode = tuuvm_astLambdaNode_create(context, gcFrame.sourcePosition,
            tuuvm_tuple_size_encode(context, hasVariadicArguments ? TUUVM_FUNCTION_FLAGS_VARIADIC : TUUVM_FUNCTION_FLAGS_NONE),
            tuuvm_tuple_size_encode(context, tuuvm_arraySlice_getSize(gcFrame.arguments)),
            gcFrame.arguments, TUUVM_NULL_TUPLE, gcFrame.bodySequence);
    }
    else
    {
        tuuvm_error("Invalid usage of (define)");
    }

    gcFrame.nameExpression = tuuvm_astLiteralNode_create(context, tuuvm_astNode_getSourcePosition(gcFrame.nameNode), tuuvm_astIdentifierReferenceNode_getValue(gcFrame.nameNode));
    tuuvm_tuple_t result = tuuvm_astLocalDefinitionNode_create(context, gcFrame.sourcePosition, gcFrame.nameExpression, TUUVM_NULL_TUPLE, gcFrame.valueNode);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return result;
}

static tuuvm_tuple_t tuuvm_astLocalDefinitionNode_primitiveDefineMacro(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
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

    if(tuuvm_astNode_isUnexpandedSExpressionNode(context, *nameOrLambdaSignature))
    {
        gcFrame.lambdaSignatureElements = tuuvm_astUnexpandedSExpressionNode_getElements(*nameOrLambdaSignature);
        if(tuuvm_arraySlice_getSize(gcFrame.lambdaSignatureElements) < 1)
            tuuvm_error("Expected function definition requires a name.");

        gcFrame.nameNode = tuuvm_arraySlice_at(gcFrame.lambdaSignatureElements, 0);
        if(!tuuvm_astNode_isIdentifierReferenceNode(context, gcFrame.nameNode))
            tuuvm_error("Expected an identifier reference node for the name.");

        bool hasVariadicArguments = false;
        gcFrame.argumentsNode = tuuvm_arraySlice_fromOffset(context, gcFrame.lambdaSignatureElements, 1);
        gcFrame.arguments = tuuvm_astLambdaNode_parseArgumentsNodes(context, gcFrame.argumentsNode, &hasVariadicArguments);
        tuuvm_tuple_t pragmas = tuuvm_arraySlice_createWithArrayOfSize(context, 0);
        gcFrame.bodySequence = tuuvm_astSequenceNode_create(context, gcFrame.sourcePosition, pragmas, *valueOrBodyNodes);
        gcFrame.valueNode = tuuvm_astLambdaNode_create(context, gcFrame.sourcePosition,
            tuuvm_tuple_size_encode(context, (hasVariadicArguments ? TUUVM_FUNCTION_FLAGS_VARIADIC : TUUVM_FUNCTION_FLAGS_NONE) | TUUVM_FUNCTION_FLAGS_MACRO),
            tuuvm_tuple_size_encode(context, tuuvm_arraySlice_getSize(gcFrame.arguments)),
            gcFrame.arguments, TUUVM_NULL_TUPLE, gcFrame.bodySequence);
    }
    else
    {
        tuuvm_error("Invalid usage of (define)");
    }

    gcFrame.nameExpression = tuuvm_astLiteralNode_create(context, tuuvm_astNode_getSourcePosition(gcFrame.nameNode), tuuvm_astIdentifierReferenceNode_getValue(gcFrame.nameNode));
    tuuvm_tuple_t result = tuuvm_astLocalDefinitionNode_create(context, gcFrame.sourcePosition, gcFrame.nameExpression, TUUVM_NULL_TUPLE, gcFrame.valueNode);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return result;
}

static tuuvm_tuple_t tuuvm_astLocalDefinitionNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
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
    if(gcFrame.localDefinitionNode->typeExpression)
        gcFrame.localDefinitionNode->typeExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.localDefinitionNode->typeExpression, *environment);
    gcFrame.localDefinitionNode->valueExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.localDefinitionNode->valueExpression, *environment);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.localDefinitionNode;
}

static tuuvm_tuple_t tuuvm_astLocalDefinitionNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_tuple_t name;
        tuuvm_tuple_t type;
        tuuvm_tuple_t value;
    } gcFrame = {};

    tuuvm_astLocalDefinitionNode_t **localDefinitionNode = (tuuvm_astLocalDefinitionNode_t**)node;

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*localDefinitionNode)->super.sourcePosition);

    gcFrame.name = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*localDefinitionNode)->nameExpression, *environment);
    if((*localDefinitionNode)->typeExpression)
        gcFrame.type = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*localDefinitionNode)->typeExpression, *environment);
    gcFrame.value = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*localDefinitionNode)->valueExpression, *environment);
    if(gcFrame.type)
        gcFrame.value = tuuvm_type_coerceValue(context, gcFrame.type, gcFrame.value);
    tuuvm_environment_setNewSymbolBinding(context, *environment, gcFrame.name, gcFrame.value);
    //TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.value;
}

static tuuvm_tuple_t tuuvm_astLocalDefinitionNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_tuple_t name;
        tuuvm_tuple_t type;
        tuuvm_tuple_t value;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    tuuvm_astLocalDefinitionNode_t **localDefinitionNode = (tuuvm_astLocalDefinitionNode_t**)node;
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*localDefinitionNode)->super.sourcePosition);

    gcFrame.name = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*localDefinitionNode)->nameExpression, *environment);
    if((*localDefinitionNode)->typeExpression)
        gcFrame.type = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*localDefinitionNode)->typeExpression, *environment);
    gcFrame.value = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*localDefinitionNode)->valueExpression, *environment);
    if(gcFrame.type)
        gcFrame.value = tuuvm_type_coerceValue(context, gcFrame.type, gcFrame.value);
    tuuvm_environment_setNewSymbolBinding(context, *environment, gcFrame.name, gcFrame.value);
    //TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.value;
}

static tuuvm_tuple_t tuuvm_astIdentifierReferenceNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
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

static tuuvm_tuple_t tuuvm_astIdentifierReferenceNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
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

    fprintf(stderr, "Failed to find symbol binding for: " TUUVM_STRING_PRINTF_FORMAT "\n", TUUVM_STRING_PRINTF_ARG((*referenceNode)->value));

    tuuvm_error("Failed to find symbol binding");
    return TUUVM_NULL_TUPLE;
}

static tuuvm_tuple_t tuuvm_astIfNode_primitiveMacro(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
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

static tuuvm_tuple_t tuuvm_astIfNode_primitiveMacroIfThen(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) tuuvm_error_argumentCountMismatch(3, argumentCount);

    tuuvm_tuple_t *macroContext = &arguments[0];
    tuuvm_tuple_t *conditionNode = &arguments[1];
    tuuvm_tuple_t *trueExpressionNode = &arguments[2];

    tuuvm_tuple_t sourcePosition = tuuvm_macroContext_getSourcePosition(*macroContext);

    return tuuvm_astIfNode_create(context, sourcePosition, *conditionNode, *trueExpressionNode, TUUVM_NULL_TUPLE);
}

static tuuvm_tuple_t tuuvm_astIfNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
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
    if(gcFrame.ifNode->trueExpression)
        gcFrame.ifNode->trueExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.ifNode->trueExpression, *environment);
    if(gcFrame.ifNode->falseExpression)
        gcFrame.ifNode->falseExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.ifNode->falseExpression, *environment);
    return (tuuvm_tuple_t)gcFrame.ifNode;
}

static tuuvm_tuple_t tuuvm_astIfNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astIfNode_t **ifNode = (tuuvm_astIfNode_t**)node;

    struct {
        tuuvm_tuple_t condition;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*ifNode)->super.sourcePosition);
    gcFrame.condition = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*ifNode)->conditionExpression, *environment);
    if(tuuvm_tuple_boolean_decode(gcFrame.condition))
    {
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        if(!(*ifNode)->trueExpression)
            return TUUVM_VOID_TUPLE;
        return tuuvm_interpreter_evaluateASTWithEnvironment(context, (*ifNode)->trueExpression, *environment);
    }
    else
    {
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        if(!(*ifNode)->falseExpression)
            return TUUVM_VOID_TUPLE;
        return tuuvm_interpreter_evaluateASTWithEnvironment(context, (*ifNode)->falseExpression, *environment);
    }
}

static tuuvm_tuple_t tuuvm_astIfNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astIfNode_t **ifNode = (tuuvm_astIfNode_t**)node;
    struct {
        tuuvm_tuple_t condition;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*ifNode)->super.sourcePosition);
    gcFrame.condition = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*ifNode)->conditionExpression, *environment);
    if(tuuvm_tuple_boolean_decode(gcFrame.condition))
    {
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        if(!(*ifNode)->trueExpression)
            return TUUVM_VOID_TUPLE;
        return tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*ifNode)->trueExpression, *environment);
    }
    else
    {
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        if(!(*ifNode)->falseExpression)
            return TUUVM_VOID_TUPLE;
        return tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*ifNode)->falseExpression, *environment);
    }
}

static tuuvm_tuple_t tuuvm_astUnexpandedApplicationNode_expandNodeWithMacro(tuuvm_context_t *context, tuuvm_tuple_t *node, tuuvm_tuple_t *macro, tuuvm_tuple_t *environment)
{
    tuuvm_astUnexpandedApplicationNode_t **unexpandedNode = (tuuvm_astUnexpandedApplicationNode_t**)node;

    size_t applicationArgumentCount = tuuvm_arraySlice_getSize((*unexpandedNode)->arguments);

    tuuvm_functionCallFrameStack_t callFrameStack = {};
    struct {
        tuuvm_tuple_t macroContext;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    tuuvm_functionCallFrameStack_begin(context, &callFrameStack, *macro, 1 + applicationArgumentCount);
    gcFrame.macroContext = tuuvm_macroContext_create(context, *node, (*unexpandedNode)->super.sourcePosition, *environment);
    tuuvm_functionCallFrameStack_push(&callFrameStack, gcFrame.macroContext);
    for(size_t i = 0; i < applicationArgumentCount; ++i)
        tuuvm_functionCallFrameStack_push(&callFrameStack, tuuvm_arraySlice_at((*unexpandedNode)->arguments, i));

    TUUVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return tuuvm_functionCallFrameStack_finish(context, &callFrameStack);
}

static tuuvm_tuple_t tuuvm_astUnexpandedApplicationNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astUnexpandedApplicationNode_t **unexpandedNode = (tuuvm_astUnexpandedApplicationNode_t**)node;

    struct {
        tuuvm_tuple_t macro;
        tuuvm_tuple_t functionOrMacroExpression;
        tuuvm_tuple_t expansionResult;
        tuuvm_tuple_t applicationNode;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*unexpandedNode)->super.sourcePosition);

    gcFrame.functionOrMacroExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, (*unexpandedNode)->functionOrMacroExpression, *environment);

    // Is this a macro?
    bool isMacro = tuuvm_astNode_isMacroExpression(context, gcFrame.functionOrMacroExpression);
    if(isMacro)
    {
        gcFrame.macro = tuuvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.functionOrMacroExpression, *environment);
        gcFrame.expansionResult = tuuvm_astUnexpandedApplicationNode_expandNodeWithMacro(context, node, &gcFrame.macro, environment);
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.expansionResult, *environment);
    }

    // Convert into application node and then analyze it.
    gcFrame.applicationNode = tuuvm_astFunctionApplicationNode_create(context, (*unexpandedNode)->super.sourcePosition, gcFrame.functionOrMacroExpression, (*unexpandedNode)->arguments);
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.applicationNode, *environment);
}

static tuuvm_tuple_t tuuvm_astUnexpandedApplicationNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astUnexpandedApplicationNode_t **unexpandedNode = (tuuvm_astUnexpandedApplicationNode_t**)node;

    struct {
        tuuvm_tuple_t functionOrMacro;
        tuuvm_tuple_t expansionResult;
        tuuvm_tuple_t argumentNode;
        tuuvm_tuple_t argument;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.functionOrMacro = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*unexpandedNode)->functionOrMacroExpression, *environment);
    bool isMacro = tuuvm_function_isMacro(context, gcFrame.functionOrMacro);

    if(isMacro)
    {
        gcFrame.expansionResult = tuuvm_astUnexpandedApplicationNode_expandNodeWithMacro(context, node, &gcFrame.functionOrMacro, environment);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.expansionResult, *environment);
    }

    tuuvm_functionCallFrameStack_t callFrameStack = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    size_t applicationArgumentCount = tuuvm_arraySlice_getSize((*unexpandedNode)->arguments);
    tuuvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.functionOrMacro, applicationArgumentCount);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = tuuvm_arraySlice_at((*unexpandedNode)->arguments, i);
        gcFrame.argument = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.argumentNode, *environment);
        tuuvm_functionCallFrameStack_push(&callFrameStack, gcFrame.argument);
    }

    //TUUVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return tuuvm_functionCallFrameStack_finish(context, &callFrameStack);
}

static tuuvm_tuple_t tuuvm_astUnexpandedSExpressionNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
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

static tuuvm_tuple_t tuuvm_astUnexpandedSExpressionNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
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

static tuuvm_tuple_t tuuvm_astFunctionApplicationNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
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

static tuuvm_tuple_t tuuvm_astFunctionApplicationNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astFunctionApplicationNode_t **applicationNode = (tuuvm_astFunctionApplicationNode_t**)node;

    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*applicationNode)->super.sourcePosition);

    tuuvm_functionCallFrameStack_t callFrameStack = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    size_t applicationArgumentCount = tuuvm_arraySlice_getSize((*applicationNode)->arguments);
    tuuvm_functionCallFrameStack_begin(context, &callFrameStack, tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*applicationNode)->functionExpression, *environment), applicationArgumentCount);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        tuuvm_tuple_t argumentNode = tuuvm_arraySlice_at((*applicationNode)->arguments, i);
        tuuvm_functionCallFrameStack_push(&callFrameStack, tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, argumentNode, *environment));
    }

    TUUVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    tuuvm_tuple_t result = tuuvm_functionCallFrameStack_finish(context, &callFrameStack);
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    return result;
}

static tuuvm_tuple_t tuuvm_astFunctionApplicationNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astFunctionApplicationNode_t **applicationNode = (tuuvm_astFunctionApplicationNode_t**)node;
    struct {
        tuuvm_tuple_t argumentNode;
        tuuvm_tuple_t result;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*applicationNode)->super.sourcePosition);

    tuuvm_functionCallFrameStack_t callFrameStack = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    size_t applicationArgumentCount = tuuvm_arraySlice_getSize((*applicationNode)->arguments);
    tuuvm_functionCallFrameStack_begin(context, &callFrameStack, tuuvm_interpreter_evaluateASTWithEnvironment(context, (*applicationNode)->functionExpression, *environment), applicationArgumentCount);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = tuuvm_arraySlice_at((*applicationNode)->arguments, i);
        tuuvm_functionCallFrameStack_push(&callFrameStack, tuuvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.argumentNode, *environment));
    }

    TUUVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    gcFrame.result = tuuvm_functionCallFrameStack_finish(context, &callFrameStack);
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astLexicalBlockNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_astLexicalBlockNode_t *lexicalBlockNode;
        tuuvm_tuple_t childEnvironment;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.lexicalBlockNode = (tuuvm_astLexicalBlockNode_t*)tuuvm_context_shallowCopy(context, *node);
    gcFrame.childEnvironment = tuuvm_environment_create(context, *environment);
    gcFrame.lexicalBlockNode->body = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.lexicalBlockNode->body, gcFrame.childEnvironment);
    
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.lexicalBlockNode;
}

static tuuvm_tuple_t tuuvm_astLexicalBlockNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astLexicalBlockNode_t **lexicalBlockNode = (tuuvm_astLexicalBlockNode_t **)node;

    struct {
        tuuvm_tuple_t childEnvironment;
        tuuvm_tuple_t result;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*lexicalBlockNode)->super.sourcePosition);
    
    gcFrame.childEnvironment = tuuvm_environment_create(context, *environment);
    gcFrame.result = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*lexicalBlockNode)->body, gcFrame.childEnvironment);

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astLexicalBlockNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astLexicalBlockNode_t **lexicalBlockNode = (tuuvm_astLexicalBlockNode_t **)node;

    struct {
        tuuvm_tuple_t childEnvironment;
        tuuvm_tuple_t result;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*lexicalBlockNode)->super.sourcePosition);
    
    gcFrame.childEnvironment = tuuvm_environment_create(context, *environment);
    gcFrame.result = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*lexicalBlockNode)->body, gcFrame.childEnvironment);

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astMakeByteArrayNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astMakeByteArrayNode_t **tupleNode = (tuuvm_astMakeByteArrayNode_t**)node;

    struct {
        tuuvm_astMakeByteArrayNode_t *analyzedTupleNode;
        tuuvm_tuple_t expressions;
        tuuvm_tuple_t analyzedExpressions;
        tuuvm_tuple_t expression;
        tuuvm_tuple_t analyzedExpression;
    } gcFrame = {};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.expressions = (*tupleNode)->elements;
    size_t expressionCount = tuuvm_arraySlice_getSize(gcFrame.expressions);
    if(expressionCount == 0)
    {
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return *node;
    }

    gcFrame.analyzedTupleNode = (tuuvm_astMakeByteArrayNode_t *)tuuvm_context_shallowCopy(context, *node);
    
    gcFrame.analyzedExpressions = tuuvm_arraySlice_createWithArrayOfSize(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expression = tuuvm_arraySlice_at(gcFrame.expressions, i);
        gcFrame.analyzedExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.expression, *environment);
        tuuvm_arraySlice_atPut(gcFrame.analyzedExpressions, i, gcFrame.analyzedExpression);
    }

    gcFrame.analyzedTupleNode->elements = gcFrame.analyzedExpressions;
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.analyzedTupleNode;
}

static tuuvm_tuple_t tuuvm_astMakeByteArrayNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astMakeByteArrayNode_t **byteArrayNode = (tuuvm_astMakeByteArrayNode_t**)node;

    struct {
        tuuvm_tuple_t result;
        tuuvm_tuple_t element;
    } gcFrame = {};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*byteArrayNode)->super.sourcePosition);

    size_t expressionCount = tuuvm_arraySlice_getSize((*byteArrayNode)->elements);
    gcFrame.result = tuuvm_byteArray_create(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        tuuvm_tuple_t expression = tuuvm_arraySlice_at((*byteArrayNode)->elements, i);
        gcFrame.element = tuuvm_interpreter_evaluateASTWithEnvironment(context, expression, *environment);
        tuuvm_arrayOrByteArray_atPut(gcFrame.result, i, gcFrame.element);
    }

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astMakeByteArrayNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astMakeByteArrayNode_t **byteArrayNode = (tuuvm_astMakeByteArrayNode_t**)node;

    struct {
        tuuvm_tuple_t result;
        tuuvm_tuple_t element;
    } gcFrame = {};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*byteArrayNode)->super.sourcePosition);

    size_t expressionCount = tuuvm_arraySlice_getSize((*byteArrayNode)->elements);
    gcFrame.result = tuuvm_byteArray_create(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        tuuvm_tuple_t expression = tuuvm_arraySlice_at((*byteArrayNode)->elements, i);
        gcFrame.element = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, expression, *environment);
        tuuvm_arrayOrByteArray_atPut(gcFrame.result, i, gcFrame.element);
    }

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astMakeTupleNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astMakeTupleNode_t **tupleNode = (tuuvm_astMakeTupleNode_t**)node;

    struct {
        tuuvm_astMakeTupleNode_t *analyzedTupleNode;
        tuuvm_tuple_t expressions;
        tuuvm_tuple_t analyzedExpressions;
        tuuvm_tuple_t expression;
        tuuvm_tuple_t analyzedExpression;
    } gcFrame = {};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.expressions = (*tupleNode)->elements;
    size_t expressionCount = tuuvm_arraySlice_getSize(gcFrame.expressions);
    if(expressionCount == 0)
    {
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return *node;
    }

    gcFrame.analyzedTupleNode = (tuuvm_astMakeTupleNode_t *)tuuvm_context_shallowCopy(context, *node);
    
    gcFrame.analyzedExpressions = tuuvm_arraySlice_createWithArrayOfSize(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expression = tuuvm_arraySlice_at(gcFrame.expressions, i);
        gcFrame.analyzedExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.expression, *environment);
        tuuvm_arraySlice_atPut(gcFrame.analyzedExpressions, i, gcFrame.analyzedExpression);
    }

    gcFrame.analyzedTupleNode->elements = gcFrame.analyzedExpressions;
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.analyzedTupleNode;
}

static tuuvm_tuple_t tuuvm_astMakeTupleNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astMakeTupleNode_t **tupleNode = (tuuvm_astMakeTupleNode_t**)node;

    struct {
        tuuvm_tuple_t result;
        tuuvm_tuple_t element;
    } gcFrame = {};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*tupleNode)->super.sourcePosition);

    size_t expressionCount = tuuvm_arraySlice_getSize((*tupleNode)->elements);
    gcFrame.result = tuuvm_array_create(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        tuuvm_tuple_t expression = tuuvm_arraySlice_at((*tupleNode)->elements, i);
        gcFrame.element = tuuvm_interpreter_evaluateASTWithEnvironment(context, expression, *environment);
        tuuvm_array_atPut(gcFrame.result, i, gcFrame.element);
    }

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astMakeTupleNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astMakeTupleNode_t **tupleNode = (tuuvm_astMakeTupleNode_t**)node;

    struct {
        tuuvm_tuple_t result;
        tuuvm_tuple_t element;
    } gcFrame = {};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*tupleNode)->super.sourcePosition);

    size_t expressionCount = tuuvm_arraySlice_getSize((*tupleNode)->elements);
    gcFrame.result = tuuvm_array_create(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        tuuvm_tuple_t expression = tuuvm_arraySlice_at((*tupleNode)->elements, i);
        gcFrame.element = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, expression, *environment);
        tuuvm_array_atPut(gcFrame.result, i, gcFrame.element);
    }

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astMessageChainMessageNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_astMessageChainMessageNode_t *sendNode;
        tuuvm_tuple_t analyzedArguments;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sendNode = (tuuvm_astMessageChainMessageNode_t*)tuuvm_context_shallowCopy(context, *node);
    gcFrame.sendNode->selector = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.sendNode->selector, *environment);

    size_t applicationArgumentCount = tuuvm_arraySlice_getSize(gcFrame.sendNode->arguments);
    gcFrame.analyzedArguments = tuuvm_array_create(context, applicationArgumentCount);
    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        tuuvm_tuple_t argumentNode = tuuvm_arraySlice_at(gcFrame.sendNode->arguments, i);
        tuuvm_array_atPut(gcFrame.analyzedArguments, i, tuuvm_interpreter_analyzeASTWithEnvironment(context, argumentNode, *environment));
    }

    gcFrame.sendNode->arguments = tuuvm_array_asArraySlice(context, gcFrame.analyzedArguments);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.sendNode;
}

static tuuvm_tuple_t tuuvm_astMessageChainNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_astMessageChainNode_t *chainNode;
        tuuvm_tuple_t analyzedChainedMessages;
        tuuvm_tuple_t chainedMessageNode;

        tuuvm_tuple_t receiverValue;
        tuuvm_tuple_t receiverType;
        tuuvm_tuple_t analysisFunction;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.chainNode = (tuuvm_astMessageChainNode_t*)tuuvm_context_shallowCopy(context, *node);
    if(gcFrame.chainNode->receiver)
    {
        gcFrame.chainNode->receiver = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.chainNode->receiver, *environment);
        if(tuuvm_astNode_isLiteralNode(context, gcFrame.chainNode->receiver))
        {
            gcFrame.receiverType = tuuvm_tuple_getType(context, gcFrame.receiverValue);
            gcFrame.analysisFunction = tuuvm_type_getAnalyzeMessageChainNodeWithEnvironmentFunction(context, tuuvm_tuple_getType(context, gcFrame.receiverType));
            if(gcFrame.analysisFunction)
            {
                TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
                return tuuvm_function_apply3(context, gcFrame.analysisFunction, gcFrame.receiverType, (tuuvm_tuple_t)gcFrame.chainNode, *environment);
            }

        }
    }

    size_t chainedMessageCount = tuuvm_arraySlice_getSize(gcFrame.chainNode->messages);
    gcFrame.analyzedChainedMessages = tuuvm_array_create(context, chainedMessageCount);
    for(size_t i = 0; i < chainedMessageCount; ++i)
    {
        gcFrame.chainedMessageNode = tuuvm_arraySlice_at(gcFrame.chainNode->messages, i);
        tuuvm_array_atPut(gcFrame.analyzedChainedMessages, i, tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.chainedMessageNode, *environment));
    }

    gcFrame.chainNode->messages = tuuvm_array_asArraySlice(context, gcFrame.analyzedChainedMessages);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.chainNode;
}

static tuuvm_tuple_t tuuvm_astMessageChainMessageNode_analyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t chainedMessage, bool hasReceiver, tuuvm_tuple_t *receiver, tuuvm_tuple_t *environment)
{
    struct {
        tuuvm_astMessageChainMessageNode_t *node;
        tuuvm_tuple_t selector;
        tuuvm_tuple_t method;
        tuuvm_tuple_t result;
        tuuvm_tuple_t argumentNode;
        tuuvm_tuple_t argument;
    } gcFrame = {
        .node = (tuuvm_astMessageChainMessageNode_t*)chainedMessage
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.node->super.sourcePosition);

    gcFrame.selector = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.node->selector, *environment);
    if(hasReceiver)
    {
        gcFrame.method = tuuvm_type_lookupSelector(context, tuuvm_tuple_getType(context, *receiver), gcFrame.selector);
        if(!gcFrame.method)
            tuuvm_error("TODO: fallback to doesNotUnderstand:");
    }
    else
    {
        if(!tuuvm_environment_lookSymbolRecursively(context, *environment, gcFrame.selector, &gcFrame.method))
            tuuvm_error("Failed to find symbol for message send without receiver.");
    }


    tuuvm_functionCallFrameStack_t callFrameStack = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    size_t applicationArgumentCount = tuuvm_arraySlice_getSize(gcFrame.node->arguments);
    tuuvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.method, applicationArgumentCount + (hasReceiver ? 1 : 0));

    if(hasReceiver)
        tuuvm_functionCallFrameStack_push(&callFrameStack, *receiver);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = tuuvm_arraySlice_at(gcFrame.node->arguments, i);
        gcFrame.argument = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.argumentNode, *environment);
        tuuvm_functionCallFrameStack_push(&callFrameStack, gcFrame.argument);
    }

    TUUVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    gcFrame.result = tuuvm_functionCallFrameStack_finish(context, &callFrameStack);

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astMessageChainNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astMessageChainNode_t **chainNode = (tuuvm_astMessageChainNode_t**)node;

    struct {
        tuuvm_tuple_t receiver;
        tuuvm_tuple_t receiverType;
        tuuvm_tuple_t analysisFunction;
        tuuvm_tuple_t chainedMessage;
        tuuvm_tuple_t result;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*chainNode)->super.sourcePosition);

    bool hasReceiver = false;
    if((*chainNode)->receiver)
    {
        gcFrame.receiver = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*chainNode)->receiver, *environment);
        hasReceiver = true;

        gcFrame.receiverType = tuuvm_tuple_getType(context, gcFrame.receiver);
        gcFrame.analysisFunction = tuuvm_type_getAnalyzeAndEvaluateMessageChainNodeForReceiverWithEnvironmentFunction(context, tuuvm_tuple_getType(context, gcFrame.receiverType));
        if(gcFrame.analysisFunction)
        {
            TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return tuuvm_function_apply4(context, gcFrame.analysisFunction, gcFrame.receiverType, *node, gcFrame.receiver, *environment);
        }
    }

    size_t chainedMessageCount = tuuvm_arraySlice_getSize((*chainNode)->messages);
    for(size_t i = 0; i < chainedMessageCount; ++i)
    {
        gcFrame.chainedMessage = tuuvm_arraySlice_at((*chainNode)->messages, i);
        gcFrame.result = tuuvm_astMessageChainMessageNode_analyzeAndEvaluate(context, gcFrame.chainedMessage, hasReceiver, &gcFrame.receiver, environment);
    }

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astMessageChainMessageNode_evaluate(tuuvm_context_t *context, tuuvm_tuple_t chainedMessage, bool hasReceiver, tuuvm_tuple_t *receiver, tuuvm_tuple_t *environment)
{
    struct {
        tuuvm_astMessageChainMessageNode_t *node;
        tuuvm_tuple_t selector;
        tuuvm_tuple_t method;
        tuuvm_tuple_t argumentNode;
        tuuvm_tuple_t argument;
        tuuvm_tuple_t result;
    } gcFrame = {
        .node = (tuuvm_astMessageChainMessageNode_t*)chainedMessage
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.node->super.sourcePosition);

    gcFrame.selector = tuuvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.node->selector, *environment);
    if(hasReceiver)
    {
        gcFrame.method = tuuvm_type_lookupSelector(context, tuuvm_tuple_getType(context, *receiver), gcFrame.selector);
        if(!gcFrame.method)
            tuuvm_error("TODO: fallback to doesNotUnderstand:");
    }
    else
    {
        if(!tuuvm_environment_lookSymbolRecursively(context, *environment, gcFrame.selector, &gcFrame.method))
            tuuvm_error("Failed to find symbol for message send without receiver.");
    }


    tuuvm_functionCallFrameStack_t callFrameStack = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    size_t applicationArgumentCount = tuuvm_arraySlice_getSize(gcFrame.node->arguments);
    tuuvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.method, applicationArgumentCount + (hasReceiver ? 1 : 0));

    if(hasReceiver)
        tuuvm_functionCallFrameStack_push(&callFrameStack, *receiver);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = tuuvm_arraySlice_at(gcFrame.node->arguments, i);
        gcFrame.argument = tuuvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.argumentNode, *environment);
        tuuvm_functionCallFrameStack_push(&callFrameStack, gcFrame.argument);
    }

    TUUVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    gcFrame.result = tuuvm_functionCallFrameStack_finish(context, &callFrameStack);

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astMessageChainNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astMessageChainNode_t **chainNode = (tuuvm_astMessageChainNode_t**)node;

    struct {
        tuuvm_tuple_t receiver;
        tuuvm_tuple_t chainedMessage;
        tuuvm_tuple_t result;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*chainNode)->super.sourcePosition);

    bool hasReceiver = false;
    if((*chainNode)->receiver)
    {
        gcFrame.receiver = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*chainNode)->receiver, *environment);
        hasReceiver = true;
    }

    size_t chainedMessageCount = tuuvm_arraySlice_getSize((*chainNode)->messages);
    for(size_t i = 0; i < chainedMessageCount; ++i)
    {
        gcFrame.chainedMessage = tuuvm_arraySlice_at((*chainNode)->messages, i);
        gcFrame.result = tuuvm_astMessageChainMessageNode_evaluate(context, gcFrame.chainedMessage, hasReceiver, &gcFrame.receiver, environment);
    }

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astMessageSendNode_primitiveMacro(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 4) tuuvm_error_argumentCountMismatch(4, argumentCount);

    tuuvm_tuple_t *macroContext = &arguments[0];
    tuuvm_tuple_t *selectorNode = &arguments[1];
    tuuvm_tuple_t *receiverNode = &arguments[2];
    tuuvm_tuple_t *argumentNodes = &arguments[3];

    tuuvm_tuple_t sourcePosition = tuuvm_macroContext_getSourcePosition(*macroContext);
    return tuuvm_astMessageSendNode_create(context, sourcePosition, *receiverNode, *selectorNode, *argumentNodes);
}

static tuuvm_tuple_t tuuvm_astMessageSendNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_astMessageSendNode_t *sendNode;
        tuuvm_tuple_t analyzedArguments;

        tuuvm_tuple_t receiverValue;
        tuuvm_tuple_t receiverType;
        tuuvm_tuple_t selector;
        tuuvm_tuple_t method;
        tuuvm_tuple_t analysisFunction;
        tuuvm_tuple_t result;
        tuuvm_tuple_t newMethodNode;
        tuuvm_tuple_t newArgumentNodes;
        tuuvm_tuple_t unexpandedApplicationNode;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sendNode = (tuuvm_astMessageSendNode_t*)tuuvm_context_shallowCopy(context, *node);
    if(gcFrame.sendNode->receiver)
    {
        gcFrame.sendNode->receiver = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.sendNode->receiver, *environment);
        if(tuuvm_astNode_isLiteralNode(context, gcFrame.sendNode->receiver))
        {
            // If the receiver is a literal node, we can attempt to forward the message send node analysis.
            gcFrame.receiverValue = tuuvm_astLiteralNode_getValue(gcFrame.sendNode->receiver);
            gcFrame.receiverType = tuuvm_tuple_getType(context, gcFrame.receiverValue);
            gcFrame.analysisFunction = tuuvm_type_getAnalyzeMessageSendNodeWithEnvironmentFunction(context, tuuvm_tuple_getType(context, gcFrame.receiverType));

            if(gcFrame.analysisFunction)
            {
                gcFrame.result = tuuvm_function_apply3(context, gcFrame.analysisFunction, gcFrame.receiverType, (tuuvm_tuple_t)gcFrame.sendNode, *environment);
                TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
                return gcFrame.result;
            }

            // If the selector is a literal, attempt to perform a static lookup.
            gcFrame.sendNode->selector = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.sendNode->selector, *environment);
            if(tuuvm_astNode_isLiteralNode(context, gcFrame.sendNode->selector))
            {
                gcFrame.selector = tuuvm_astLiteralNode_getValue(gcFrame.sendNode->selector);
                gcFrame.method = tuuvm_type_lookupMacroSelector(context, gcFrame.receiverType, gcFrame.selector);
                if(!gcFrame.method)
                    gcFrame.method = tuuvm_type_lookupSelector(context, gcFrame.receiverType, gcFrame.selector);
                if(!gcFrame.method)
                    gcFrame.method = tuuvm_type_lookupFallbackSelector(context, gcFrame.receiverType, gcFrame.selector);
                // Allow definining some macros at the any value type level.
                if(!gcFrame.method)
                    gcFrame.method = tuuvm_type_lookupMacroSelector(context, context->roots.anyValueType, gcFrame.selector);
            }
        }
        else
        {
            gcFrame.sendNode->selector = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.sendNode->selector, *environment);
            if(tuuvm_astNode_isLiteralNode(context, gcFrame.sendNode->selector))
            {
                gcFrame.selector = tuuvm_astLiteralNode_getValue(gcFrame.sendNode->selector);
                gcFrame.method = tuuvm_type_lookupMacroSelector(context, context->roots.anyValueType, gcFrame.selector);
            }
        }
    }
    else
    {
        gcFrame.sendNode->selector = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.sendNode->selector, *environment);
    }

    // Turn this node onto an unexpanded application.
    if(gcFrame.method && tuuvm_function_shouldOptimizeLookup(context, gcFrame.method, gcFrame.receiverType))
    {
        size_t applicationArgumentCount = tuuvm_arraySlice_getSize(gcFrame.sendNode->arguments);
        gcFrame.newMethodNode = tuuvm_astLiteralNode_create(context, gcFrame.sendNode->super.sourcePosition, gcFrame.method);

        gcFrame.newArgumentNodes = tuuvm_array_create(context, 1 + applicationArgumentCount);
        tuuvm_array_atPut(gcFrame.newArgumentNodes, 0, gcFrame.sendNode->receiver);
        for(size_t i = 0; i < applicationArgumentCount; ++i)
            tuuvm_array_atPut(gcFrame.newArgumentNodes, i + 1, tuuvm_arraySlice_at(gcFrame.sendNode->arguments, i));

        gcFrame.newArgumentNodes = tuuvm_array_asArraySlice(context, gcFrame.newArgumentNodes);
        gcFrame.unexpandedApplicationNode = tuuvm_astUnexpandedApplicationNode_create(context, gcFrame.sendNode->super.sourcePosition, gcFrame.newMethodNode, gcFrame.newArgumentNodes);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.unexpandedApplicationNode, *environment);
    }

    size_t applicationArgumentCount = tuuvm_arraySlice_getSize(gcFrame.sendNode->arguments);
    gcFrame.analyzedArguments = tuuvm_array_create(context, applicationArgumentCount);
    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        tuuvm_tuple_t argumentNode = tuuvm_arraySlice_at(gcFrame.sendNode->arguments, i);
        tuuvm_array_atPut(gcFrame.analyzedArguments, i, tuuvm_interpreter_analyzeASTWithEnvironment(context, argumentNode, *environment));
    }

    gcFrame.sendNode->arguments = tuuvm_array_asArraySlice(context, gcFrame.analyzedArguments);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.sendNode;
}

static tuuvm_tuple_t tuuvm_astMessageSendNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astMessageSendNode_t **sendNode = (tuuvm_astMessageSendNode_t**)node;

    struct {
        tuuvm_tuple_t receiver;
        tuuvm_tuple_t receiverType;
        tuuvm_tuple_t selector;
        tuuvm_tuple_t method;

        tuuvm_tuple_t analysisAndEvaluationFunction;
        tuuvm_tuple_t expansionResult;
        tuuvm_tuple_t receiverLiteralNode;
        tuuvm_tuple_t argumentNode;
        tuuvm_tuple_t argument;
        tuuvm_tuple_t result;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*sendNode)->super.sourcePosition);

    bool hasReceiver = false;
    if((*sendNode)->receiver)
    {
        gcFrame.receiver = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*sendNode)->receiver, *environment);
        hasReceiver = true;

        gcFrame.receiverType = tuuvm_tuple_getType(context, gcFrame.receiver);
        gcFrame.analysisAndEvaluationFunction = tuuvm_type_getAnalyzeAndEvaluateMessageSendNodeForReceiverWithEnvironmentFunction(context, tuuvm_tuple_getType(context, gcFrame.receiverType));
        if(gcFrame.analysisAndEvaluationFunction)
        {
            gcFrame.result = tuuvm_function_apply4(context, gcFrame.analysisAndEvaluationFunction, gcFrame.receiverType, *node, gcFrame.receiver, *environment);
            TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
            TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return gcFrame.result;
        }

        gcFrame.selector = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*sendNode)->selector, *environment);
        gcFrame.method = tuuvm_type_lookupMacroSelector(context, gcFrame.receiverType, gcFrame.selector);
        if(!gcFrame.method)
            gcFrame.method = tuuvm_type_lookupSelector(context, gcFrame.receiverType, gcFrame.selector);
        if(!gcFrame.method)
            gcFrame.method = tuuvm_type_lookupFallbackSelector(context, gcFrame.receiverType, gcFrame.selector);
        // Allow definining some macros at the any value type level.
        if(!gcFrame.method)
            gcFrame.method = tuuvm_type_lookupMacroSelector(context, context->roots.anyValueType, gcFrame.selector);
        if(!gcFrame.method)
        {
            fprintf(stderr, "doesNotUnderstand " TUUVM_STRING_PRINTF_FORMAT "\n", TUUVM_STRING_PRINTF_ARG(gcFrame.selector));
            tuuvm_error("TODO: fallback to doesNotUnderstand:");
        }
    }
    else
    {
        gcFrame.selector = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*sendNode)->selector, *environment);
        if(!tuuvm_environment_lookSymbolRecursively(context, *environment, gcFrame.selector, &gcFrame.method))
            tuuvm_error("Failed to find symbol for message send without receiver.");
    }

    bool isMacro = tuuvm_function_isMacro(context, gcFrame.method);
    tuuvm_functionCallFrameStack_t callFrameStack = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    if(isMacro)
    {
        size_t applicationArgumentCount = tuuvm_arraySlice_getSize((*sendNode)->arguments);
        tuuvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.method, 1 + applicationArgumentCount + (hasReceiver ? 1 : 0));

        tuuvm_functionCallFrameStack_push(&callFrameStack, tuuvm_macroContext_create(context, *node, (*sendNode)->super.sourcePosition, *environment));

        // We need to push the receiver as a node, so wrap it in a literal node here.
        if(hasReceiver)
        {
            gcFrame.receiverLiteralNode = tuuvm_astLiteralNode_create(context, tuuvm_astNode_getSourcePosition((*sendNode)->receiver), gcFrame.receiver);
            tuuvm_functionCallFrameStack_push(&callFrameStack, gcFrame.receiverLiteralNode);
        }

        // Push the argument nodes.
        for(size_t i = 0; i < applicationArgumentCount; ++i)
            tuuvm_functionCallFrameStack_push(&callFrameStack, tuuvm_arraySlice_at((*sendNode)->arguments, i));

        TUUVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
        gcFrame.expansionResult = tuuvm_functionCallFrameStack_finish(context, &callFrameStack);
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);

        // Analyze and evaluate the resulting node.
        return tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.expansionResult, *environment);
    }
    else
    {
        size_t applicationArgumentCount = tuuvm_arraySlice_getSize((*sendNode)->arguments);
        tuuvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.method, applicationArgumentCount + (hasReceiver ? 1 : 0));

        if(hasReceiver)
            tuuvm_functionCallFrameStack_push(&callFrameStack, gcFrame.receiver);

        for(size_t i = 0; i < applicationArgumentCount; ++i)
        {
            gcFrame.argumentNode = tuuvm_arraySlice_at((*sendNode)->arguments, i);
            gcFrame.argument = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.argumentNode, *environment);
            tuuvm_functionCallFrameStack_push(&callFrameStack, gcFrame.argument);
        }

        TUUVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
        gcFrame.result = tuuvm_functionCallFrameStack_finish(context, &callFrameStack);
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.result;
    }
}

static tuuvm_tuple_t tuuvm_astMessageSendNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astMessageSendNode_t **sendNode = (tuuvm_astMessageSendNode_t**)node;
    struct {
        tuuvm_tuple_t receiver;
        tuuvm_tuple_t selector;
        tuuvm_tuple_t method;
        tuuvm_tuple_t argumentNode;
        tuuvm_tuple_t argument;
        tuuvm_tuple_t result;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*sendNode)->super.sourcePosition);

    bool hasReceiver = false;
    if((*sendNode)->receiver)
    {
        gcFrame.receiver = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*sendNode)->receiver, *environment);
        hasReceiver = true;

        gcFrame.selector = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*sendNode)->selector, *environment);
        gcFrame.method = tuuvm_type_lookupSelector(context, tuuvm_tuple_getType(context, gcFrame.receiver), gcFrame.selector);
        if(!gcFrame.method)
        {
            fprintf(stderr, "doesNotUnderstand " TUUVM_STRING_PRINTF_FORMAT "\n", TUUVM_STRING_PRINTF_ARG(gcFrame.selector));
            tuuvm_error("TODO: fallback to doesNotUnderstand:");
        }
    }
    else
    {
        gcFrame.selector = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*sendNode)->selector, *environment);
        if(!tuuvm_environment_lookSymbolRecursively(context, *environment, gcFrame.selector, &gcFrame.method))
            tuuvm_error("Failed to find symbol for message send without receiver.");
    }


    tuuvm_functionCallFrameStack_t callFrameStack = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    size_t applicationArgumentCount = tuuvm_arraySlice_getSize((*sendNode)->arguments);
    tuuvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.method, applicationArgumentCount + (hasReceiver ? 1 : 0));

    if(hasReceiver)
        tuuvm_functionCallFrameStack_push(&callFrameStack, gcFrame.receiver);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = tuuvm_arraySlice_at((*sendNode)->arguments, i);
        gcFrame.argument = tuuvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.argumentNode, *environment);
        tuuvm_functionCallFrameStack_push(&callFrameStack, gcFrame.argument);
    }

    TUUVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    gcFrame.result = tuuvm_functionCallFrameStack_finish(context, &callFrameStack);
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astDoWhileContinueWithNode_primitiveMacro(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
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

static tuuvm_tuple_t tuuvm_astDoWhileContinueWithNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
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

static tuuvm_tuple_t tuuvm_astDoWhileContinueWithNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
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

static tuuvm_tuple_t tuuvm_astDoWhileContinueWithNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    return tuuvm_interpreter_evaluateASTWithEnvironment(context, tuuvm_interpreter_analyzeASTWithEnvironment(context, *node, *environment), *environment);
}

static tuuvm_tuple_t tuuvm_astWhileContinueWithNode_primitiveMacro(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
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

static tuuvm_tuple_t tuuvm_astWhileContinueWithNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
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

static tuuvm_tuple_t tuuvm_astWhileContinueWithNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
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

static tuuvm_tuple_t tuuvm_astWhileContinueWithNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    return tuuvm_interpreter_evaluateASTWithEnvironment(context, tuuvm_interpreter_analyzeASTWithEnvironment(context, *node, *environment), *environment);
}

static void tuuvm_interpreter_evaluateArgumentNodeInEnvironment(tuuvm_context_t *context, tuuvm_tuple_t argumentNode, tuuvm_tuple_t *activationEnvironment, tuuvm_tuple_t *argumentValue)
{
    struct {
        tuuvm_astArgumentNode_t *argumentNode;
        tuuvm_tuple_t name;
        tuuvm_tuple_t expectedType;
        tuuvm_tuple_t value;
    } gcFrame = {
        .argumentNode = (tuuvm_astArgumentNode_t*)argumentNode,
        .value = *argumentValue
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.argumentNode->super.sourcePosition);

    gcFrame.name = tuuvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.argumentNode->name, *activationEnvironment);
    if(gcFrame.argumentNode->type)
    {
        gcFrame.expectedType = tuuvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.argumentNode->type, *activationEnvironment);
        if(gcFrame.expectedType)
            gcFrame.value = tuuvm_type_coerceValue(context, gcFrame.expectedType, gcFrame.value);
    }

    tuuvm_environment_setNewSymbolBinding(context, *activationEnvironment, gcFrame.name, gcFrame.value);

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
}

static tuuvm_tuple_t tuuvm_interpreter_evaluateResultTypeCoercionInEnvironment(tuuvm_context_t *context, tuuvm_function_t **closureASTFunction, tuuvm_tuple_t *environment, tuuvm_tuple_t result)
{
    if(!(*closureASTFunction)->resultTypeNode)
        return result;

    struct {
        tuuvm_tuple_t resultType;
        tuuvm_tuple_t result;
    } gcFrame = {
        .result = result
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.resultType = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*closureASTFunction)->resultTypeNode, *environment);
    if(gcFrame.resultType)
        gcFrame.result = tuuvm_type_coerceValue(context, gcFrame.resultType, gcFrame.result);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

TUUVM_API tuuvm_tuple_t tuuvm_interpreter_applyClosureASTFunction(tuuvm_context_t *context, tuuvm_tuple_t *function, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    tuuvm_stackFrameFunctionActivationRecord_t functionActivationRecord = {
        .type = TUUVM_STACK_FRAME_RECORD_TYPE_FUNCTION_ACTIVATION,
        .function = *function,
    };
    tuuvm_stackFrame_pushRecord((tuuvm_stackFrameRecord_t*)&functionActivationRecord);  
    tuuvm_function_t **closureASTFunction = (tuuvm_function_t**)&functionActivationRecord.function;

    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*closureASTFunction)->sourcePosition);

    size_t expectedArgumentCount = tuuvm_arraySlice_getSize((*closureASTFunction)->argumentNodes);
    if(argumentCount != expectedArgumentCount)
        tuuvm_error_argumentCountMismatch(expectedArgumentCount, argumentCount);
    functionActivationRecord.applicationEnvironment = tuuvm_environment_create(context, (*closureASTFunction)->closureEnvironment);

    for(size_t i = 0; i < argumentCount; ++i)
        tuuvm_interpreter_evaluateArgumentNodeInEnvironment(context, tuuvm_arraySlice_at((*closureASTFunction)->argumentNodes, i), &functionActivationRecord.applicationEnvironment, &arguments[i]);

    // Use setjmp for implementing the #return: statement.
    if(!setjmp(functionActivationRecord.jmpbuffer))
    {
        tuuvm_gc_safepoint(context);
        functionActivationRecord.result = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*closureASTFunction)->body, functionActivationRecord.applicationEnvironment);
    }
    
    functionActivationRecord.result = tuuvm_interpreter_evaluateResultTypeCoercionInEnvironment(context, closureASTFunction, &functionActivationRecord.applicationEnvironment, functionActivationRecord.result);
    
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    tuuvm_stackFrame_popRecord((tuuvm_stackFrameRecord_t*)&functionActivationRecord);  
    return functionActivationRecord.result;
}

void tuuvm_astInterpreter_setupASTInterpreter(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "begin", 2, TUUVM_FUNCTION_FLAGS_MACRO | TUUVM_FUNCTION_FLAGS_VARIADIC, NULL, tuuvm_astSequenceNode_primitiveMacro);

    tuuvm_type_setAstNodeAnalysisFunction(context, context->roots.astArgumentNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astArgumentNode_primitiveAnalyze));

    tuuvm_type_setAstNodeAnalysisFunction(context, context->roots.astSequenceNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astSequenceNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context, context->roots.astSequenceNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astSequenceNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context, context->roots.astSequenceNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astSequenceNode_primitiveAnalyzeAndEvaluate));

    tuuvm_type_setAstNodeAnalysisFunction(context, context->roots.astLiteralNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astLiteralNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context, context->roots.astLiteralNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astLiteralNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context, context->roots.astLiteralNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astLiteralNode_primitiveEvaluate));

    tuuvm_type_setAstNodeAnalysisFunction(context, context->roots.astIdentifierReferenceNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astIdentifierReferenceNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context, context->roots.astIdentifierReferenceNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astIdentifierReferenceNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context, context->roots.astIdentifierReferenceNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astIdentifierReferenceNode_primitiveEvaluate));

    tuuvm_type_setAstNodeAnalysisFunction(context, context->roots.astUnexpandedApplicationNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astUnexpandedApplicationNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context, context->roots.astUnexpandedApplicationNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astUnexpandedApplicationNode_primitiveAnalyzeAndEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context, context->roots.astUnexpandedApplicationNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astUnexpandedApplicationNode_primitiveAnalyzeAndEvaluate));

    tuuvm_type_setAstNodeAnalysisFunction(context, context->roots.astUnexpandedSExpressionNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astUnexpandedSExpressionNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context, context->roots.astUnexpandedSExpressionNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astUnexpandedSExpressionNode_primitiveAnalyzeAndEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context, context->roots.astUnexpandedSExpressionNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astUnexpandedSExpressionNode_primitiveAnalyzeAndEvaluate));

    tuuvm_type_setAstNodeAnalysisFunction(context, context->roots.astFunctionApplicationNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astFunctionApplicationNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context, context->roots.astFunctionApplicationNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astFunctionApplicationNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context, context->roots.astFunctionApplicationNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astFunctionApplicationNode_primitiveAnalyzeAndEvaluate));

    tuuvm_type_setAstNodeAnalysisFunction(context, context->roots.astLexicalBlockNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astLexicalBlockNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context, context->roots.astLexicalBlockNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astLexicalBlockNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context, context->roots.astLexicalBlockNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astLexicalBlockNode_primitiveAnalyzeAndEvaluate));

    tuuvm_type_setAstNodeAnalysisFunction(context, context->roots.astMakeByteArrayNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astMakeByteArrayNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context, context->roots.astMakeByteArrayNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astMakeByteArrayNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context, context->roots.astMakeByteArrayNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astMakeByteArrayNode_primitiveAnalyzeAndEvaluate));

    tuuvm_type_setAstNodeAnalysisFunction(context, context->roots.astMakeTupleNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astMakeTupleNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context, context->roots.astMakeTupleNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astMakeTupleNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context, context->roots.astMakeTupleNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astMakeTupleNode_primitiveAnalyzeAndEvaluate));

    tuuvm_type_setAstNodeAnalysisFunction(context, context->roots.astMessageChainMessageNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astMessageChainMessageNode_primitiveAnalyze));

    tuuvm_type_setAstNodeAnalysisFunction(context, context->roots.astMessageChainNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astMessageChainNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context, context->roots.astMessageChainNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astMessageChainNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context, context->roots.astMessageChainNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astMessageChainNode_primitiveAnalyzeAndEvaluate));

    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "send", 4, TUUVM_FUNCTION_FLAGS_MACRO | TUUVM_FUNCTION_FLAGS_VARIADIC, NULL, tuuvm_astMessageSendNode_primitiveMacro);
    tuuvm_type_setAstNodeAnalysisFunction(context, context->roots.astMessageSendNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astMessageSendNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context, context->roots.astMessageSendNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astMessageSendNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context, context->roots.astMessageSendNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astMessageSendNode_primitiveAnalyzeAndEvaluate));

    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "lambda", 3, TUUVM_FUNCTION_FLAGS_MACRO | TUUVM_FUNCTION_FLAGS_VARIADIC, NULL, tuuvm_astLambdaNode_primitiveMacro);
    tuuvm_type_setAstNodeAnalysisFunction(context, context->roots.astLambdaNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astLambdaNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context, context->roots.astLambdaNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astLambdaNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context, context->roots.astLambdaNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astLambdaNode_primitiveAnalyzeAndEvaluate));

    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "define", 3, TUUVM_FUNCTION_FLAGS_MACRO | TUUVM_FUNCTION_FLAGS_VARIADIC, NULL, tuuvm_astLocalDefinitionNode_primitiveMacro);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "defineMacro", 3, TUUVM_FUNCTION_FLAGS_MACRO | TUUVM_FUNCTION_FLAGS_VARIADIC, NULL, tuuvm_astLocalDefinitionNode_primitiveDefineMacro);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "let:with:", 3, TUUVM_FUNCTION_FLAGS_MACRO, NULL, tuuvm_astLocalDefinitionNode_letWithPrimitiveMacro);
    tuuvm_type_setAstNodeAnalysisFunction(context, context->roots.astLocalDefinitionNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astLocalDefinitionNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context, context->roots.astLocalDefinitionNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astLocalDefinitionNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context, context->roots.astLocalDefinitionNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astLocalDefinitionNode_primitiveAnalyzeAndEvaluate));

    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "if:then:else:", 4, TUUVM_FUNCTION_FLAGS_MACRO, NULL, tuuvm_astIfNode_primitiveMacro);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "if:then:", 3, TUUVM_FUNCTION_FLAGS_MACRO, NULL, tuuvm_astIfNode_primitiveMacroIfThen);
    tuuvm_type_setAstNodeAnalysisFunction(context, context->roots.astIfNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astIfNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context, context->roots.astIfNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astIfNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context, context->roots.astIfNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astIfNode_primitiveAnalyzeAndEvaluate));

    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "do:while:do:continueWith:", 4, TUUVM_FUNCTION_FLAGS_MACRO, NULL, tuuvm_astDoWhileContinueWithNode_primitiveMacro);
    tuuvm_type_setAstNodeAnalysisFunction(context, context->roots.astDoWhileContinueWithNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astDoWhileContinueWithNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context, context->roots.astDoWhileContinueWithNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astDoWhileContinueWithNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context, context->roots.astDoWhileContinueWithNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astDoWhileContinueWithNode_primitiveAnalyzeAndEvaluate));

    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "while:do:continueWith:", 4, TUUVM_FUNCTION_FLAGS_MACRO, NULL, tuuvm_astWhileContinueWithNode_primitiveMacro);
    tuuvm_type_setAstNodeAnalysisFunction(context, context->roots.astWhileContinueWithNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astWhileContinueWithNode_primitiveAnalyze));
    tuuvm_type_setAstNodeEvaluationFunction(context, context->roots.astWhileContinueWithNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astWhileContinueWithNode_primitiveEvaluate));
    tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context, context->roots.astWhileContinueWithNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astWhileContinueWithNode_primitiveAnalyzeAndEvaluate));
}
