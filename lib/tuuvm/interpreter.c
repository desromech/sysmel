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
#include "tuuvm/filesystem.h"
#include "tuuvm/io.h"
#include "tuuvm/macro.h"
#include "tuuvm/message.h"
#include "tuuvm/parser.h"
#include "tuuvm/string.h"
#include "tuuvm/sourceCode.h"
#include "tuuvm/stackFrame.h"
#include "tuuvm/sysmelParser.h"
#include "tuuvm/type.h"
#include "internal/context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define tuuvm_gc_safepoint(x) false

static void tuuvm_functionDefinition_ensureAnalysis(tuuvm_context_t *context, tuuvm_functionDefinition_t **functionDefinition);

TUUVM_API tuuvm_tuple_t tuuvm_interpreter_analyzeASTWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t astNode, tuuvm_tuple_t environment)
{
    struct {
        tuuvm_tuple_t function;
    } gcFrame = {0};

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
    } gcFrame = {0};

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

TUUVM_API tuuvm_tuple_t tuuvm_interpreter_analyzeAndEvaluateStringWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t sourceCodeText, tuuvm_tuple_t sourceCodeDirectory, tuuvm_tuple_t sourceCodeName, tuuvm_tuple_t sourceCodeLanguage)
{
    struct {
        tuuvm_tuple_t environment;
        tuuvm_tuple_t astNode;
        tuuvm_tuple_t result;
    } gcFrame = {
        .environment = environment,
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.astNode = tuuvm_sourceCode_create(context, sourceCodeText, sourceCodeDirectory, sourceCodeName, sourceCodeLanguage);
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
    tuuvm_tuple_t pragmas = tuuvm_array_create(context, 0);
    return tuuvm_astSequenceNode_create(context, sourcePosition, pragmas, tuuvm_arraySlice_asArray(context, *bodyNodes));
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

        tuuvm_tuple_t pragmas;
        tuuvm_tuple_t analyzedPragmas;
        tuuvm_tuple_t pragma;
        tuuvm_tuple_t analyzedPragma;

        tuuvm_tuple_t expressions;
        tuuvm_tuple_t analyzedExpressions;
        tuuvm_tuple_t expression;
        tuuvm_tuple_t analyzedExpression;

        tuuvm_tuple_t elementValue;
        tuuvm_tuple_t elementType;
        tuuvm_tuple_t elementMetaType;
        tuuvm_tuple_t resultType;
        tuuvm_tuple_t concretizeFunction;
    } gcFrame = {0};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*sequenceNode)->super.sourcePosition);

    gcFrame.pragmas = (*sequenceNode)->pragmas;
    gcFrame.expressions = (*sequenceNode)->expressions;
    size_t pragmaCount = tuuvm_array_getSize(gcFrame.pragmas);
    size_t expressionCount = tuuvm_array_getSize(gcFrame.expressions);
    if(pragmaCount == 0 && expressionCount == 0)
    {
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return *node;
    }

    gcFrame.analyzedSequenceNode = (tuuvm_astSequenceNode_t *)tuuvm_context_shallowCopy(context, *node);

    gcFrame.analyzedPragmas = tuuvm_array_create(context, pragmaCount);
    for(size_t i = 0; i < pragmaCount; ++i)
    {
        gcFrame.pragma = tuuvm_array_at(gcFrame.pragmas, i);
        gcFrame.analyzedPragma = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.pragma, *environment);
        tuuvm_array_atPut(gcFrame.analyzedPragmas, i, gcFrame.analyzedPragma);
    }

    gcFrame.analyzedSequenceNode->pragmas = gcFrame.analyzedPragmas;

    gcFrame.analyzedExpressions = tuuvm_array_create(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expression = tuuvm_array_at(gcFrame.expressions, i);
        gcFrame.analyzedExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.expression, *environment);

        if(tuuvm_astNode_isLiteralNode(context, gcFrame.analyzedExpression))
        {
            gcFrame.elementValue = tuuvm_astLiteralNode_getValue(gcFrame.analyzedExpression);
            gcFrame.elementType = tuuvm_tuple_getType(context, gcFrame.elementValue);
            gcFrame.elementMetaType = tuuvm_tuple_getType(context, gcFrame.elementType);
            gcFrame.concretizeFunction = tuuvm_type_getAnalyzeConcreteMetaValueWithEnvironmentFunction(context, gcFrame.elementMetaType);
            if(gcFrame.concretizeFunction)
                gcFrame.analyzedExpression = tuuvm_function_apply3(context, gcFrame.concretizeFunction, gcFrame.elementType, gcFrame.analyzedExpression, *environment);
        }

        gcFrame.resultType = tuuvm_astNode_getAnalyzedType(gcFrame.analyzedExpression);
        tuuvm_array_atPut(gcFrame.analyzedExpressions, i, gcFrame.analyzedExpression);
    }

    gcFrame.analyzedSequenceNode->expressions = gcFrame.analyzedExpressions;
    gcFrame.analyzedSequenceNode->super.analyzedType = gcFrame.resultType;
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);

    if(pragmaCount == 0 && expressionCount == 1)
        return tuuvm_array_at(gcFrame.analyzedExpressions, 0);

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

    size_t expressionCount = tuuvm_array_getSize((*sequenceNode)->expressions);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expression = tuuvm_array_at((*sequenceNode)->expressions, i);
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
        tuuvm_tuple_t elementMetaType;
        tuuvm_tuple_t concretizeFunction;
    } gcFrame = {
        .result = TUUVM_VOID_TUPLE
    };

    tuuvm_astSequenceNode_t **sequenceNode = (tuuvm_astSequenceNode_t**)node;

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*sequenceNode)->super.sourcePosition);

    size_t expressionCount = tuuvm_array_getSize((*sequenceNode)->expressions);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expression = tuuvm_array_at((*sequenceNode)->expressions, i);
        gcFrame.result = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.expression, *environment);
        
        gcFrame.elementType = tuuvm_tuple_getType(context, gcFrame.result);
        gcFrame.elementMetaType = tuuvm_tuple_getType(context, gcFrame.elementType);
        gcFrame.concretizeFunction = tuuvm_type_getAnalyzeAndEvaluateConcreteMetaValueWithEnvironmentFunction(context, gcFrame.elementMetaType);
        if(gcFrame.concretizeFunction)
            gcFrame.result = tuuvm_function_apply3(context, gcFrame.concretizeFunction, gcFrame.elementType, gcFrame.result, *environment);
    }

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
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
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.argumentsNode = unsafeArgumentsNode;
    gcFrame.argumentList = tuuvm_arrayList_create(context);
    size_t argumentNodeCount = tuuvm_array_getSize(gcFrame.argumentsNode);
    *hasVariadicArguments = false;
    for(size_t i = 0; i < argumentNodeCount; ++i)
    {
        gcFrame.unparsedArgumentNode = tuuvm_array_at(gcFrame.argumentsNode, i);
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

    tuuvm_tuple_t result = tuuvm_arrayList_asArray(context, gcFrame.argumentList);
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
        tuuvm_tuple_t pragmas;
        tuuvm_tuple_t bodyNodes;
        tuuvm_tuple_t bodySequence;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    if(!tuuvm_astNode_isUnexpandedSExpressionNode(context, *argumentsSExpressionNode))
        tuuvm_error("Expected a S-Expression with the arguments node.");

    bool hasVariadicArguments = false;
    gcFrame.argumentsNode = tuuvm_astUnexpandedSExpressionNode_getElements(*argumentsSExpressionNode);
    gcFrame.sourcePosition = tuuvm_macroContext_getSourcePosition(*macroContext);
    gcFrame.argumentsArraySlice = tuuvm_astLambdaNode_parseArgumentsNodes(context, gcFrame.argumentsNode, &hasVariadicArguments);
    gcFrame.pragmas = tuuvm_array_create(context, 0);
    gcFrame.bodyNodes = tuuvm_arraySlice_asArray(context, *bodyNodes);
    gcFrame.bodySequence = tuuvm_astSequenceNode_create(context, gcFrame.sourcePosition, gcFrame.pragmas, gcFrame.bodyNodes);
    tuuvm_tuple_t result = tuuvm_astLambdaNode_create(context, gcFrame.sourcePosition,
        tuuvm_tuple_size_encode(context, hasVariadicArguments ? TUUVM_FUNCTION_FLAGS_VARIADIC : TUUVM_FUNCTION_FLAGS_NONE),
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
        tuuvm_tuple_t analyzedName;
        tuuvm_tuple_t analyzedType;
        tuuvm_tuple_t evaluatedName;
        tuuvm_tuple_t evaluatedType;
        tuuvm_tuple_t argumentBinding;

    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.argumentNode = (tuuvm_astArgumentNode_t*)tuuvm_context_shallowCopy(context, *node);

    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.argumentNode->super.sourcePosition);

    if(gcFrame.argumentNode->name)
    {
        gcFrame.analyzedName = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.argumentNode->name, *environment);
        gcFrame.argumentNode->name = gcFrame.analyzedName;

        if(tuuvm_astNode_isLiteralNode(context, gcFrame.analyzedName))
            gcFrame.evaluatedName = tuuvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.analyzedName, *environment);
    }
    if(gcFrame.argumentNode->type)
    {
        gcFrame.analyzedType = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.argumentNode->type, *environment);
        gcFrame.argumentNode->type = gcFrame.analyzedType;
        if(tuuvm_astNode_isLiteralNode(context, gcFrame.analyzedType))
            gcFrame.evaluatedType = tuuvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.analyzedType, *environment);
    }

    if(gcFrame.evaluatedName)
    {
        gcFrame.argumentBinding = tuuvm_analysisEnvironment_setNewSymbolArgumentBinding(context, *environment, gcFrame.argumentNode->super.sourcePosition, gcFrame.evaluatedName, gcFrame.evaluatedType);
        gcFrame.argumentNode->binding = gcFrame.argumentBinding;
    }

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.argumentNode;
}

static tuuvm_tuple_t tuuvm_astErrorNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    //tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astErrorNode_t **errorNode = (tuuvm_astErrorNode_t**)node;
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*errorNode)->super.sourcePosition);

    tuuvm_errorWithMessageTuple((*errorNode)->errorMessage);
    return TUUVM_NULL_TUPLE;
}

tuuvm_tuple_t tuuvm_interpreter_recompileAndOptimizeFunction(tuuvm_context_t *context, tuuvm_function_t **functionObject)
{
    //return (tuuvm_tuple_t)*functionObject;
    struct {
        tuuvm_functionDefinition_t *functionDefinition;
        tuuvm_tuple_t optimizedFunction;
        tuuvm_tuple_t optimizedDefinitionEnvironment;
        tuuvm_functionDefinition_t *optimizedFunctionDefinition;
        tuuvm_tuple_t captureValue;
        tuuvm_tuple_t captureBinding;
    } gcFrame = {
        .functionDefinition = (tuuvm_functionDefinition_t *)(*functionObject)->definition,
    };

    // If the function is not yet optimized, just return it back.
    if(!gcFrame.functionDefinition || !gcFrame.functionDefinition->analysisEnvironment)
        return (tuuvm_tuple_t)*functionObject;

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.optimizedFunctionDefinition = (tuuvm_functionDefinition_t*)tuuvm_context_shallowCopy(context, (tuuvm_tuple_t)gcFrame.functionDefinition);

    // Construct the closure environment by reading the capture vector.
    gcFrame.optimizedDefinitionEnvironment = tuuvm_environment_create(context, gcFrame.optimizedFunctionDefinition->definitionEnvironment);
    gcFrame.optimizedFunctionDefinition->definitionEnvironment = gcFrame.optimizedDefinitionEnvironment;

    size_t captureCount = tuuvm_array_getSize((*functionObject)->captureVector);
    for(size_t i = 0; i < captureCount; ++i)
    {
        gcFrame.captureValue = tuuvm_array_at((*functionObject)->captureVector, i);
        gcFrame.captureBinding = tuuvm_array_at(gcFrame.functionDefinition->analyzedCaptures, i);
        tuuvm_environment_setNewSymbolBindingWithValue(context, gcFrame.optimizedDefinitionEnvironment, tuuvm_symbolBinding_getName(gcFrame.captureBinding), gcFrame.captureValue);
    }

    gcFrame.optimizedFunctionDefinition->analysisEnvironment = TUUVM_NULL_TUPLE;
    gcFrame.optimizedFunctionDefinition->analyzedCaptures = TUUVM_NULL_TUPLE;
    gcFrame.optimizedFunctionDefinition->analyzedArguments = TUUVM_NULL_TUPLE;
    gcFrame.optimizedFunctionDefinition->analyzedLocals = TUUVM_NULL_TUPLE;

    gcFrame.optimizedFunctionDefinition->analyzedArgumentNodes = TUUVM_NULL_TUPLE;
    gcFrame.optimizedFunctionDefinition->analyzedBodyNode = TUUVM_NULL_TUPLE;
    gcFrame.optimizedFunctionDefinition->analyzedResultTypeNode = TUUVM_NULL_TUPLE;

    tuuvm_functionDefinition_ensureAnalysis(context, &gcFrame.optimizedFunctionDefinition);
    TUUVM_ASSERT(tuuvm_array_getSize(gcFrame.optimizedFunctionDefinition->analyzedCaptures) == 0);
    gcFrame.optimizedFunction = tuuvm_function_createClosureWithCaptureVector(context, (tuuvm_tuple_t)gcFrame.optimizedFunctionDefinition, tuuvm_array_create(context, 0));
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.optimizedFunction;
}

static void tuuvm_functionDefinition_analyze(tuuvm_context_t *context, tuuvm_functionDefinition_t **functionDefinition)
{
    struct {
        tuuvm_tuple_t analysisEnvironment;
        tuuvm_functionAnalysisEnvironment_t *analysisEnvironmentObject;
        tuuvm_tuple_t analyzedArgumentNode;
        tuuvm_tuple_t analyzedArgumentsNode;
        tuuvm_tuple_t analyzedBodyNode;
        tuuvm_tuple_t analyzedResultTypeNode;
    } gcFrame = {0};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*functionDefinition)->sourcePosition);

    gcFrame.analysisEnvironment = tuuvm_functionAnalysisEnvironment_create(context, (*functionDefinition)->definitionEnvironment, (tuuvm_tuple_t)*functionDefinition);
    gcFrame.analysisEnvironmentObject = (tuuvm_functionAnalysisEnvironment_t*)gcFrame.analysisEnvironment;
    size_t argumentNodeCount = tuuvm_array_getSize((*functionDefinition)->definitionArgumentNodes);
    gcFrame.analyzedArgumentsNode = tuuvm_array_create(context, argumentNodeCount);
    for(size_t i = 0; i < argumentNodeCount; ++i)
    {
        gcFrame.analyzedArgumentNode = tuuvm_array_at((*functionDefinition)->definitionArgumentNodes, i);
        gcFrame.analyzedArgumentNode = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.analyzedArgumentNode, gcFrame.analysisEnvironment);
        tuuvm_array_atPut(gcFrame.analyzedArgumentsNode, i, gcFrame.analyzedArgumentNode);
    }

    if((*functionDefinition)->definitionResultTypeNode)
        gcFrame.analyzedResultTypeNode = tuuvm_interpreter_analyzeASTWithEnvironment(context, (*functionDefinition)->definitionResultTypeNode, gcFrame.analysisEnvironment);

    gcFrame.analyzedBodyNode = tuuvm_interpreter_analyzeASTWithEnvironment(context, (*functionDefinition)->definitionBodyNode, gcFrame.analysisEnvironment);

    (*functionDefinition)->analysisEnvironment = gcFrame.analysisEnvironment;
    (*functionDefinition)->analyzedCaptures = tuuvm_arrayList_asArray(context, gcFrame.analysisEnvironmentObject->captureBindingList);
    (*functionDefinition)->analyzedArguments = tuuvm_arrayList_asArray(context, gcFrame.analysisEnvironmentObject->argumentBindingList);
    (*functionDefinition)->analyzedLocals = tuuvm_arrayList_asArray(context, gcFrame.analysisEnvironmentObject->localBindingList);

    (*functionDefinition)->analyzedArgumentNodes = gcFrame.analyzedArgumentsNode;
    (*functionDefinition)->analyzedResultTypeNode = gcFrame.analyzedResultTypeNode;
    (*functionDefinition)->analyzedBodyNode = gcFrame.analyzedBodyNode;

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
}

static void tuuvm_functionDefinition_ensureAnalysis(tuuvm_context_t *context, tuuvm_functionDefinition_t **functionDefinition)
{
    // Make sure this is a valid function definition for analysis.
    if(!tuuvm_tuple_isKindOf(context, (tuuvm_tuple_t)*functionDefinition, context->roots.functionDefinitionType) || !(*functionDefinition)->definitionArgumentNodes || !(*functionDefinition)->definitionBodyNode)
        return;

    // Is it already analyzed?
    if((*functionDefinition)->analysisEnvironment)
        return;

    tuuvm_functionDefinition_analyze(context, functionDefinition);
}

static tuuvm_tuple_t tuuvm_functionDefinition_primitiveEnsureAnalysis(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_functionDefinition_t **functionDefinition = (tuuvm_functionDefinition_t **)&arguments[0];
    tuuvm_functionDefinition_ensureAnalysis(context, functionDefinition);
    return TUUVM_NULL_TUPLE;
}

static tuuvm_tuple_t tuuvm_astLambdaNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_astLambdaNode_t *lambdaNode;
        tuuvm_tuple_t argumentNode;
        tuuvm_tuple_t argumentCount;
        tuuvm_functionDefinition_t *functionDefinition;
        tuuvm_tuple_t capturelessFunction;
        tuuvm_tuple_t capturelessLiteral;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.lambdaNode = (tuuvm_astLambdaNode_t*)tuuvm_context_shallowCopy(context, *node);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.lambdaNode->super.sourcePosition);

    // Count the actual argument count.
    size_t lambdaArgumentCount = 0;
    size_t argumentNodeCount = tuuvm_array_getSize(gcFrame.lambdaNode->arguments);
    for(size_t i = 0; i < argumentNodeCount; ++i)
    {
        gcFrame.argumentNode = tuuvm_array_at(gcFrame.lambdaNode->arguments, i);
        if(!tuuvm_astArgumentNode_isForAll(gcFrame.argumentNode))
            ++lambdaArgumentCount;
    }

    gcFrame.argumentCount = tuuvm_tuple_size_encode(context, lambdaArgumentCount);
    gcFrame.functionDefinition = (tuuvm_functionDefinition_t *)tuuvm_functionDefinition_create(context,
        gcFrame.lambdaNode->super.sourcePosition, gcFrame.lambdaNode->flags,
        gcFrame.argumentCount, *environment,
        gcFrame.lambdaNode->arguments, gcFrame.lambdaNode->resultType, gcFrame.lambdaNode->body
    );
    gcFrame.lambdaNode->functionDefinition = (tuuvm_tuple_t)gcFrame.functionDefinition;

    // Perform the lambda analysis.
    tuuvm_functionDefinition_ensureAnalysis(context, &gcFrame.functionDefinition);

    // Optimize lambdas without captures by turning them onto a literal.
    if(tuuvm_array_getSize(gcFrame.functionDefinition->analyzedCaptures) == 0)
    {
        gcFrame.capturelessFunction = tuuvm_function_createClosureWithCaptureVector(context, (tuuvm_tuple_t)gcFrame.functionDefinition, tuuvm_array_create(context, 0));
        gcFrame.capturelessLiteral = tuuvm_astLiteralNode_create(context, gcFrame.lambdaNode->super.sourcePosition, gcFrame.capturelessFunction);
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.capturelessLiteral;
    }

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
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

    tuuvm_functionDefinition_t *functionDefinition = (tuuvm_functionDefinition_t*)(*lambdaNode)->functionDefinition;
    size_t captureVectorSize = tuuvm_array_getSize(functionDefinition->analyzedCaptures);
    tuuvm_tuple_t captureVector = tuuvm_array_create(context, captureVectorSize);
    for(size_t i = 0; i < captureVectorSize; ++i)
    {
        tuuvm_tuple_t captureBinding = tuuvm_symbolCaptureBinding_getSourceBinding(tuuvm_array_at(functionDefinition->analyzedCaptures, i));
        tuuvm_tuple_t captureValue = tuuvm_environment_evaluateSymbolBinding(context, *environment, captureBinding);
        tuuvm_array_atPut(captureVector, i, captureValue);
    }

    return tuuvm_function_createClosureWithCaptureVector(context, (*lambdaNode)->functionDefinition, captureVector);
}

static tuuvm_tuple_t tuuvm_astLambdaNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_tuple_t argumentNode;
        tuuvm_tuple_t argumentCount;
        tuuvm_functionDefinition_t* functionDefinition;
        
        tuuvm_tuple_t captureVector;
        tuuvm_tuple_t captureBinding;
        tuuvm_tuple_t captureValue;
        tuuvm_tuple_t closure;
    } gcFrame = {0};

    tuuvm_astLambdaNode_t **lambdaNode = (tuuvm_astLambdaNode_t**)node;
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*lambdaNode)->super.sourcePosition);

    // Count the actual argument count.
    size_t lambdaArgumentCount = 0;
    size_t argumentNodeCount = tuuvm_array_getSize((*lambdaNode)->arguments);
    for(size_t i = 0; i < argumentNodeCount; ++i)
    {
        gcFrame.argumentNode = tuuvm_array_at((*lambdaNode)->arguments, i);
        if(!tuuvm_astArgumentNode_isForAll(gcFrame.argumentNode))
            ++lambdaArgumentCount;
    }

    gcFrame.argumentCount = tuuvm_tuple_size_encode(context,  lambdaArgumentCount);
    gcFrame.functionDefinition = (tuuvm_functionDefinition_t*)tuuvm_functionDefinition_create(context,
        (*lambdaNode)->super.sourcePosition, (*lambdaNode)->flags,
        gcFrame.argumentCount, *environment,
        (*lambdaNode)->arguments, (*lambdaNode)->resultType, (*lambdaNode)->body
    );
    
    if(!tuuvm_tuple_boolean_decode((*lambdaNode)->hasLazyAnalysis))
    {
        tuuvm_functionDefinition_ensureAnalysis(context, &gcFrame.functionDefinition);

        size_t captureVectorSize = tuuvm_array_getSize(gcFrame.functionDefinition->analyzedCaptures);
        gcFrame.captureVector = tuuvm_array_create(context, captureVectorSize);
        for(size_t i = 0; i < captureVectorSize; ++i)
        {
            gcFrame.captureBinding = tuuvm_symbolCaptureBinding_getSourceBinding(tuuvm_array_at(gcFrame.functionDefinition->analyzedCaptures, i));
            gcFrame.captureValue = tuuvm_environment_evaluateSymbolBinding(context, *environment, gcFrame.captureBinding);
            tuuvm_array_atPut(gcFrame.captureVector, i, gcFrame.captureValue);
        }
        gcFrame.closure = tuuvm_function_createClosureWithCaptureVector(context, (tuuvm_tuple_t)gcFrame.functionDefinition, gcFrame.captureVector);
    }
    else
    {
        // TODO: Implement this case properly
        abort();
    }

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.closure;
}

static tuuvm_tuple_t tuuvm_astLiteralNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_astLiteralNode_t *literalNode = (tuuvm_astLiteralNode_t *)arguments[0];
    if(literalNode->super.analyzedType)
        return (tuuvm_tuple_t)literalNode;

    tuuvm_astLiteralNode_t *analyzedNode = (tuuvm_astLiteralNode_t*)tuuvm_context_shallowCopy(context, (tuuvm_tuple_t)literalNode);
    analyzedNode->super.analyzedType = tuuvm_tuple_getType(context, analyzedNode->value);
    return (tuuvm_tuple_t)analyzedNode;
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
        tuuvm_tuple_t pragmas;
        tuuvm_tuple_t bodyNodes;
        tuuvm_tuple_t bodySequence;
        tuuvm_tuple_t nameExpression;
        tuuvm_tuple_t result;
    } gcFrame = {0};
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
        if(tuuvm_array_getSize(gcFrame.lambdaSignatureElements) < 1)
            tuuvm_error("Expected function definition requires a name.");

        gcFrame.nameNode = tuuvm_array_at(gcFrame.lambdaSignatureElements, 0);
        if(!tuuvm_astNode_isIdentifierReferenceNode(context, gcFrame.nameNode))
            tuuvm_error("Expected an identifier reference node for the name.");

        bool hasVariadicArguments = false;
        gcFrame.argumentsNode = tuuvm_array_fromOffset(context, gcFrame.lambdaSignatureElements, 1);
        gcFrame.arguments = tuuvm_astLambdaNode_parseArgumentsNodes(context, gcFrame.argumentsNode, &hasVariadicArguments);
        gcFrame.pragmas = tuuvm_array_create(context, 0);
        gcFrame.bodyNodes = tuuvm_arraySlice_asArray(context, *valueOrBodyNodes);
        gcFrame.bodySequence = tuuvm_astSequenceNode_create(context, gcFrame.sourcePosition, gcFrame.pragmas, gcFrame.bodyNodes);
        gcFrame.valueNode = tuuvm_astLambdaNode_create(context, gcFrame.sourcePosition,
            tuuvm_tuple_size_encode(context, hasVariadicArguments ? TUUVM_FUNCTION_FLAGS_VARIADIC : TUUVM_FUNCTION_FLAGS_NONE),
            gcFrame.arguments, TUUVM_NULL_TUPLE, gcFrame.bodySequence);
    }
    else
    {
        tuuvm_error("Invalid usage of (define)");
    }

    gcFrame.nameExpression = tuuvm_astLiteralNode_create(context, tuuvm_astNode_getSourcePosition(gcFrame.nameNode), tuuvm_astIdentifierReferenceNode_getValue(gcFrame.nameNode));
    gcFrame.result = tuuvm_astLocalDefinitionNode_create(context, gcFrame.sourcePosition, gcFrame.nameExpression, TUUVM_NULL_TUPLE, gcFrame.valueNode);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
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
        tuuvm_tuple_t pragmas;
        tuuvm_tuple_t bodyNodes;
        tuuvm_tuple_t bodySequence;
        tuuvm_tuple_t nameExpression;
        tuuvm_tuple_t result;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sourcePosition = tuuvm_macroContext_getSourcePosition(*macroContext);

    if(tuuvm_astNode_isUnexpandedSExpressionNode(context, *nameOrLambdaSignature))
    {
        gcFrame.lambdaSignatureElements = tuuvm_astUnexpandedSExpressionNode_getElements(*nameOrLambdaSignature);
        if(tuuvm_array_getSize(gcFrame.lambdaSignatureElements) < 1)
            tuuvm_error("Expected function definition requires a name.");

        gcFrame.nameNode = tuuvm_array_at(gcFrame.lambdaSignatureElements, 0);
        if(!tuuvm_astNode_isIdentifierReferenceNode(context, gcFrame.nameNode))
            tuuvm_error("Expected an identifier reference node for the name.");

        bool hasVariadicArguments = false;
        gcFrame.argumentsNode = tuuvm_array_fromOffset(context, gcFrame.lambdaSignatureElements, 1);
        gcFrame.arguments = tuuvm_astLambdaNode_parseArgumentsNodes(context, gcFrame.argumentsNode, &hasVariadicArguments);
        gcFrame.pragmas = tuuvm_array_create(context, 0);
        gcFrame.bodyNodes = tuuvm_arraySlice_asArray(context, *valueOrBodyNodes);
        gcFrame.bodySequence = tuuvm_astSequenceNode_create(context, gcFrame.sourcePosition, gcFrame.pragmas, gcFrame.bodyNodes);
        gcFrame.valueNode = tuuvm_astLambdaNode_create(context, gcFrame.sourcePosition,
            tuuvm_tuple_size_encode(context, (hasVariadicArguments ? TUUVM_FUNCTION_FLAGS_VARIADIC : TUUVM_FUNCTION_FLAGS_NONE) | TUUVM_FUNCTION_FLAGS_MACRO),
            gcFrame.arguments, TUUVM_NULL_TUPLE, gcFrame.bodySequence);
    }
    else
    {
        tuuvm_error("Invalid usage of (define)");
    }

    gcFrame.nameExpression = tuuvm_astLiteralNode_create(context, tuuvm_astNode_getSourcePosition(gcFrame.nameNode), tuuvm_astIdentifierReferenceNode_getValue(gcFrame.nameNode));
    gcFrame.result = tuuvm_astLocalDefinitionNode_create(context, gcFrame.sourcePosition, gcFrame.nameExpression, TUUVM_NULL_TUPLE, gcFrame.valueNode);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astLocalDefinitionNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_astLocalDefinitionNode_t *localDefinitionNode;
        tuuvm_tuple_t analyzedNameExpression;
        tuuvm_tuple_t analyzedTypeExpression;
        tuuvm_tuple_t analyzedValueExpression;
        
        tuuvm_tuple_t localBinding;
        tuuvm_tuple_t name;
        tuuvm_tuple_t type;
        tuuvm_tuple_t value;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.localDefinitionNode = (tuuvm_astLocalDefinitionNode_t*)tuuvm_context_shallowCopy(context, *node);

    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.localDefinitionNode->super.sourcePosition);

    gcFrame.analyzedNameExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.localDefinitionNode->nameExpression, *environment);
    gcFrame.localDefinitionNode->nameExpression = gcFrame.analyzedNameExpression;

    if(gcFrame.localDefinitionNode->typeExpression)
    {
        gcFrame.analyzedTypeExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.localDefinitionNode->typeExpression, *environment);
        gcFrame.localDefinitionNode->typeExpression = gcFrame.analyzedTypeExpression;

        if(tuuvm_astNode_isLiteralNode(context, gcFrame.analyzedTypeExpression))
            gcFrame.type = tuuvm_astLiteralNode_getValue(gcFrame.analyzedTypeExpression);
    }

    if(gcFrame.localDefinitionNode->valueExpression)
    {
        gcFrame.analyzedValueExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.localDefinitionNode->valueExpression, *environment);
        gcFrame.localDefinitionNode->valueExpression = gcFrame.analyzedValueExpression;
        if(!gcFrame.type)
            gcFrame.type = tuuvm_astNode_getAnalyzedType(gcFrame.analyzedValueExpression);
    }

    gcFrame.localDefinitionNode->super.analyzedType = gcFrame.type;

    if(!gcFrame.analyzedNameExpression)
    {
        if(!gcFrame.analyzedValueExpression)
            gcFrame.analyzedValueExpression = tuuvm_astLiteralNode_create(context, gcFrame.localDefinitionNode->super.sourcePosition, TUUVM_NULL_TUPLE);

        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.analyzedValueExpression;
    }

    if(!tuuvm_astNode_isLiteralNode(context, gcFrame.analyzedNameExpression))
        tuuvm_error("Local definition analyzed name must be a literal node.");

    gcFrame.name = tuuvm_astLiteralNode_getValue(gcFrame.analyzedNameExpression);
    if((!gcFrame.analyzedTypeExpression || tuuvm_astNode_isLiteralNode(context, gcFrame.analyzedTypeExpression)) && 
        (!gcFrame.analyzedValueExpression || tuuvm_astNode_isLiteralNode(context, gcFrame.analyzedValueExpression)))
    {
        if(gcFrame.analyzedValueExpression)
            gcFrame.value = tuuvm_astLiteralNode_getValue(gcFrame.analyzedValueExpression);
        gcFrame.value = tuuvm_type_coerceValue(context, gcFrame.type, gcFrame.value);
        gcFrame.analyzedValueExpression = tuuvm_astLiteralNode_create(context, gcFrame.localDefinitionNode->super.sourcePosition, gcFrame.value);
        gcFrame.localBinding = tuuvm_analysisEnvironment_setNewValueBinding(context, *environment, gcFrame.localDefinitionNode->super.sourcePosition, gcFrame.name, gcFrame.value);
        gcFrame.localDefinitionNode->binding = gcFrame.localBinding;

        // Replace the node by its literal value.
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.analyzedValueExpression;
    }
    else
    {
        gcFrame.localBinding = tuuvm_analysisEnvironment_setNewSymbolLocalBinding(context, *environment, gcFrame.localDefinitionNode->super.sourcePosition, gcFrame.name, gcFrame.type);
        gcFrame.localDefinitionNode->binding = gcFrame.localBinding;
    }

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
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
        tuuvm_tuple_t type;
        tuuvm_tuple_t value;
    } gcFrame = {
        .value = TUUVM_NULL_TUPLE
    };

    tuuvm_astLocalDefinitionNode_t **localDefinitionNode = (tuuvm_astLocalDefinitionNode_t**)node;

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*localDefinitionNode)->super.sourcePosition);

    if((*localDefinitionNode)->typeExpression)
        gcFrame.type = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*localDefinitionNode)->typeExpression, *environment);
    if((*localDefinitionNode)->valueExpression)
        gcFrame.value = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*localDefinitionNode)->valueExpression, *environment);
    if(gcFrame.type)
        gcFrame.value = tuuvm_type_coerceValue(context, gcFrame.type, gcFrame.value);
    tuuvm_functionActivationEnvironment_setBindingActivationValue(context, *environment, (*localDefinitionNode)->binding, gcFrame.value, (*localDefinitionNode)->super.sourcePosition);
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
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
    } gcFrame = {
        .value = TUUVM_NULL_TUPLE
    };
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    tuuvm_astLocalDefinitionNode_t **localDefinitionNode = (tuuvm_astLocalDefinitionNode_t**)node;
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*localDefinitionNode)->super.sourcePosition);

    gcFrame.name = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*localDefinitionNode)->nameExpression, *environment);
    if((*localDefinitionNode)->typeExpression)
        gcFrame.type = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*localDefinitionNode)->typeExpression, *environment);
    if((*localDefinitionNode)->valueExpression)
        gcFrame.value = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*localDefinitionNode)->valueExpression, *environment);
    if(gcFrame.type)
        gcFrame.value = tuuvm_type_coerceValue(context, gcFrame.type, gcFrame.value);
    tuuvm_environment_setNewSymbolBindingWithValueAtSourcePosition(context, *environment, gcFrame.name, gcFrame.value, (*localDefinitionNode)->super.sourcePosition);
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
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

    struct {
        tuuvm_astIdentifierReferenceNode_t *analyzedNode;
        tuuvm_tuple_t binding;
        tuuvm_tuple_t symbolString;
        tuuvm_tuple_t errorMessage;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*referenceNode)->super.sourcePosition);

    // Attempt to replace the symbol with its binding, if it exists.
    if(tuuvm_analysisEnvironment_lookSymbolRecursively(context, *environment, (*referenceNode)->value, &gcFrame.binding))
    {
        if(tuuvm_symbolBinding_isValue(context, gcFrame.binding))
        {
            TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
            return tuuvm_astLiteralNode_create(context, (*referenceNode)->super.sourcePosition, tuuvm_symbolValueBinding_getValue(gcFrame.binding));
        }
    }
    else
    {
        gcFrame.symbolString = tuuvm_tuple_printString(context, (*referenceNode)->value);
        gcFrame.errorMessage = tuuvm_string_concat(context, tuuvm_string_createWithCString(context, "Failed to find symbol binding for "), gcFrame.symbolString);
        tuuvm_errorWithMessageTuple(gcFrame.errorMessage);
    }

    gcFrame.analyzedNode = (tuuvm_astIdentifierReferenceNode_t*)tuuvm_context_shallowCopy(context, (tuuvm_tuple_t)*referenceNode);
    gcFrame.analyzedNode->binding = gcFrame.binding;
    gcFrame.analyzedNode->super.analyzedType = tuuvm_symbolBinding_getType(gcFrame.binding);

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    return (tuuvm_tuple_t)gcFrame.analyzedNode;
}

static tuuvm_tuple_t tuuvm_astIdentifierReferenceNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astIdentifierReferenceNode_t **referenceNode = (tuuvm_astIdentifierReferenceNode_t**)node;

    struct {
        tuuvm_tuple_t binding;
        tuuvm_tuple_t symbolString;
        tuuvm_tuple_t errorMessage;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*referenceNode)->super.sourcePosition);

    if(tuuvm_environment_lookSymbolRecursively(context, *environment, (*referenceNode)->value, &gcFrame.binding))
    {
        if(tuuvm_symbolBinding_isValue(context, gcFrame.binding))
        {
            TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
            return tuuvm_symbolValueBinding_getValue(gcFrame.binding);
        }
    }

    gcFrame.symbolString = tuuvm_tuple_printString(context, (*referenceNode)->value);
    gcFrame.errorMessage = tuuvm_string_concat(context, tuuvm_string_createWithCString(context, "Failed to find symbol binding for "), gcFrame.symbolString);
    tuuvm_errorWithMessageTuple(gcFrame.errorMessage);
    return TUUVM_NULL_TUPLE;
}

static tuuvm_tuple_t tuuvm_astIdentifierReferenceNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astIdentifierReferenceNode_t **referenceNode = (tuuvm_astIdentifierReferenceNode_t**)node;
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*referenceNode)->super.sourcePosition);

    tuuvm_tuple_t result = tuuvm_environment_evaluateSymbolBinding(context, *environment, (*referenceNode)->binding);
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    return result;
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
        tuuvm_tuple_t analyzedCondition;
        tuuvm_tuple_t analyzedTrueExpression;
        tuuvm_tuple_t analyzedTrueExpressionType;
        tuuvm_tuple_t analyzedFalseExpression;
        tuuvm_tuple_t analyzedFalseExpressionType;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.ifNode = (tuuvm_astIfNode_t*)tuuvm_context_shallowCopy(context, *node);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.ifNode->super.sourcePosition);

    gcFrame.analyzedCondition = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.ifNode->conditionExpression, *environment);
    gcFrame.ifNode->conditionExpression = gcFrame.analyzedCondition;
    if(gcFrame.ifNode->trueExpression)
    {
        gcFrame.analyzedTrueExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.ifNode->trueExpression, *environment);
        gcFrame.ifNode->trueExpression = gcFrame.analyzedTrueExpression;
    }
    if(gcFrame.ifNode->falseExpression)
    {
        gcFrame.analyzedFalseExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.ifNode->falseExpression, *environment);
        gcFrame.ifNode->falseExpression = gcFrame.analyzedFalseExpression;
    }

    // Require the same, otherwise fallback to nil.
    gcFrame.ifNode->super.analyzedType = TUUVM_NULL_TUPLE;
    if(gcFrame.analyzedTrueExpression && gcFrame.analyzedFalseExpression)
    {
        gcFrame.analyzedTrueExpressionType = tuuvm_astNode_getAnalyzedType(gcFrame.analyzedTrueExpression);
        gcFrame.analyzedFalseExpressionType = tuuvm_astNode_getAnalyzedType(gcFrame.analyzedFalseExpression);
        if(gcFrame.analyzedTrueExpressionType == gcFrame.analyzedFalseExpressionType)
            gcFrame.ifNode->super.analyzedType = gcFrame.analyzedTrueExpressionType;
    }

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
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
    } gcFrame = {0};
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
    } gcFrame = {0};
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

    size_t applicationArgumentCount = tuuvm_array_getSize((*unexpandedNode)->arguments);

    tuuvm_functionCallFrameStack_t callFrameStack = {0};
    struct {
        tuuvm_tuple_t macroContext;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    tuuvm_functionCallFrameStack_begin(context, &callFrameStack, *macro, 1 + applicationArgumentCount);
    gcFrame.macroContext = tuuvm_macroContext_create(context, *node, (*unexpandedNode)->super.sourcePosition, *environment);
    tuuvm_functionCallFrameStack_push(&callFrameStack, gcFrame.macroContext);
    for(size_t i = 0; i < applicationArgumentCount; ++i)
        tuuvm_functionCallFrameStack_push(&callFrameStack, tuuvm_array_at((*unexpandedNode)->arguments, i));

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
    } gcFrame = {0};
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
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*unexpandedNode)->super.sourcePosition);

    gcFrame.functionOrMacro = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*unexpandedNode)->functionOrMacroExpression, *environment);
    bool isMacro = tuuvm_function_isMacro(context, gcFrame.functionOrMacro);

    if(isMacro)
    {
        gcFrame.expansionResult = tuuvm_astUnexpandedApplicationNode_expandNodeWithMacro(context, node, &gcFrame.functionOrMacro, environment);
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.expansionResult, *environment);
    }

    tuuvm_functionCallFrameStack_t callFrameStack = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    size_t applicationArgumentCount = tuuvm_array_getSize((*unexpandedNode)->arguments);
    tuuvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.functionOrMacro, applicationArgumentCount);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = tuuvm_array_at((*unexpandedNode)->arguments, i);
        gcFrame.argument = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.argumentNode, *environment);
        tuuvm_functionCallFrameStack_push(&callFrameStack, gcFrame.argument);
    }

    //TUUVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
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
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*unexpandedSExpressionNode)->super.sourcePosition);

    size_t elementCount = tuuvm_array_getSize((*unexpandedSExpressionNode)->elements);
    if(elementCount == 0)
    {
        // Empty array.
        gcFrame.array = tuuvm_array_create(context, 0);
        gcFrame.literalNode = tuuvm_astLiteralNode_create(context, (*unexpandedSExpressionNode)->super.sourcePosition, gcFrame.array);
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.literalNode, *environment);
    }
    else
    {
        // Unexpanded application node.
        gcFrame.functionOrMacroExpression = tuuvm_array_at((*unexpandedSExpressionNode)->elements, 0);
        gcFrame.argumentExpressions = tuuvm_array_fromOffset(context, (*unexpandedSExpressionNode)->elements, 1);
        gcFrame.unexpandedApplicationNode = tuuvm_astUnexpandedApplicationNode_create(context, (*unexpandedSExpressionNode)->super.sourcePosition, gcFrame.functionOrMacroExpression, gcFrame.argumentExpressions);
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
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
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*unexpandedSExpressionNode)->super.sourcePosition);

    size_t elementCount = tuuvm_array_getSize((*unexpandedSExpressionNode)->elements);
    if(elementCount == 0)
    {
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return tuuvm_array_create(context, 0);
    }
    else
    {
        // Unexpanded application node.
        gcFrame.functionOrMacroExpression = tuuvm_array_at((*unexpandedSExpressionNode)->elements, 0);
        gcFrame.argumentExpressions = tuuvm_array_fromOffset(context, (*unexpandedSExpressionNode)->elements, 1);
        gcFrame.unexpandedApplicationNode = tuuvm_astUnexpandedApplicationNode_create(context, (*unexpandedSExpressionNode)->super.sourcePosition, gcFrame.functionOrMacroExpression, gcFrame.argumentExpressions);
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.unexpandedApplicationNode, *environment);
    }
}

static tuuvm_tuple_t tuuvm_astPragmaNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_astPragmaNode_t *pragmaNode;
        tuuvm_tuple_t analyzedSelector;
        tuuvm_tuple_t argumentNode;
        tuuvm_tuple_t analyzedArgument;
        tuuvm_tuple_t analyzedArguments;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.pragmaNode = (tuuvm_astPragmaNode_t*)tuuvm_context_shallowCopy(context, *node);

    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.pragmaNode->super.sourcePosition);

    gcFrame.analyzedSelector = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.pragmaNode->selector, *environment);

    size_t pragmaArgumentCount = tuuvm_array_getSize(gcFrame.pragmaNode->arguments);
    gcFrame.analyzedArguments = tuuvm_array_create(context, pragmaArgumentCount);
    for(size_t i = 0; i < pragmaArgumentCount; ++i)
    {
        gcFrame.argumentNode = tuuvm_array_at(gcFrame.pragmaNode->arguments, i);
        gcFrame.analyzedArgument = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.argumentNode, *environment);
        tuuvm_array_atPut(gcFrame.analyzedArguments, i, gcFrame.analyzedArgument);
    }

    gcFrame.pragmaNode->arguments = gcFrame.analyzedArguments;
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.pragmaNode;
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
        tuuvm_tuple_t analyzedFunctionExpression;
        tuuvm_tuple_t argumentNode;
        tuuvm_tuple_t analyzedArgument;
        tuuvm_tuple_t analyzedArguments;
        
        tuuvm_tuple_t literalFunction;
        tuuvm_tuple_t pureCallResult;
        tuuvm_tuple_t argumentValue;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.applicationNode = (tuuvm_astFunctionApplicationNode_t*)tuuvm_context_shallowCopy(context, *node);

    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.applicationNode->super.sourcePosition);

    gcFrame.analyzedFunctionExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.applicationNode->functionExpression, *environment);
    gcFrame.applicationNode->functionExpression = gcFrame.analyzedFunctionExpression;

    bool canOptimizeCall = false;
    if(tuuvm_astNode_isLiteralNode(context, gcFrame.analyzedFunctionExpression))
    {
        gcFrame.literalFunction = tuuvm_astLiteralNode_getValue(gcFrame.analyzedFunctionExpression);
        canOptimizeCall = tuuvm_function_isPure(context, gcFrame.literalFunction);
    }

    size_t applicationArgumentCount = tuuvm_array_getSize(gcFrame.applicationNode->arguments);
    gcFrame.analyzedArguments = tuuvm_array_create(context, applicationArgumentCount);
    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = tuuvm_array_at(gcFrame.applicationNode->arguments, i);
        gcFrame.analyzedArgument = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.argumentNode, *environment);
        canOptimizeCall = canOptimizeCall && tuuvm_astNode_isLiteralNode(context, gcFrame.analyzedArgument);
        tuuvm_array_atPut(gcFrame.analyzedArguments, i, gcFrame.analyzedArgument);
    }

    gcFrame.applicationNode->arguments = gcFrame.analyzedArguments;

    if(canOptimizeCall)
    {
        tuuvm_functionCallFrameStack_t callFrameStack = {0};
        TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

        tuuvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.literalFunction, applicationArgumentCount);

        for(size_t i = 0; i < applicationArgumentCount; ++i)
        {
            gcFrame.analyzedArgument = tuuvm_array_at(gcFrame.analyzedArguments, i);
            gcFrame.argumentValue = tuuvm_astLiteralNode_getValue(gcFrame.analyzedArgument);
            tuuvm_functionCallFrameStack_push(&callFrameStack, gcFrame.argumentValue);
        }

        TUUVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
        gcFrame.pureCallResult = tuuvm_functionCallFrameStack_finish(context, &callFrameStack);

        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return tuuvm_astLiteralNode_create(context, gcFrame.applicationNode->super.sourcePosition, gcFrame.pureCallResult);
    }

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
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

    tuuvm_functionCallFrameStack_t callFrameStack = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    size_t applicationArgumentCount = tuuvm_array_getSize((*applicationNode)->arguments);
    tuuvm_functionCallFrameStack_begin(context, &callFrameStack, tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*applicationNode)->functionExpression, *environment), applicationArgumentCount);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        tuuvm_tuple_t argumentNode = tuuvm_array_at((*applicationNode)->arguments, i);
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
        tuuvm_tuple_t function;
        tuuvm_tuple_t argumentNode;
        tuuvm_tuple_t result;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*applicationNode)->super.sourcePosition);

    size_t applicationArgumentCount = tuuvm_array_getSize((*applicationNode)->arguments);
    gcFrame.function = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*applicationNode)->functionExpression, *environment);

    tuuvm_functionCallFrameStack_t callFrameStack = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);
    tuuvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.function, applicationArgumentCount);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = tuuvm_array_at((*applicationNode)->arguments, i);
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
        tuuvm_tuple_t analyzedBodyNode;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.lexicalBlockNode = (tuuvm_astLexicalBlockNode_t*)tuuvm_context_shallowCopy(context, *node);

    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.lexicalBlockNode->super.sourcePosition);

    gcFrame.childEnvironment = tuuvm_localAnalysisEnvironment_create(context, *environment);
    gcFrame.analyzedBodyNode = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.lexicalBlockNode->body, gcFrame.childEnvironment);
    gcFrame.lexicalBlockNode->body = gcFrame.analyzedBodyNode;
    gcFrame.lexicalBlockNode->bodyEnvironment = gcFrame.childEnvironment;
    gcFrame.lexicalBlockNode->super.analyzedType = tuuvm_astNode_getAnalyzedType(gcFrame.analyzedBodyNode);
    
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
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
    } gcFrame = {0};
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

    return tuuvm_interpreter_evaluateASTWithEnvironment(context, (*lexicalBlockNode)->body, *environment);
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
    } gcFrame = {0};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*tupleNode)->super.sourcePosition);

    gcFrame.expressions = (*tupleNode)->elements;
    size_t expressionCount = tuuvm_array_getSize(gcFrame.expressions);
    if(expressionCount == 0 && (*tupleNode)->super.analyzedType)
    {
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return *node;
    }

    gcFrame.analyzedTupleNode = (tuuvm_astMakeByteArrayNode_t *)tuuvm_context_shallowCopy(context, *node);
    
    gcFrame.analyzedExpressions = tuuvm_array_create(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expression = tuuvm_array_at(gcFrame.expressions, i);
        gcFrame.analyzedExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.expression, *environment);
        tuuvm_array_atPut(gcFrame.analyzedExpressions, i, gcFrame.analyzedExpression);
    }

    gcFrame.analyzedTupleNode->super.analyzedType = context->roots.byteArrayType;
    gcFrame.analyzedTupleNode->elements = gcFrame.analyzedExpressions;
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
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
    } gcFrame = {0};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*byteArrayNode)->super.sourcePosition);

    size_t expressionCount = tuuvm_array_getSize((*byteArrayNode)->elements);
    gcFrame.result = tuuvm_byteArray_create(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        tuuvm_tuple_t expression = tuuvm_array_at((*byteArrayNode)->elements, i);
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
    } gcFrame = {0};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*byteArrayNode)->super.sourcePosition);

    size_t expressionCount = tuuvm_array_getSize((*byteArrayNode)->elements);
    gcFrame.result = tuuvm_byteArray_create(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        tuuvm_tuple_t expression = tuuvm_array_at((*byteArrayNode)->elements, i);
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
    } gcFrame = {0};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*tupleNode)->super.sourcePosition);

    gcFrame.expressions = (*tupleNode)->elements;
    size_t expressionCount = tuuvm_array_getSize(gcFrame.expressions);
    if(expressionCount == 0 && (*tupleNode)->super.analyzedType)
    {
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return *node;
    }

    gcFrame.analyzedTupleNode = (tuuvm_astMakeTupleNode_t *)tuuvm_context_shallowCopy(context, *node);
    
    gcFrame.analyzedExpressions = tuuvm_array_create(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expression = tuuvm_array_at(gcFrame.expressions, i);
        gcFrame.analyzedExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.expression, *environment);
        tuuvm_array_atPut(gcFrame.analyzedExpressions, i, gcFrame.analyzedExpression);
    }

    gcFrame.analyzedTupleNode->super.analyzedType = context->roots.arrayType;
    gcFrame.analyzedTupleNode->elements = gcFrame.analyzedExpressions;
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
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
    } gcFrame = {0};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*tupleNode)->super.sourcePosition);

    size_t expressionCount = tuuvm_array_getSize((*tupleNode)->elements);
    gcFrame.result = tuuvm_array_create(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        tuuvm_tuple_t expression = tuuvm_array_at((*tupleNode)->elements, i);
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
        tuuvm_tuple_t expression;
        tuuvm_tuple_t element;
    } gcFrame = {0};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*tupleNode)->super.sourcePosition);

    size_t expressionCount = tuuvm_array_getSize((*tupleNode)->elements);
    gcFrame.result = tuuvm_array_create(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expression = tuuvm_array_at((*tupleNode)->elements, i);
        gcFrame.element = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.expression, *environment);
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
        tuuvm_tuple_t analyzedSelector;
        tuuvm_tuple_t analyzedArguments;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sendNode = (tuuvm_astMessageChainMessageNode_t*)tuuvm_context_shallowCopy(context, *node);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.sendNode->super.sourcePosition);

    gcFrame.analyzedSelector = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.sendNode->selector, *environment);
    gcFrame.sendNode->selector = gcFrame.analyzedSelector;

    size_t applicationArgumentCount = tuuvm_array_getSize(gcFrame.sendNode->arguments);
    gcFrame.analyzedArguments = tuuvm_array_create(context, applicationArgumentCount);
    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        tuuvm_tuple_t argumentNode = tuuvm_array_at(gcFrame.sendNode->arguments, i);
        tuuvm_array_atPut(gcFrame.analyzedArguments, i, tuuvm_interpreter_analyzeASTWithEnvironment(context, argumentNode, *environment));
    }

    gcFrame.sendNode->arguments = gcFrame.analyzedArguments;
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
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
        tuuvm_tuple_t analyzedReceiver;
        tuuvm_tuple_t analyzedChainedMessages;
        tuuvm_tuple_t analyzedChainedMessageNode;
        tuuvm_tuple_t chainedMessageNode;

        tuuvm_tuple_t receiverType;
        tuuvm_tuple_t receiverMetaType;
        tuuvm_tuple_t analysisFunction;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    
    gcFrame.chainNode = (tuuvm_astMessageChainNode_t*)tuuvm_context_shallowCopy(context, *node);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.chainNode->super.sourcePosition);

    if(gcFrame.chainNode->receiver)
    {
        gcFrame.analyzedReceiver = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.chainNode->receiver, *environment);
        gcFrame.chainNode->receiver = gcFrame.analyzedReceiver;
        gcFrame.receiverType = tuuvm_astNode_getAnalyzedType(gcFrame.analyzedReceiver);
        if(gcFrame.receiverType)
        {
            gcFrame.receiverMetaType = tuuvm_tuple_getType(context, gcFrame.receiverType);
            gcFrame.analysisFunction = tuuvm_type_getAnalyzeMessageChainNodeWithEnvironmentFunction(context, gcFrame.receiverMetaType);
            if(gcFrame.analysisFunction)
            {
                TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
                TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
                return tuuvm_function_apply3(context, gcFrame.analysisFunction, gcFrame.receiverType, (tuuvm_tuple_t)gcFrame.chainNode, *environment);
            }

        }
    }

    size_t chainedMessageCount = tuuvm_array_getSize(gcFrame.chainNode->messages);
    gcFrame.analyzedChainedMessages = tuuvm_array_create(context, chainedMessageCount);
    gcFrame.chainNode->super.analyzedType = TUUVM_NULL_TUPLE;
    for(size_t i = 0; i < chainedMessageCount; ++i)
    {
        gcFrame.chainedMessageNode = tuuvm_array_at(gcFrame.chainNode->messages, i);
        gcFrame.analyzedChainedMessageNode = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.chainedMessageNode, *environment);
        gcFrame.chainNode->super.analyzedType = tuuvm_astNode_getAnalyzedType(gcFrame.analyzedChainedMessageNode);
        tuuvm_array_atPut(gcFrame.analyzedChainedMessages, i, gcFrame.analyzedChainedMessageNode);
    }

    gcFrame.chainNode->messages = gcFrame.analyzedChainedMessages;
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.chainNode;
}

static tuuvm_tuple_t tuuvm_astMessageChainMessageNode_analyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t chainedMessage, bool hasReceiver, tuuvm_tuple_t *receiver, tuuvm_tuple_t *environment)
{
    struct {
        tuuvm_astMessageChainMessageNode_t *node;
        tuuvm_tuple_t selector;
        tuuvm_tuple_t receiverType;
        tuuvm_tuple_t methodBinding;
        tuuvm_tuple_t method;
        tuuvm_tuple_t result;

        tuuvm_tuple_t message;
        tuuvm_tuple_t arguments;
        tuuvm_tuple_t argumentNode;
        tuuvm_tuple_t argument;
    } gcFrame = {
        .node = (tuuvm_astMessageChainMessageNode_t*)chainedMessage
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.node->super.sourcePosition);

    gcFrame.selector = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.node->selector, *environment);
    size_t applicationArgumentCount = tuuvm_array_getSize(gcFrame.node->arguments);

    if(hasReceiver)
    {
        gcFrame.receiverType = tuuvm_tuple_getType(context, *receiver);
        gcFrame.method = tuuvm_type_lookupSelector(context, gcFrame.receiverType, gcFrame.selector);
        if(!gcFrame.method)
        {
            gcFrame.method = tuuvm_type_lookupSelector(context, gcFrame.receiverType, context->roots.doesNotUnderstandSelector);
            if(!gcFrame.method)
                tuuvm_error("Message not understood.");

            gcFrame.arguments = tuuvm_array_create(context, applicationArgumentCount);
            for(size_t i = 0; i < applicationArgumentCount; ++i)
            {
                gcFrame.argumentNode = tuuvm_array_at(gcFrame.node->arguments, i);
                gcFrame.argument = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.argumentNode, *environment);
                tuuvm_array_atPut(gcFrame.argument, i, gcFrame.argument);
            }

            gcFrame.message = tuuvm_message_create(context, gcFrame.selector, gcFrame.arguments);
            gcFrame.result = tuuvm_function_apply2(context, gcFrame.method, *receiver, gcFrame.message);
            TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
            TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return gcFrame.result;
        }
    }
    else
    {
        if(!tuuvm_environment_lookSymbolRecursively(context, *environment, gcFrame.selector, &gcFrame.methodBinding)
            || !tuuvm_symbolBinding_isValue(context, gcFrame.methodBinding))
            tuuvm_error("Failed to find symbol for message send without receiver.");
        gcFrame.method = tuuvm_symbolValueBinding_getValue(gcFrame.methodBinding);
    }


    tuuvm_functionCallFrameStack_t callFrameStack = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    tuuvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.method, applicationArgumentCount + (hasReceiver ? 1 : 0));

    if(hasReceiver)
        tuuvm_functionCallFrameStack_push(&callFrameStack, *receiver);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = tuuvm_array_at(gcFrame.node->arguments, i);
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
    } gcFrame = {0};
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

    size_t chainedMessageCount = tuuvm_array_getSize((*chainNode)->messages);
    for(size_t i = 0; i < chainedMessageCount; ++i)
    {
        gcFrame.chainedMessage = tuuvm_array_at((*chainNode)->messages, i);
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
        tuuvm_tuple_t receiverType;
        tuuvm_tuple_t selector;
        tuuvm_tuple_t methodBinding;
        tuuvm_tuple_t method;
        tuuvm_tuple_t arguments;
        tuuvm_tuple_t argumentNode;
        tuuvm_tuple_t argument;
        tuuvm_tuple_t message;
        tuuvm_tuple_t result;
    } gcFrame = {
        .node = (tuuvm_astMessageChainMessageNode_t*)chainedMessage
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.node->super.sourcePosition);

    size_t applicationArgumentCount = tuuvm_array_getSize(gcFrame.node->arguments);
    gcFrame.selector = tuuvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.node->selector, *environment);
    if(hasReceiver)
    {
        gcFrame.receiverType = tuuvm_tuple_getType(context, *receiver);
        gcFrame.method = tuuvm_type_lookupSelector(context, gcFrame.receiverType, gcFrame.selector);
        if(!gcFrame.method)
        {
            gcFrame.method = tuuvm_type_lookupSelector(context, gcFrame.receiverType, context->roots.doesNotUnderstandSelector);
            if(!gcFrame.method)
                tuuvm_error("Message not understood.");

            gcFrame.arguments = tuuvm_array_create(context, applicationArgumentCount);
            for(size_t i = 0; i < applicationArgumentCount; ++i)
            {
                gcFrame.argumentNode = tuuvm_array_at(gcFrame.node->arguments, i);
                gcFrame.argument = tuuvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.argumentNode, *environment);
                tuuvm_array_atPut(gcFrame.argument, i, gcFrame.argument);
            }

            gcFrame.message = tuuvm_message_create(context, gcFrame.selector, gcFrame.arguments);
            gcFrame.result = tuuvm_function_apply2(context, gcFrame.method, *receiver, gcFrame.message);
            TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
            TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return gcFrame.result;
        }
    }
    else
    {
        if(!tuuvm_environment_lookSymbolRecursively(context, *environment, gcFrame.selector, &gcFrame.methodBinding)
            || !tuuvm_symbolBinding_isValue(context, gcFrame.methodBinding))
            tuuvm_error("Failed to find symbol for message send without receiver.");
        gcFrame.method = tuuvm_symbolValueBinding_getValue(gcFrame.methodBinding);
    }


    tuuvm_functionCallFrameStack_t callFrameStack = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    tuuvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.method, applicationArgumentCount + (hasReceiver ? 1 : 0));

    if(hasReceiver)
        tuuvm_functionCallFrameStack_push(&callFrameStack, *receiver);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = tuuvm_array_at(gcFrame.node->arguments, i);
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
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*chainNode)->super.sourcePosition);

    bool hasReceiver = false;
    if((*chainNode)->receiver)
    {
        gcFrame.receiver = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*chainNode)->receiver, *environment);
        hasReceiver = true;
    }

    size_t chainedMessageCount = tuuvm_array_getSize((*chainNode)->messages);
    for(size_t i = 0; i < chainedMessageCount; ++i)
    {
        gcFrame.chainedMessage = tuuvm_array_at((*chainNode)->messages, i);
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
    return tuuvm_astMessageSendNode_create(context, sourcePosition, *receiverNode, *selectorNode, tuuvm_arraySlice_asArray(context, *argumentNodes));
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
        tuuvm_tuple_t analyzedReceiver;
        tuuvm_tuple_t analyzedSelector;
        tuuvm_tuple_t analyzedArguments;

        tuuvm_tuple_t receiverType;
        tuuvm_tuple_t receiverMetaType;
        tuuvm_tuple_t selector;
        tuuvm_tuple_t method;
        tuuvm_tuple_t analysisFunction;
        tuuvm_tuple_t result;
        tuuvm_tuple_t newMethodNode;
        tuuvm_tuple_t newArgumentNodes;
        tuuvm_tuple_t unexpandedApplicationNode;

        tuuvm_tuple_t macroContext;
        tuuvm_tuple_t expansionResult;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sendNode = (tuuvm_astMessageSendNode_t*)tuuvm_context_shallowCopy(context, *node);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.sendNode->super.sourcePosition);

    bool isDoesNotUnderstand = false;
    if(gcFrame.sendNode->receiver)
    {
        gcFrame.analyzedReceiver = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.sendNode->receiver, *environment);
        gcFrame.sendNode->receiver = gcFrame.analyzedReceiver;
        gcFrame.receiverType = tuuvm_astNode_getAnalyzedType(gcFrame.analyzedReceiver);

        if(gcFrame.receiverType)
        {
            // If the receiver is a literal node, we can attempt to forward the message send node analysis.
            gcFrame.receiverMetaType = tuuvm_tuple_getType(context, gcFrame.receiverType);
            gcFrame.analysisFunction = tuuvm_type_getAnalyzeMessageSendNodeWithEnvironmentFunction(context, gcFrame.receiverMetaType);

            if(gcFrame.analysisFunction)
            {
                gcFrame.result = tuuvm_function_apply3(context, gcFrame.analysisFunction, gcFrame.receiverType, (tuuvm_tuple_t)gcFrame.sendNode, *environment);
                TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
                TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
                return gcFrame.result;
            }

            // If the selector is a literal, attempt to perform a static lookup.
            gcFrame.analyzedSelector = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.sendNode->selector, *environment);
            gcFrame.sendNode->selector = gcFrame.analyzedSelector;
            if(tuuvm_astNode_isLiteralNode(context, gcFrame.sendNode->selector))
            {
                gcFrame.selector = tuuvm_astLiteralNode_getValue(gcFrame.sendNode->selector);
                gcFrame.method = tuuvm_type_lookupMacroSelector(context, gcFrame.receiverType, gcFrame.selector);
                if(!gcFrame.method)
                    gcFrame.method = tuuvm_type_lookupSelector(context, gcFrame.receiverType, gcFrame.selector);
                if(!gcFrame.method)
                    gcFrame.method = tuuvm_type_lookupFallbackSelector(context, gcFrame.receiverType, gcFrame.selector);

                // does not understand macro?
                if(!gcFrame.method)
                {
                    if(!gcFrame.method)
                        gcFrame.method = tuuvm_type_lookupMacroSelector(context, gcFrame.receiverType, context->roots.doesNotUnderstandSelector);
                    if(!gcFrame.method)
                        gcFrame.method = tuuvm_type_lookupSelector(context, gcFrame.receiverType, context->roots.doesNotUnderstandSelector);
                    if(!gcFrame.method)
                        gcFrame.method = tuuvm_type_lookupFallbackSelector(context, gcFrame.receiverType, context->roots.doesNotUnderstandSelector);

                    if(!tuuvm_function_isMacro(context, gcFrame.method))
                        gcFrame.method = TUUVM_NULL_TUPLE;
                    isDoesNotUnderstand = gcFrame.method != TUUVM_NULL_TUPLE;
                }
                    
                // Allow definining some macros at the any value type level.
                if(!gcFrame.method)
                    gcFrame.method = tuuvm_type_lookupMacroSelector(context, context->roots.anyValueType, gcFrame.selector);
            }
        }
        else
        {
            gcFrame.analyzedSelector = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.sendNode->selector, *environment);
            gcFrame.sendNode->selector = gcFrame.analyzedSelector;
            if(tuuvm_astNode_isLiteralNode(context, gcFrame.sendNode->selector))
            {
                gcFrame.selector = tuuvm_astLiteralNode_getValue(gcFrame.sendNode->selector);
                gcFrame.method = tuuvm_type_lookupMacroSelector(context, context->roots.anyValueType, gcFrame.selector);
            }
        }
    }
    else
    {
        gcFrame.analyzedSelector = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.sendNode->selector, *environment);
        gcFrame.sendNode->selector = gcFrame.analyzedSelector;
    }

    // Does not understand: 
    if(isDoesNotUnderstand && tuuvm_function_isMacro(context, gcFrame.method))
    {
        gcFrame.macroContext = tuuvm_macroContext_create(context, *node, gcFrame.sendNode->super.sourcePosition, *environment);
        gcFrame.expansionResult = tuuvm_function_apply3(context, gcFrame.method, gcFrame.macroContext, gcFrame.sendNode->receiver, (tuuvm_tuple_t)gcFrame.sendNode);
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);

        // Analyze and evaluate the resulting node.
        return tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.expansionResult, *environment);
    }

    // Turn this node onto an unexpanded application.
    if(gcFrame.method && tuuvm_function_shouldOptimizeLookup(context, gcFrame.method, gcFrame.receiverType))
    {
        size_t applicationArgumentCount = tuuvm_array_getSize(gcFrame.sendNode->arguments);
        gcFrame.newMethodNode = tuuvm_astLiteralNode_create(context, gcFrame.sendNode->super.sourcePosition, gcFrame.method);

        gcFrame.newArgumentNodes = tuuvm_array_create(context, 1 + applicationArgumentCount);
        tuuvm_array_atPut(gcFrame.newArgumentNodes, 0, gcFrame.sendNode->receiver);
        for(size_t i = 0; i < applicationArgumentCount; ++i)
            tuuvm_array_atPut(gcFrame.newArgumentNodes, i + 1, tuuvm_array_at(gcFrame.sendNode->arguments, i));

        gcFrame.unexpandedApplicationNode = tuuvm_astUnexpandedApplicationNode_create(context, gcFrame.sendNode->super.sourcePosition, gcFrame.newMethodNode, gcFrame.newArgumentNodes);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.unexpandedApplicationNode, *environment);
    }

    size_t applicationArgumentCount = tuuvm_array_getSize(gcFrame.sendNode->arguments);
    gcFrame.analyzedArguments = tuuvm_array_create(context, applicationArgumentCount);
    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        tuuvm_tuple_t argumentNode = tuuvm_array_at(gcFrame.sendNode->arguments, i);
        tuuvm_array_atPut(gcFrame.analyzedArguments, i, tuuvm_interpreter_analyzeASTWithEnvironment(context, argumentNode, *environment));
    }

    gcFrame.sendNode->arguments = gcFrame.analyzedArguments;
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
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
        tuuvm_tuple_t methodBinding;

        tuuvm_tuple_t analysisAndEvaluationFunction;
        tuuvm_tuple_t macroContext;
        tuuvm_tuple_t expansionResult;
        tuuvm_tuple_t receiverLiteralNode;
        tuuvm_tuple_t selectorLiteralNode;
        tuuvm_astMessageSendNode_t *messageNode;
        tuuvm_tuple_t arguments;
        tuuvm_tuple_t argumentNode;
        tuuvm_tuple_t argument;
        tuuvm_tuple_t message;
        tuuvm_tuple_t result;
    } gcFrame = {0};
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
            size_t applicationArgumentCount = tuuvm_array_getSize((*sendNode)->arguments);
            gcFrame.method = tuuvm_type_lookupMacroSelector(context, gcFrame.receiverType, context->roots.doesNotUnderstandSelector);
            if(!gcFrame.method)
                gcFrame.method = tuuvm_type_lookupSelector(context, gcFrame.receiverType, context->roots.doesNotUnderstandSelector);
            if(!gcFrame.method)
                gcFrame.method = tuuvm_type_lookupFallbackSelector(context, gcFrame.receiverType, context->roots.doesNotUnderstandSelector);
            if(!gcFrame.method)
                tuuvm_error("Message not understood.");

            // Does not understand: 
            if(tuuvm_function_isMacro(context, gcFrame.method))
            {
                gcFrame.macroContext = tuuvm_macroContext_create(context, *node, (*sendNode)->super.sourcePosition, *environment);
                gcFrame.messageNode = (tuuvm_astMessageSendNode_t *)tuuvm_context_shallowCopy(context, (tuuvm_tuple_t)*sendNode);

                gcFrame.receiverLiteralNode = tuuvm_astLiteralNode_create(context, tuuvm_astNode_getSourcePosition((*sendNode)->receiver), gcFrame.receiver);
                gcFrame.messageNode->receiver = gcFrame.receiverLiteralNode;

                gcFrame.selectorLiteralNode = tuuvm_astLiteralNode_create(context, tuuvm_astNode_getSourcePosition((*sendNode)->selector), gcFrame.selector);
                gcFrame.messageNode->selector = gcFrame.selectorLiteralNode;

                gcFrame.expansionResult = tuuvm_function_apply3(context, gcFrame.method, gcFrame.macroContext, gcFrame.receiverLiteralNode, (tuuvm_tuple_t)gcFrame.messageNode);
                TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
                TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);

                // Analyze and evaluate the resulting node.
                return tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.expansionResult, *environment);
            }
            else
            {
                gcFrame.arguments = tuuvm_array_create(context, applicationArgumentCount);
                for(size_t i = 0; i < applicationArgumentCount; ++i)
                {
                    gcFrame.argumentNode = tuuvm_array_at((*sendNode)->arguments, i);
                    gcFrame.argument = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.argumentNode, *environment);
                    tuuvm_array_atPut(gcFrame.argument, i, gcFrame.argument);
                }

                gcFrame.message = tuuvm_message_create(context, gcFrame.selector, gcFrame.arguments);
                gcFrame.result = tuuvm_function_apply2(context, gcFrame.method, gcFrame.receiver, gcFrame.message);
                TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
                TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
                return gcFrame.result;
            }
        }
    }
    else
    {
        gcFrame.selector = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*sendNode)->selector, *environment);
        if(!tuuvm_environment_lookSymbolRecursively(context, *environment, gcFrame.selector, &gcFrame.methodBinding)
            || !tuuvm_symbolBinding_isValue(context, gcFrame.methodBinding))
            tuuvm_error("Failed to find symbol for message send without receiver.");
        gcFrame.method = tuuvm_symbolValueBinding_getValue(gcFrame.methodBinding);
    }

    bool isMacro = tuuvm_function_isMacro(context, gcFrame.method);
    tuuvm_functionCallFrameStack_t callFrameStack = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    if(isMacro)
    {
        size_t applicationArgumentCount = tuuvm_array_getSize((*sendNode)->arguments);
        tuuvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.method, 1 + applicationArgumentCount + (hasReceiver ? 1 : 0));

        gcFrame.macroContext = tuuvm_macroContext_create(context, *node, (*sendNode)->super.sourcePosition, *environment);
        tuuvm_functionCallFrameStack_push(&callFrameStack, gcFrame.macroContext);

        // We need to push the receiver as a node, so wrap it in a literal node here.
        if(hasReceiver)
        {
            gcFrame.receiverLiteralNode = tuuvm_astLiteralNode_create(context, tuuvm_astNode_getSourcePosition((*sendNode)->receiver), gcFrame.receiver);
            tuuvm_functionCallFrameStack_push(&callFrameStack, gcFrame.receiverLiteralNode);
        }

        // Push the argument nodes.
        for(size_t i = 0; i < applicationArgumentCount; ++i)
            tuuvm_functionCallFrameStack_push(&callFrameStack, tuuvm_array_at((*sendNode)->arguments, i));

        TUUVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
        gcFrame.expansionResult = tuuvm_functionCallFrameStack_finish(context, &callFrameStack);
        TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);

        // Analyze and evaluate the resulting node.
        return tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.expansionResult, *environment);
    }
    else
    {
        size_t applicationArgumentCount = tuuvm_array_getSize((*sendNode)->arguments);
        tuuvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.method, applicationArgumentCount + (hasReceiver ? 1 : 0));

        if(hasReceiver)
            tuuvm_functionCallFrameStack_push(&callFrameStack, gcFrame.receiver);

        for(size_t i = 0; i < applicationArgumentCount; ++i)
        {
            gcFrame.argumentNode = tuuvm_array_at((*sendNode)->arguments, i);
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
        tuuvm_tuple_t receiverType;
        tuuvm_tuple_t receiver;
        tuuvm_tuple_t selector;
        tuuvm_tuple_t method;
        tuuvm_tuple_t methodBinding;
        tuuvm_tuple_t argumentNode;
        tuuvm_tuple_t argument;
        tuuvm_tuple_t arguments;
        tuuvm_tuple_t message;
        tuuvm_tuple_t result;

        tuuvm_tuple_t receiverString;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*sendNode)->super.sourcePosition);

    bool hasReceiver = false;
    size_t applicationArgumentCount = tuuvm_array_getSize((*sendNode)->arguments);
    if((*sendNode)->receiver)
    {
        gcFrame.receiver = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*sendNode)->receiver, *environment);
        hasReceiver = true;

        gcFrame.receiverType = tuuvm_tuple_getType(context, gcFrame.receiver);
        gcFrame.selector = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*sendNode)->selector, *environment);
        gcFrame.method = tuuvm_type_lookupSelector(context, gcFrame.receiverType, gcFrame.selector);
        if(!gcFrame.method)
        {
            gcFrame.method = tuuvm_type_lookupSelector(context, gcFrame.receiverType, context->roots.doesNotUnderstandSelector);
            if(!gcFrame.method)
                tuuvm_error("Message not understood.");

            gcFrame.arguments = tuuvm_array_create(context, applicationArgumentCount);
            for(size_t i = 0; i < applicationArgumentCount; ++i)
            {
                gcFrame.argumentNode = tuuvm_array_at((*sendNode)->arguments, i);
                gcFrame.argument = tuuvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.argumentNode, *environment);
                tuuvm_array_atPut(gcFrame.argument, i, gcFrame.argument);
            }

            gcFrame.message = tuuvm_message_create(context, gcFrame.selector, gcFrame.arguments);
            gcFrame.result = tuuvm_function_apply2(context, gcFrame.method, gcFrame.receiver, gcFrame.message);
            TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
            TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return gcFrame.result;
        }
    }
    else
    {
        gcFrame.selector = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*sendNode)->selector, *environment);
        if(!tuuvm_environment_lookSymbolRecursively(context, *environment, gcFrame.selector, &gcFrame.methodBinding)
            || !tuuvm_symbolBinding_isValue(context, gcFrame.methodBinding))
            tuuvm_error("Failed to find symbol for message send without receiver.");
        gcFrame.method = tuuvm_symbolValueBinding_getValue(gcFrame.methodBinding);
    }


    tuuvm_functionCallFrameStack_t callFrameStack = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    tuuvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.method, applicationArgumentCount + (hasReceiver ? 1 : 0));

    if(hasReceiver)
        tuuvm_functionCallFrameStack_push(&callFrameStack, gcFrame.receiver);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = tuuvm_array_at((*sendNode)->arguments, i);
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
        tuuvm_tuple_t analyzedBodyExpression;
        tuuvm_tuple_t analyzedConditionExpression;
        tuuvm_tuple_t analyzedContinueExpression;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.doWhileNode = (tuuvm_astDoWhileContinueWithNode_t*)tuuvm_context_shallowCopy(context, *node);
    gcFrame.doWhileNode->super.analyzedType = context->roots.voidType;
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.doWhileNode->super.sourcePosition);

    if(gcFrame.doWhileNode->bodyExpression)
    {
        gcFrame.analyzedBodyExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.doWhileNode->bodyExpression, *environment);
        gcFrame.doWhileNode->bodyExpression = gcFrame.analyzedBodyExpression;
    }

    if(gcFrame.doWhileNode->conditionExpression)
    {
        gcFrame.analyzedConditionExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.doWhileNode->conditionExpression, *environment);
        gcFrame.doWhileNode->conditionExpression = gcFrame.analyzedConditionExpression;
    }

    if(gcFrame.doWhileNode->continueExpression)
    {
        gcFrame.analyzedContinueExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.doWhileNode->continueExpression, *environment);
        gcFrame.doWhileNode->continueExpression = gcFrame.analyzedContinueExpression;
    }

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
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
        if((*doWhileNode)->bodyExpression)
            tuuvm_interpreter_evaluateASTWithEnvironment(context, (*doWhileNode)->bodyExpression, *environment);
        if((*doWhileNode)->conditionExpression)
            shouldContinue = tuuvm_tuple_boolean_decode(tuuvm_interpreter_evaluateASTWithEnvironment(context, (*doWhileNode)->conditionExpression, *environment));
        else
            shouldContinue = true;

        if(shouldContinue)
        {
            if((*doWhileNode)->continueExpression)
                tuuvm_interpreter_evaluateASTWithEnvironment(context, (*doWhileNode)->continueExpression, *environment);
        }

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

    tuuvm_astDoWhileContinueWithNode_t **doWhileNode = (tuuvm_astDoWhileContinueWithNode_t**)node;
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*doWhileNode)->super.sourcePosition);
    bool shouldContinue = false;
    do
    {
        if((*doWhileNode)->bodyExpression)
            tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*doWhileNode)->bodyExpression, *environment);
        if((*doWhileNode)->conditionExpression)
            shouldContinue = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*doWhileNode)->conditionExpression, *environment);
        else
            shouldContinue = true;

        if(shouldContinue)
        {
            if((*doWhileNode)->continueExpression)
                tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*doWhileNode)->continueExpression, *environment);
        }

        tuuvm_gc_safepoint(context);
    } while (shouldContinue);
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);

    return TUUVM_VOID_TUPLE;
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
        tuuvm_tuple_t analyzedConditionExpression;
        tuuvm_tuple_t analyzedBodyExpression;
        tuuvm_tuple_t analyzedContinueExpression;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.whileNode = (tuuvm_astWhileContinueWithNode_t*)tuuvm_context_shallowCopy(context, *node);
    gcFrame.whileNode->super.analyzedType = context->roots.voidType;

    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.whileNode->super.sourcePosition);
    
    if(gcFrame.whileNode->conditionExpression)
    {
        gcFrame.analyzedConditionExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.whileNode->conditionExpression, *environment);
        gcFrame.whileNode->conditionExpression = gcFrame.analyzedConditionExpression;
    }

    if(gcFrame.whileNode->bodyExpression)
    {
        gcFrame.analyzedBodyExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.whileNode->bodyExpression, *environment);
        gcFrame.whileNode->bodyExpression = gcFrame.analyzedBodyExpression;
    }

    if(gcFrame.whileNode->continueExpression)
    {
        gcFrame.analyzedContinueExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.whileNode->continueExpression, *environment);
        gcFrame.whileNode->continueExpression = gcFrame.analyzedContinueExpression;
    }
    
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.whileNode;
}

static tuuvm_tuple_t tuuvm_astWhileContinueWithNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_tuple_t loopEnvironment;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.loopEnvironment = *environment;

    tuuvm_astWhileContinueWithNode_t **whileNode = (tuuvm_astWhileContinueWithNode_t**)node;
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*whileNode)->super.sourcePosition);

    bool shouldContinue = true;
    
    tuuvm_stackFrameBreakTargetRecord_t breakTargetRecord = {
        .type = TUUVM_STACK_FRAME_RECORD_TYPE_BREAK_TARGET,
        .environment = gcFrame.loopEnvironment
    };
    tuuvm_stackFrame_pushRecord((tuuvm_stackFrameRecord_t*)&breakTargetRecord);  
    tuuvm_environment_setBreakTarget(gcFrame.loopEnvironment, tuuvm_tuple_uintptr_encode(context, (uintptr_t)&breakTargetRecord));

    if(!setjmp(breakTargetRecord.jmpbuffer))
    {
        if((*whileNode)->conditionExpression)
            shouldContinue = tuuvm_tuple_boolean_decode(tuuvm_interpreter_evaluateASTWithEnvironment(context, (*whileNode)->conditionExpression, gcFrame.loopEnvironment));

        while(shouldContinue)
        {
            if((*whileNode)->bodyExpression)
            {
                tuuvm_stackFrameContinueTargetRecord_t continueTargetRecord = {
                    .type = TUUVM_STACK_FRAME_RECORD_TYPE_CONTINUE_TARGET,
                    .environment = gcFrame.loopEnvironment
                };
                tuuvm_stackFrame_pushRecord((tuuvm_stackFrameRecord_t*)&continueTargetRecord);
                tuuvm_environment_setContinueTarget(gcFrame.loopEnvironment, tuuvm_tuple_uintptr_encode(context, (uintptr_t)&breakTargetRecord));

                if(!setjmp(continueTargetRecord.jmpbuffer))
                {
                    tuuvm_interpreter_evaluateASTWithEnvironment(context, (*whileNode)->bodyExpression, gcFrame.loopEnvironment);
                }

                tuuvm_environment_setContinueTarget(gcFrame.loopEnvironment, TUUVM_NULL_TUPLE);
                tuuvm_stackFrame_popRecord((tuuvm_stackFrameRecord_t*)&continueTargetRecord);  
            }

            if((*whileNode)->continueExpression)
                tuuvm_interpreter_evaluateASTWithEnvironment(context, (*whileNode)->continueExpression, gcFrame.loopEnvironment);

            if((*whileNode)->conditionExpression)
                shouldContinue = tuuvm_tuple_boolean_decode(tuuvm_interpreter_evaluateASTWithEnvironment(context, (*whileNode)->conditionExpression, gcFrame.loopEnvironment));

            if(shouldContinue)
                tuuvm_gc_safepoint(context);
        }

        tuuvm_environment_setBreakTarget(gcFrame.loopEnvironment, TUUVM_NULL_TUPLE);
    }

    tuuvm_stackFrame_popRecord((tuuvm_stackFrameRecord_t*)&breakTargetRecord);  
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);

    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_astWhileContinueWithNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_tuple_t loopEnvironment;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.loopEnvironment = tuuvm_environment_create(context, *environment);

    tuuvm_astWhileContinueWithNode_t **whileNode = (tuuvm_astWhileContinueWithNode_t**)node;
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*whileNode)->super.sourcePosition);

    bool shouldContinue = true;
    
    tuuvm_stackFrameBreakTargetRecord_t breakTargetRecord = {
        .type = TUUVM_STACK_FRAME_RECORD_TYPE_BREAK_TARGET,
        .environment = gcFrame.loopEnvironment
    };
    tuuvm_stackFrame_pushRecord((tuuvm_stackFrameRecord_t*)&breakTargetRecord);  
    tuuvm_environment_setBreakTarget(gcFrame.loopEnvironment, tuuvm_tuple_uintptr_encode(context, (uintptr_t)&breakTargetRecord));

    if(!setjmp(breakTargetRecord.jmpbuffer))
    {
        if((*whileNode)->conditionExpression)
            shouldContinue = tuuvm_tuple_boolean_decode(tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*whileNode)->conditionExpression, gcFrame.loopEnvironment));

        while(shouldContinue)
        {
            if((*whileNode)->bodyExpression)
            {
                tuuvm_stackFrameContinueTargetRecord_t continueTargetRecord = {
                    .type = TUUVM_STACK_FRAME_RECORD_TYPE_CONTINUE_TARGET,
                    .environment = gcFrame.loopEnvironment
                };
                tuuvm_stackFrame_pushRecord((tuuvm_stackFrameRecord_t*)&continueTargetRecord);
                tuuvm_environment_setContinueTarget(gcFrame.loopEnvironment, tuuvm_tuple_uintptr_encode(context, (uintptr_t)&breakTargetRecord));

                if(!setjmp(continueTargetRecord.jmpbuffer))
                {
                    tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*whileNode)->bodyExpression, gcFrame.loopEnvironment);
                }

                tuuvm_environment_setContinueTarget(gcFrame.loopEnvironment, TUUVM_NULL_TUPLE);
                tuuvm_stackFrame_popRecord((tuuvm_stackFrameRecord_t*)&continueTargetRecord);  
            }

            if((*whileNode)->continueExpression)
                tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*whileNode)->continueExpression, gcFrame.loopEnvironment);

            if((*whileNode)->conditionExpression)
                shouldContinue = tuuvm_tuple_boolean_decode(tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*whileNode)->conditionExpression, gcFrame.loopEnvironment));

            if(shouldContinue)
                tuuvm_gc_safepoint(context);
        }

        tuuvm_environment_setBreakTarget(gcFrame.loopEnvironment, TUUVM_NULL_TUPLE);
    }

    tuuvm_stackFrame_popRecord((tuuvm_stackFrameRecord_t*)&breakTargetRecord);  
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);

    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_astBreakNode_primitiveMacro(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_tuple_t *macroContext = &arguments[0];

    struct {
        tuuvm_tuple_t sourcePosition;
        tuuvm_tuple_t result;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sourcePosition = tuuvm_macroContext_getSourcePosition(*macroContext);
    gcFrame.result = tuuvm_astBreakNode_create(context, gcFrame.sourcePosition);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astBreakNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    //tuuvm_tuple_t *environment = &arguments[1];

    return *node;
}

static tuuvm_tuple_t tuuvm_astBreakNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astBreakNode_t **breakNode = (tuuvm_astBreakNode_t**)node;
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*breakNode)->super.sourcePosition);

    tuuvm_tuple_t breakTarget = tuuvm_environment_lookBreakTargetRecursively(context, *environment);
    if(!breakTarget)
        tuuvm_error("No target available for break.");

    tuuvm_stackFrame_breakInto((tuuvm_stackFrameRecord_t*)tuuvm_tuple_uintptr_decode(breakTarget));
    return TUUVM_NULL_TUPLE;
}

static tuuvm_tuple_t tuuvm_astContinueNode_primitiveMacro(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_tuple_t *macroContext = &arguments[0];

    struct {
        tuuvm_tuple_t sourcePosition;
        tuuvm_tuple_t result;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sourcePosition = tuuvm_macroContext_getSourcePosition(*macroContext);
    gcFrame.result = tuuvm_astContinueNode_create(context, gcFrame.sourcePosition);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astContinueNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    //tuuvm_tuple_t *environment = &arguments[1];

    return *node;
}

static tuuvm_tuple_t tuuvm_astContinueNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astContinueNode_t **continueNode = (tuuvm_astContinueNode_t**)node;
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*continueNode)->super.sourcePosition);

    tuuvm_tuple_t continueTarget = tuuvm_environment_lookContinueTargetRecursively(context, *environment);
    if(!continueTarget)
        tuuvm_error("No target available for continue.");

    tuuvm_stackFrame_continueInto((tuuvm_stackFrameRecord_t*)tuuvm_tuple_uintptr_decode(continueTarget));
    return TUUVM_NULL_TUPLE;
}

static tuuvm_tuple_t tuuvm_astReturnNode_primitiveMacro(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *macroContext = &arguments[0];
    tuuvm_tuple_t *expression = &arguments[1];

    struct {
        tuuvm_tuple_t sourcePosition;
        tuuvm_tuple_t result;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sourcePosition = tuuvm_macroContext_getSourcePosition(*macroContext);
    gcFrame.result = tuuvm_astReturnNode_create(context, gcFrame.sourcePosition, *expression);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astReturnNode_primitiveAnalyze(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    struct {
        tuuvm_astReturnNode_t *returnNode;
        tuuvm_tuple_t analyzedExpression;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.returnNode = (tuuvm_astReturnNode_t*)tuuvm_context_shallowCopy(context, *node);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.returnNode->super.sourcePosition);

    if(gcFrame.returnNode->expression)
    {
        gcFrame.analyzedExpression = tuuvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.returnNode->expression, *environment);
        gcFrame.returnNode->expression = gcFrame.analyzedExpression;
    }

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.returnNode;
}

static tuuvm_tuple_t tuuvm_astReturnNode_primitiveEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astReturnNode_t **returnNode = (tuuvm_astReturnNode_t**)node;

    struct {
        tuuvm_tuple_t result;
    } gcFrame = {
        .result = TUUVM_VOID_TUPLE
    };
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*returnNode)->super.sourcePosition);

    if((*returnNode)->expression)
        gcFrame.result = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*returnNode)->expression, *environment);
    
    tuuvm_tuple_t returnTarget = tuuvm_environment_lookReturnTargetRecursively(context, *environment);
    if(!returnTarget)
        tuuvm_error("No target available for returning value.");

    tuuvm_stackFrame_returnValueInto(gcFrame.result, (tuuvm_stackFrameRecord_t*)tuuvm_tuple_uintptr_decode(returnTarget));
    return TUUVM_NULL_TUPLE;
}

static tuuvm_tuple_t tuuvm_astReturnNode_primitiveAnalyzeAndEvaluate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *environment = &arguments[1];

    tuuvm_astReturnNode_t **returnNode = (tuuvm_astReturnNode_t**)node;

    struct {
        tuuvm_tuple_t result;
    } gcFrame = {
        .result = TUUVM_VOID_TUPLE
    };
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*returnNode)->super.sourcePosition);

    if((*returnNode)->expression)
        gcFrame.result = tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*returnNode)->expression, *environment);
    
    tuuvm_tuple_t returnTarget = tuuvm_environment_lookReturnTargetRecursively(context, *environment);
    if(!returnTarget)
        tuuvm_error("No target available for returning value.");

    tuuvm_stackFrame_returnValueInto(gcFrame.result, (tuuvm_stackFrameRecord_t*)tuuvm_tuple_uintptr_decode(returnTarget));
    return TUUVM_NULL_TUPLE;
}

static void tuuvm_interpreter_evaluateArgumentNodeInEnvironment(tuuvm_context_t *context, size_t argumentNodeIndex, tuuvm_tuple_t argumentNode, tuuvm_tuple_t *activationEnvironment, tuuvm_tuple_t *argumentValue)
{
    struct {
        tuuvm_astArgumentNode_t *argumentNode;
        tuuvm_tuple_t expectedType;
        tuuvm_tuple_t value;
    } gcFrame = {
        .argumentNode = (tuuvm_astArgumentNode_t*)argumentNode,
        .value = *argumentValue
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.argumentNode->super.sourcePosition);

    if(gcFrame.argumentNode->type)
    {
        gcFrame.expectedType = tuuvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.argumentNode->type, *activationEnvironment);
        if(gcFrame.expectedType)
            gcFrame.value = tuuvm_type_coerceValue(context, gcFrame.expectedType, gcFrame.value);
    }

    tuuvm_functionActivationEnvironment_t **activationEnvironmentObject = (tuuvm_functionActivationEnvironment_t **)activationEnvironment;
    tuuvm_array_atPut((*activationEnvironmentObject)->valueVector, argumentNodeIndex, gcFrame.value);

    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
}

static tuuvm_tuple_t tuuvm_interpreter_evaluateResultTypeCoercionInEnvironment(tuuvm_context_t *context, tuuvm_functionDefinition_t **functionDefinition, tuuvm_tuple_t *environment, tuuvm_tuple_t result)
{
    if(!(*functionDefinition)->analyzedResultTypeNode)
        return result;

    struct {
        tuuvm_tuple_t resultType;
        tuuvm_tuple_t result;
    } gcFrame = {
        .result = result
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.resultType = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*functionDefinition)->analyzedResultTypeNode, *environment);
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
    tuuvm_function_t **closure = (tuuvm_function_t**)&functionActivationRecord.function;

    functionActivationRecord.functionDefinition = (*closure)->definition;
    tuuvm_functionDefinition_t **functionDefinition = (tuuvm_functionDefinition_t**)&functionActivationRecord.functionDefinition;

    TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*functionDefinition)->sourcePosition);

    tuuvm_functionDefinition_ensureAnalysis(context, functionDefinition);

    size_t expectedArgumentCount = tuuvm_array_getSize((*functionDefinition)->analyzedArgumentNodes);
    if(argumentCount != expectedArgumentCount)
        tuuvm_error_argumentCountMismatch(expectedArgumentCount, argumentCount);
    functionActivationRecord.applicationEnvironment = tuuvm_functionActivationEnvironment_create(context, TUUVM_NULL_TUPLE, functionActivationRecord.function);
    tuuvm_environment_setReturnTarget(functionActivationRecord.applicationEnvironment, tuuvm_tuple_uintptr_encode(context, (uintptr_t)&functionActivationRecord));

    // FIXME: Add support for the forall arguments here.
    for(size_t i = 0; i < argumentCount; ++i)
        tuuvm_interpreter_evaluateArgumentNodeInEnvironment(context, i, tuuvm_array_at((*functionDefinition)->analyzedArgumentNodes, i), &functionActivationRecord.applicationEnvironment, &arguments[i]);

    // Use setjmp for implementing the #return: statement.
    if(!setjmp(functionActivationRecord.jmpbuffer))
    {
        tuuvm_gc_safepoint(context);
        functionActivationRecord.result = tuuvm_interpreter_evaluateASTWithEnvironment(context, (*functionDefinition)->analyzedBodyNode, functionActivationRecord.applicationEnvironment);
    }

    functionActivationRecord.result = tuuvm_interpreter_evaluateResultTypeCoercionInEnvironment(context, functionDefinition, &functionActivationRecord.applicationEnvironment, functionActivationRecord.result);
    tuuvm_gc_safepoint(context);
    
    TUUVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    tuuvm_stackFrame_popRecord((tuuvm_stackFrameRecord_t*)&functionActivationRecord);  
    return functionActivationRecord.result;
}

TUUVM_API tuuvm_tuple_t tuuvm_interpreter_loadSourceNamedWithSolvedPath(tuuvm_context_t *context, tuuvm_tuple_t filename)
{
    struct {
        tuuvm_tuple_t filename;
        tuuvm_tuple_t sourceString;
        tuuvm_tuple_t sourceDirectory;
        tuuvm_tuple_t sourceName;
        tuuvm_tuple_t sourceLanguage;
        tuuvm_tuple_t sourceCode;
        tuuvm_tuple_t sourceEnvironment;
        tuuvm_tuple_t result;
    } gcFrame = {
        .filename = filename,
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.sourceString = tuuvm_io_readWholeFileNamedAsString(context, gcFrame.filename);
    gcFrame.sourceDirectory = tuuvm_filesystem_dirname(context, gcFrame.filename);
    gcFrame.sourceName = tuuvm_filesystem_basename(context, gcFrame.filename);
    gcFrame.sourceLanguage = tuuvm_sourceCode_inferLanguageFromSourceName(context, gcFrame.sourceName);
    gcFrame.sourceCode = tuuvm_sourceCode_create(context, gcFrame.sourceString, gcFrame.sourceDirectory, gcFrame.sourceName, gcFrame.sourceLanguage);
    gcFrame.sourceEnvironment = tuuvm_environment_createDefaultForSourceCodeEvaluation(context, gcFrame.sourceCode);
    gcFrame.result = tuuvm_interpreter_analyzeAndEvaluateSourceCodeWithEnvironment(context, gcFrame.sourceEnvironment, gcFrame.sourceCode);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_interpreter_primitive_loadSourceNamedWithSolvedPath(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_interpreter_loadSourceNamedWithSolvedPath(context, arguments[0]);
}

static tuuvm_tuple_t tuuvm_interpreter_primitive_loadSourceNamedMacro(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *macroContext = &arguments[0];
    tuuvm_tuple_t *sourceName = &arguments[1];

    tuuvm_tuple_t sourcePosition = tuuvm_macroContext_getSourcePosition(*macroContext);

    tuuvm_tuple_t sourceDirectory = tuuvm_astIdentifierReferenceNode_create(context, sourcePosition, tuuvm_symbol_internWithCString(context, "__SourceDirectory__"));
    tuuvm_tuple_t solveNameArguments = tuuvm_array_create(context, 2);
    tuuvm_array_atPut(solveNameArguments, 0, sourceDirectory);
    tuuvm_array_atPut(solveNameArguments, 1, *sourceName);

    tuuvm_tuple_t solveNameFunction = tuuvm_astIdentifierReferenceNode_create(context, sourcePosition, tuuvm_symbol_internWithCString(context, "FileSystem::joinPath:"));
    tuuvm_tuple_t solveNameCall = tuuvm_astUnexpandedApplicationNode_create(context, sourcePosition, solveNameFunction, solveNameArguments);

    tuuvm_tuple_t loadSourceArguments = tuuvm_array_create(context, 1);
    tuuvm_array_atPut(loadSourceArguments, 0, solveNameCall);

    tuuvm_tuple_t loadSourceNamedWithSolvedPath = tuuvm_astIdentifierReferenceNode_create(context, sourcePosition, tuuvm_symbol_internWithCString(context, "loadSourceNamedWithSolvedPath:"));
    tuuvm_tuple_t loadSourceCall = tuuvm_astUnexpandedApplicationNode_create(context, sourcePosition, loadSourceNamedWithSolvedPath, loadSourceArguments);
    return loadSourceCall;
}

void tuuvm_astInterpreter_registerPrimitives(void)
{
    tuuvm_primitiveTable_registerFunction(tuuvm_interpreter_primitive_loadSourceNamedWithSolvedPath);
    tuuvm_primitiveTable_registerFunction(tuuvm_interpreter_primitive_loadSourceNamedMacro);

    tuuvm_primitiveTable_registerFunction(tuuvm_astSequenceNode_primitiveMacro);

    tuuvm_primitiveTable_registerFunction(tuuvm_astArgumentNode_primitiveAnalyze);

    tuuvm_primitiveTable_registerFunction(tuuvm_astErrorNode_primitiveEvaluate);

    tuuvm_primitiveTable_registerFunction(tuuvm_astPragmaNode_primitiveAnalyze);

    tuuvm_primitiveTable_registerFunction(tuuvm_astSequenceNode_primitiveAnalyze);
    tuuvm_primitiveTable_registerFunction(tuuvm_astSequenceNode_primitiveEvaluate);
    tuuvm_primitiveTable_registerFunction(tuuvm_astSequenceNode_primitiveAnalyzeAndEvaluate);

    tuuvm_primitiveTable_registerFunction(tuuvm_astLiteralNode_primitiveAnalyze);
    tuuvm_primitiveTable_registerFunction(tuuvm_astLiteralNode_primitiveEvaluate);
    tuuvm_primitiveTable_registerFunction(tuuvm_astLiteralNode_primitiveEvaluate);

    tuuvm_primitiveTable_registerFunction(tuuvm_astIdentifierReferenceNode_primitiveAnalyze);
    tuuvm_primitiveTable_registerFunction(tuuvm_astIdentifierReferenceNode_primitiveEvaluate);
    tuuvm_primitiveTable_registerFunction(tuuvm_astIdentifierReferenceNode_primitiveEvaluate);

    tuuvm_primitiveTable_registerFunction(tuuvm_astUnexpandedApplicationNode_primitiveAnalyze);
    tuuvm_primitiveTable_registerFunction(tuuvm_astUnexpandedApplicationNode_primitiveAnalyzeAndEvaluate);
    tuuvm_primitiveTable_registerFunction(tuuvm_astUnexpandedApplicationNode_primitiveAnalyzeAndEvaluate);

    tuuvm_primitiveTable_registerFunction(tuuvm_astUnexpandedSExpressionNode_primitiveAnalyze);
    tuuvm_primitiveTable_registerFunction(tuuvm_astUnexpandedSExpressionNode_primitiveAnalyzeAndEvaluate);
    tuuvm_primitiveTable_registerFunction(tuuvm_astUnexpandedSExpressionNode_primitiveAnalyzeAndEvaluate);

    tuuvm_primitiveTable_registerFunction(tuuvm_astFunctionApplicationNode_primitiveAnalyze);
    tuuvm_primitiveTable_registerFunction(tuuvm_astFunctionApplicationNode_primitiveEvaluate);
    tuuvm_primitiveTable_registerFunction(tuuvm_astFunctionApplicationNode_primitiveAnalyzeAndEvaluate);

    tuuvm_primitiveTable_registerFunction(tuuvm_astLexicalBlockNode_primitiveAnalyze);
    tuuvm_primitiveTable_registerFunction(tuuvm_astLexicalBlockNode_primitiveEvaluate);
    tuuvm_primitiveTable_registerFunction(tuuvm_astLexicalBlockNode_primitiveAnalyzeAndEvaluate);

    tuuvm_primitiveTable_registerFunction(tuuvm_astMakeByteArrayNode_primitiveAnalyze);
    tuuvm_primitiveTable_registerFunction(tuuvm_astMakeByteArrayNode_primitiveEvaluate);
    tuuvm_primitiveTable_registerFunction(tuuvm_astMakeByteArrayNode_primitiveAnalyzeAndEvaluate);

    tuuvm_primitiveTable_registerFunction(tuuvm_astMakeTupleNode_primitiveAnalyze);
    tuuvm_primitiveTable_registerFunction(tuuvm_astMakeTupleNode_primitiveEvaluate);
    tuuvm_primitiveTable_registerFunction(tuuvm_astMakeTupleNode_primitiveAnalyzeAndEvaluate);

    tuuvm_primitiveTable_registerFunction(tuuvm_astMessageChainMessageNode_primitiveAnalyze);

    tuuvm_primitiveTable_registerFunction(tuuvm_astMessageChainNode_primitiveAnalyze);
    tuuvm_primitiveTable_registerFunction(tuuvm_astMessageChainNode_primitiveEvaluate);
    tuuvm_primitiveTable_registerFunction(tuuvm_astMessageChainNode_primitiveAnalyzeAndEvaluate);

    tuuvm_primitiveTable_registerFunction(tuuvm_astMessageSendNode_primitiveMacro);
    tuuvm_primitiveTable_registerFunction(tuuvm_astMessageSendNode_primitiveAnalyze);
    tuuvm_primitiveTable_registerFunction(tuuvm_astMessageSendNode_primitiveEvaluate);
    tuuvm_primitiveTable_registerFunction(tuuvm_astMessageSendNode_primitiveAnalyzeAndEvaluate);

    tuuvm_primitiveTable_registerFunction(tuuvm_astLambdaNode_primitiveMacro);
    tuuvm_primitiveTable_registerFunction(tuuvm_astLambdaNode_primitiveAnalyze);
    tuuvm_primitiveTable_registerFunction(tuuvm_astLambdaNode_primitiveEvaluate);
    tuuvm_primitiveTable_registerFunction(tuuvm_astLambdaNode_primitiveAnalyzeAndEvaluate);

    tuuvm_primitiveTable_registerFunction(tuuvm_astLocalDefinitionNode_primitiveMacro);
    tuuvm_primitiveTable_registerFunction(tuuvm_astLocalDefinitionNode_primitiveDefineMacro);
    tuuvm_primitiveTable_registerFunction(tuuvm_astLocalDefinitionNode_letWithPrimitiveMacro);
    tuuvm_primitiveTable_registerFunction(tuuvm_astLocalDefinitionNode_primitiveAnalyze);
    tuuvm_primitiveTable_registerFunction(tuuvm_astLocalDefinitionNode_primitiveEvaluate);
    tuuvm_primitiveTable_registerFunction(tuuvm_astLocalDefinitionNode_primitiveAnalyzeAndEvaluate);

    tuuvm_primitiveTable_registerFunction(tuuvm_astIfNode_primitiveMacro);
    tuuvm_primitiveTable_registerFunction(tuuvm_astIfNode_primitiveMacroIfThen);
    tuuvm_primitiveTable_registerFunction(tuuvm_astIfNode_primitiveAnalyze);
    tuuvm_primitiveTable_registerFunction(tuuvm_astIfNode_primitiveEvaluate);
    tuuvm_primitiveTable_registerFunction(tuuvm_astIfNode_primitiveAnalyzeAndEvaluate);

    tuuvm_primitiveTable_registerFunction(tuuvm_astDoWhileContinueWithNode_primitiveMacro);
    tuuvm_primitiveTable_registerFunction(tuuvm_astDoWhileContinueWithNode_primitiveAnalyze);
    tuuvm_primitiveTable_registerFunction(tuuvm_astDoWhileContinueWithNode_primitiveEvaluate);
    tuuvm_primitiveTable_registerFunction(tuuvm_astDoWhileContinueWithNode_primitiveAnalyzeAndEvaluate);

    tuuvm_primitiveTable_registerFunction(tuuvm_astWhileContinueWithNode_primitiveMacro);
    tuuvm_primitiveTable_registerFunction(tuuvm_astWhileContinueWithNode_primitiveAnalyze);
    tuuvm_primitiveTable_registerFunction(tuuvm_astWhileContinueWithNode_primitiveEvaluate);
    tuuvm_primitiveTable_registerFunction(tuuvm_astWhileContinueWithNode_primitiveAnalyzeAndEvaluate);

    tuuvm_primitiveTable_registerFunction(tuuvm_astBreakNode_primitiveMacro);
    tuuvm_primitiveTable_registerFunction(tuuvm_astBreakNode_primitiveAnalyze);
    tuuvm_primitiveTable_registerFunction(tuuvm_astBreakNode_primitiveEvaluate);
    tuuvm_primitiveTable_registerFunction(tuuvm_astBreakNode_primitiveEvaluate);

    tuuvm_primitiveTable_registerFunction(tuuvm_astContinueNode_primitiveMacro);
    tuuvm_primitiveTable_registerFunction(tuuvm_astContinueNode_primitiveAnalyze);
    tuuvm_primitiveTable_registerFunction(tuuvm_astContinueNode_primitiveEvaluate);
    tuuvm_primitiveTable_registerFunction(tuuvm_astContinueNode_primitiveEvaluate);

    tuuvm_primitiveTable_registerFunction(tuuvm_astReturnNode_primitiveMacro);
    tuuvm_primitiveTable_registerFunction(tuuvm_astReturnNode_primitiveAnalyze);
    tuuvm_primitiveTable_registerFunction(tuuvm_astReturnNode_primitiveEvaluate);
    tuuvm_primitiveTable_registerFunction(tuuvm_astReturnNode_primitiveAnalyzeAndEvaluate);

    tuuvm_primitiveTable_registerFunction(tuuvm_functionDefinition_primitiveEnsureAnalysis);
}

static void tuuvm_astInterpreter_setupNodeInterpretationFunctions(tuuvm_context_t *context, tuuvm_tuple_t astNodeType, tuuvm_functionEntryPoint_t analysisFunction, tuuvm_functionEntryPoint_t evaluationFunction, tuuvm_functionEntryPoint_t analysisAndEvaluationFunction)
{
    if(analysisFunction)
        tuuvm_type_setAstNodeAnalysisFunction(context, astNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, analysisFunction));
    if(evaluationFunction)
        tuuvm_type_setAstNodeEvaluationFunction(context, astNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, evaluationFunction));
    if(analysisAndEvaluationFunction)
        tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(context, astNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, analysisAndEvaluationFunction));
}

void tuuvm_astInterpreter_setupASTInterpreter(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "loadSourceNamedWithSolvedPath:", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_interpreter_primitive_loadSourceNamedWithSolvedPath);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "loadSourceNamed:", 2, TUUVM_FUNCTION_FLAGS_MACRO | TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_interpreter_primitive_loadSourceNamedMacro);

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "begin", 2, TUUVM_FUNCTION_FLAGS_MACRO | TUUVM_FUNCTION_FLAGS_VARIADIC, NULL, tuuvm_astSequenceNode_primitiveMacro);

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "FunctionDefinition::ensureAnalysis", context->roots.functionDefinitionType, "ensureAnalysis", 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_functionDefinition_primitiveEnsureAnalysis);

    // FIXME: Collapse these into more convenient and shorter calls.
    tuuvm_type_setAstNodeAnalysisFunction(context, context->roots.astArgumentNodeType, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_astArgumentNode_primitiveAnalyze));

    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astErrorNodeType,
        tuuvm_astErrorNode_primitiveEvaluate,
        tuuvm_astErrorNode_primitiveEvaluate,
        tuuvm_astErrorNode_primitiveEvaluate
    );
    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astPragmaNodeType,
        tuuvm_astPragmaNode_primitiveAnalyze,
        NULL,
        NULL
    );
    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astSequenceNodeType,
        tuuvm_astSequenceNode_primitiveAnalyze,
        tuuvm_astSequenceNode_primitiveEvaluate,
        tuuvm_astSequenceNode_primitiveAnalyzeAndEvaluate
    );
    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astLiteralNodeType,
        tuuvm_astLiteralNode_primitiveAnalyze,
        tuuvm_astLiteralNode_primitiveEvaluate,
        tuuvm_astLiteralNode_primitiveEvaluate
    );
    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astIdentifierReferenceNodeType,
        tuuvm_astIdentifierReferenceNode_primitiveAnalyze,
        tuuvm_astIdentifierReferenceNode_primitiveEvaluate,
        tuuvm_astIdentifierReferenceNode_primitiveAnalyzeAndEvaluate
    );
    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astUnexpandedApplicationNodeType,
        tuuvm_astUnexpandedApplicationNode_primitiveAnalyze,
        tuuvm_astUnexpandedApplicationNode_primitiveAnalyzeAndEvaluate,
        tuuvm_astUnexpandedApplicationNode_primitiveAnalyzeAndEvaluate
    );
    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astUnexpandedSExpressionNodeType,
        tuuvm_astUnexpandedSExpressionNode_primitiveAnalyze,
        tuuvm_astUnexpandedSExpressionNode_primitiveAnalyzeAndEvaluate,
        tuuvm_astUnexpandedSExpressionNode_primitiveAnalyzeAndEvaluate
    );
    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astFunctionApplicationNodeType,
        tuuvm_astFunctionApplicationNode_primitiveAnalyze,
        tuuvm_astFunctionApplicationNode_primitiveEvaluate,
        tuuvm_astFunctionApplicationNode_primitiveAnalyzeAndEvaluate
    );
    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astLexicalBlockNodeType,
        tuuvm_astLexicalBlockNode_primitiveAnalyze,
        tuuvm_astLexicalBlockNode_primitiveEvaluate,
        tuuvm_astLexicalBlockNode_primitiveAnalyzeAndEvaluate
    );
    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astMakeByteArrayNodeType,
        tuuvm_astMakeByteArrayNode_primitiveAnalyze,
        tuuvm_astMakeByteArrayNode_primitiveEvaluate,
        tuuvm_astMakeByteArrayNode_primitiveAnalyzeAndEvaluate
    );
    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astMakeTupleNodeType,
        tuuvm_astMakeTupleNode_primitiveAnalyze,
        tuuvm_astMakeTupleNode_primitiveEvaluate,
        tuuvm_astMakeTupleNode_primitiveAnalyzeAndEvaluate
    );
    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astMessageChainMessageNodeType,
        tuuvm_astMessageChainMessageNode_primitiveAnalyze,
        NULL,
        NULL
    );
    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astMessageChainNodeType,
        tuuvm_astMessageChainNode_primitiveAnalyze,
        tuuvm_astMessageChainNode_primitiveEvaluate,
        tuuvm_astMessageChainNode_primitiveAnalyzeAndEvaluate
    );

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "send", 4, TUUVM_FUNCTION_FLAGS_MACRO | TUUVM_FUNCTION_FLAGS_VARIADIC, NULL, tuuvm_astMessageSendNode_primitiveMacro);
    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astMessageSendNodeType,
        tuuvm_astMessageSendNode_primitiveAnalyze,
        tuuvm_astMessageSendNode_primitiveEvaluate,
        tuuvm_astMessageSendNode_primitiveAnalyzeAndEvaluate
    );

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "lambda", 3, TUUVM_FUNCTION_FLAGS_MACRO | TUUVM_FUNCTION_FLAGS_VARIADIC, NULL, tuuvm_astLambdaNode_primitiveMacro);
    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astLambdaNodeType,
        tuuvm_astLambdaNode_primitiveAnalyze,
        tuuvm_astLambdaNode_primitiveEvaluate,
        tuuvm_astLambdaNode_primitiveAnalyzeAndEvaluate
    );

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "define", 3, TUUVM_FUNCTION_FLAGS_MACRO | TUUVM_FUNCTION_FLAGS_VARIADIC, NULL, tuuvm_astLocalDefinitionNode_primitiveMacro);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "defineMacro", 3, TUUVM_FUNCTION_FLAGS_MACRO | TUUVM_FUNCTION_FLAGS_VARIADIC, NULL, tuuvm_astLocalDefinitionNode_primitiveDefineMacro);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "let:with:", 3, TUUVM_FUNCTION_FLAGS_MACRO, NULL, tuuvm_astLocalDefinitionNode_letWithPrimitiveMacro);
    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astLocalDefinitionNodeType,
        tuuvm_astLocalDefinitionNode_primitiveAnalyze,
        tuuvm_astLocalDefinitionNode_primitiveEvaluate,
        tuuvm_astLocalDefinitionNode_primitiveAnalyzeAndEvaluate
    );

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "if:then:else:", 4, TUUVM_FUNCTION_FLAGS_MACRO, NULL, tuuvm_astIfNode_primitiveMacro);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "if:then:", 3, TUUVM_FUNCTION_FLAGS_MACRO, NULL, tuuvm_astIfNode_primitiveMacroIfThen);
    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astIfNodeType,
        tuuvm_astIfNode_primitiveAnalyze,
        tuuvm_astIfNode_primitiveEvaluate,
        tuuvm_astIfNode_primitiveAnalyzeAndEvaluate
    );

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "do:while:continueWith:", 4, TUUVM_FUNCTION_FLAGS_MACRO, NULL, tuuvm_astDoWhileContinueWithNode_primitiveMacro);
    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astDoWhileContinueWithNodeType,
        tuuvm_astDoWhileContinueWithNode_primitiveAnalyze,
        tuuvm_astDoWhileContinueWithNode_primitiveEvaluate,
        tuuvm_astDoWhileContinueWithNode_primitiveAnalyzeAndEvaluate
    );

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "while:do:continueWith:", 4, TUUVM_FUNCTION_FLAGS_MACRO, NULL, tuuvm_astWhileContinueWithNode_primitiveMacro);
    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astWhileContinueWithNodeType,
        tuuvm_astWhileContinueWithNode_primitiveAnalyze,
        tuuvm_astWhileContinueWithNode_primitiveEvaluate,
        tuuvm_astWhileContinueWithNode_primitiveAnalyzeAndEvaluate
    );

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "break", 1, TUUVM_FUNCTION_FLAGS_MACRO, NULL, tuuvm_astBreakNode_primitiveMacro);
    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astBreakNodeType,
        tuuvm_astBreakNode_primitiveAnalyze,
        tuuvm_astBreakNode_primitiveEvaluate,
        tuuvm_astBreakNode_primitiveEvaluate
    );

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "continue", 1, TUUVM_FUNCTION_FLAGS_MACRO, NULL, tuuvm_astContinueNode_primitiveMacro);
    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astContinueNodeType,
        tuuvm_astContinueNode_primitiveAnalyze,
        tuuvm_astContinueNode_primitiveEvaluate,
        tuuvm_astContinueNode_primitiveEvaluate
    );

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "return:", 2, TUUVM_FUNCTION_FLAGS_MACRO, NULL, tuuvm_astReturnNode_primitiveMacro);
    tuuvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astReturnNodeType,
        tuuvm_astReturnNode_primitiveAnalyze,
        tuuvm_astReturnNode_primitiveEvaluate,
        tuuvm_astReturnNode_primitiveAnalyzeAndEvaluate
    );
}
