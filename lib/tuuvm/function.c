#include "tuuvm/function.h"
#include "tuuvm/array.h"
#include "tuuvm/assert.h"
#include "tuuvm/bytecode.h"
#include "tuuvm/bytecodeCompiler.h"
#include "tuuvm/dictionary.h"
#include "tuuvm/errors.h"
#include "tuuvm/interpreter.h"
#include "tuuvm/stackFrame.h"
#include "tuuvm/string.h"
#include "tuuvm/type.h"
#include "internal/context.h"
#include <stdlib.h>
#include <stdio.h>

#define PRIMITIVE_TABLE_CAPACITY 1024

static bool tuuvm_primitiveTableIsComputed = false;
static size_t tuuvm_primitiveTableSize = 0;
static bool tuuvm_checkPrimitivesArePresentInTable = true;
static bool tuuvm_function_useBytecodeInterpreter = true;

typedef struct tuuvm_primitiveTableEntry_s
{
    tuuvm_functionEntryPoint_t entryPoint;
    const char *name;
} tuuvm_primitiveTableEntry_t;

static tuuvm_primitiveTableEntry_t tuuvm_primitiveTable[PRIMITIVE_TABLE_CAPACITY];

extern void tuuvm_context_registerPrimitives(void);
extern tuuvm_tuple_t tuuvm_interpreter_recompileAndOptimizeFunction(tuuvm_context_t *context, tuuvm_function_t **functionObject);

void tuuvm_primitiveTable_registerFunction(tuuvm_functionEntryPoint_t primitiveEntryPoint, const char *primitiveName)
{
    for(size_t i = 0; i < tuuvm_primitiveTableSize; ++i)
    {
        if(tuuvm_primitiveTable[i].entryPoint == primitiveEntryPoint)
            return;
    }

    TUUVM_ASSERT(tuuvm_primitiveTableSize < PRIMITIVE_TABLE_CAPACITY);
    tuuvm_primitiveTable[tuuvm_primitiveTableSize].entryPoint = primitiveEntryPoint;
    tuuvm_primitiveTable[tuuvm_primitiveTableSize].name = primitiveName;
    ++tuuvm_primitiveTableSize;
}

static void tuuvm_primitiveTable_ensureIsComputed(void)
{
    if(tuuvm_primitiveTableIsComputed)
        return;

    tuuvm_primitiveTableIsComputed = true;
    tuuvm_context_registerPrimitives();
}

static bool tuuvm_primitiveTable_findEntryFor(tuuvm_functionEntryPoint_t primitiveEntryPoint, size_t *outEntryIndex)
{
    tuuvm_primitiveTable_ensureIsComputed();
    
    for(size_t i = 0; i < tuuvm_primitiveTableSize; ++i)
    {
        if(tuuvm_primitiveTable[i].entryPoint == primitiveEntryPoint)
        {
            *outEntryIndex = i;
            return true;
        }
    }
    return false;
}

TUUVM_API tuuvm_tuple_t tuuvm_functionDefinition_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t flags, tuuvm_tuple_t argumentCount, tuuvm_tuple_t definitionEnvironment, tuuvm_tuple_t argumentNodes, tuuvm_tuple_t resultTypeNode, tuuvm_tuple_t body)
{
    tuuvm_functionDefinition_t *result = (tuuvm_functionDefinition_t*)tuuvm_context_allocatePointerTuple(context, context->roots.functionDefinitionType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_functionDefinition_t));
    result->sourcePosition = sourcePosition;
    result->flags = flags;
    result->argumentCount = argumentCount; 
    result->definitionEnvironment = definitionEnvironment;
    result->definitionArgumentNodes = argumentNodes;
    result->definitionResultTypeNode = resultTypeNode;
    result->definitionBodyNode = body;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_function_createPrimitive(tuuvm_context_t *context, size_t argumentCount, tuuvm_bitflags_t flags, void *userdata, tuuvm_functionEntryPoint_t entryPoint)
{
    tuuvm_function_t *result = (tuuvm_function_t*)tuuvm_context_allocatePointerTuple(context, context->roots.functionType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_function_t));
    result->argumentCount = tuuvm_tuple_size_encode(context, argumentCount);
    result->flags = tuuvm_tuple_bitflags_encode(flags);
    size_t primitiveEntryIndex = 0;
    if(!userdata && tuuvm_primitiveTable_findEntryFor(entryPoint, &primitiveEntryIndex))
    {
        result->primitiveTableIndex = tuuvm_tuple_uint32_encode(context, primitiveEntryIndex + 1);
        const char *primitiveName = tuuvm_primitiveTable[primitiveEntryIndex].name;
        if(primitiveName)
            result->primitiveName = tuuvm_symbol_internWithCString(context, primitiveName);
    }
    else
    {
        TUUVM_ASSERT(!tuuvm_checkPrimitivesArePresentInTable);
        result->nativeUserdata = tuuvm_tuple_systemHandle_encode(context, (intptr_t)userdata);
        result->nativeEntryPoint = tuuvm_tuple_systemHandle_encode(context, (intptr_t)entryPoint);
    }
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_function_createClosureWithCaptureVector(tuuvm_context_t *context, tuuvm_tuple_t functionDefinition, tuuvm_tuple_t captureVector)
{
    if(!tuuvm_tuple_isKindOf(context, functionDefinition, context->roots.functionDefinitionType))
        tuuvm_error("An actual function definition is required here.");

    tuuvm_functionDefinition_t *functionDefinitionObject = (tuuvm_functionDefinition_t*)functionDefinition;
    TUUVM_ASSERT(functionDefinitionObject->analyzedType);

    tuuvm_function_t *result = (tuuvm_function_t*)tuuvm_context_allocatePointerTuple(context, functionDefinitionObject->analyzedType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_function_t));
    result->flags = functionDefinitionObject->flags;
    result->argumentCount = functionDefinitionObject->argumentCount; 
    result->captureVector = captureVector;
    result->definition = functionDefinition;
    result->primitiveName = functionDefinitionObject->analyzedPrimitiveName;
    return (tuuvm_tuple_t)result;
}

TUUVM_API size_t tuuvm_function_getArgumentCount(tuuvm_context_t *context, tuuvm_tuple_t function)
{
    if(tuuvm_tuple_isFunction(context, function))
    {
        tuuvm_function_t *functionObject = (tuuvm_function_t*)function;
        return tuuvm_tuple_size_decode(functionObject->argumentCount);
    }

    return 0;
}

TUUVM_API void tuuvm_function_setFlags(tuuvm_context_t *context, tuuvm_tuple_t function, tuuvm_bitflags_t flags)
{
    if(!tuuvm_tuple_isFunction(context, function))
        tuuvm_error("Expected a function.");

    tuuvm_function_t *functionObject = (tuuvm_function_t*)function;
    functionObject->flags = tuuvm_tuple_bitflags_encode(flags);
}

TUUVM_API void tuuvm_function_addFlags(tuuvm_context_t *context, tuuvm_tuple_t function, tuuvm_bitflags_t flags)
{
    if(!tuuvm_tuple_isFunction(context, function))
        tuuvm_error("Expected a function.");

    tuuvm_function_t *functionObject = (tuuvm_function_t*)function;
    functionObject->flags = tuuvm_tuple_bitflags_encode(tuuvm_tuple_bitflags_decode(functionObject->flags) | flags);
}

TUUVM_API tuuvm_tuple_t tuuvm_ordinaryFunction_nativeApply(tuuvm_context_t *context, tuuvm_tuple_t function, tuuvm_functionEntryPoint_t nativeEntryPoint, size_t argumentCount, tuuvm_tuple_t *arguments, uint32_t applicationFlags)
{
    (void)applicationFlags;
    tuuvm_stackFrameGCRootsRecord_t argumentsRecord = {
        .type = TUUVM_STACK_FRAME_RECORD_TYPE_GC_ROOTS,
        .rootCount = argumentCount,
        .roots = arguments
    };
    tuuvm_stackFrame_pushRecord((tuuvm_stackFrameRecord_t*)&argumentsRecord);

    tuuvm_tuple_t result = nativeEntryPoint(context, function, argumentCount, arguments);
        
    tuuvm_stackFrame_popRecord((tuuvm_stackFrameRecord_t*)&argumentsRecord);
    return result;
}

TUUVM_API tuuvm_functionEntryPoint_t tuuvm_function_getNumberedPrimitiveEntryPoint(tuuvm_context_t *context, uint32_t primitiveNumber)
{
    (void)context;
    if(primitiveNumber > 0 && primitiveNumber <= tuuvm_primitiveTableSize)
        return tuuvm_primitiveTable[primitiveNumber - 1].entryPoint;
    return NULL;
}

TUUVM_API tuuvm_tuple_t tuuvm_ordinaryFunction_directApply(tuuvm_context_t *context, tuuvm_tuple_t function, size_t argumentCount, tuuvm_tuple_t *arguments, uint32_t applicationFlags)
{
    tuuvm_function_t *functionObject = (tuuvm_function_t*)function;

    // Find the entry point in the primitive table.
    if(functionObject->primitiveTableIndex)
    {
        tuuvm_primitiveTable_ensureIsComputed();
        uint32_t primitiveNumber = tuuvm_tuple_uint32_decode(functionObject->primitiveTableIndex);
        if(primitiveNumber > 0 && primitiveNumber <= tuuvm_primitiveTableSize)
            return tuuvm_ordinaryFunction_nativeApply(context, function, tuuvm_primitiveTable[primitiveNumber - 1].entryPoint, argumentCount, arguments, applicationFlags);
    }

    if(functionObject->nativeEntryPoint)
        return tuuvm_ordinaryFunction_nativeApply(context, function, (tuuvm_functionEntryPoint_t)(uintptr_t)tuuvm_tuple_systemHandle_decode(functionObject->nativeEntryPoint), argumentCount, arguments, applicationFlags);
    
    if(!functionObject->definition)
        tuuvm_error("Cannot apply a function without a proper definition.");

    // Attempt to use the bytecode.
    tuuvm_functionDefinition_t *definition = (tuuvm_functionDefinition_t*)functionObject->definition;
    if(tuuvm_function_useBytecodeInterpreter && definition->bytecode)
        return tuuvm_bytecodeInterpreter_apply(context, function, argumentCount, arguments);
    
    return tuuvm_interpreter_applyClosureASTFunction(context, function, argumentCount, arguments, applicationFlags);
}

TUUVM_API tuuvm_tuple_t tuuvm_ordinaryFunction_memoizedApply(tuuvm_context_t *context, tuuvm_tuple_t function_, size_t argumentCount, tuuvm_tuple_t *arguments, uint32_t applicationFlags)
{
    struct {
        tuuvm_tuple_t function;
        tuuvm_tuple_t memoizationKey;
        tuuvm_tuple_t result;
    } gcFrame = {
        .function = function_
    };
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    tuuvm_stackFrameGCRootsRecord_t argumentsRecord = {
        .type = TUUVM_STACK_FRAME_RECORD_TYPE_GC_ROOTS,
        .rootCount = argumentCount,
        .roots = arguments
    };
    tuuvm_stackFrame_pushRecord((tuuvm_stackFrameRecord_t*)&argumentsRecord);

    tuuvm_function_t **functionObject = (tuuvm_function_t**)&gcFrame.function;

    // Make the memoization lookup key.
    if(argumentCount > 1)
    {
        gcFrame.memoizationKey = tuuvm_array_create(context, argumentCount);
        for(size_t i = 0; i < argumentCount; ++i)
            tuuvm_array_atPut(gcFrame.memoizationKey, i , arguments[i]);
    }
    else if(argumentCount == 1)
    {
        gcFrame.memoizationKey = arguments[0];
    }

    // Find the result in the memoization table
    if((*functionObject)->memoizationTable)
    {
        if(tuuvm_weakValueDictionary_find(context, (*functionObject)->memoizationTable, gcFrame.memoizationKey, &gcFrame.result))
        {
            if(gcFrame.result == TUUVM_PENDING_MEMOIZATION_VALUE)
                tuuvm_error("Computing cyclic memoized value.");

            tuuvm_stackFrame_popRecord((tuuvm_stackFrameRecord_t*)&argumentsRecord);
            TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return gcFrame.result;
        }
        else
        {
            tuuvm_weakValueDictionary_atPut(context, (*functionObject)->memoizationTable, gcFrame.memoizationKey, TUUVM_PENDING_MEMOIZATION_VALUE);
        }
    }
    else
    {
        (*functionObject)->memoizationTable = tuuvm_weakValueDictionary_create(context);
    }

    // Apply the actual function.
    tuuvm_stackFrame_popRecord((tuuvm_stackFrameRecord_t*)&argumentsRecord);
    gcFrame.result = tuuvm_ordinaryFunction_directApply(context, gcFrame.function, argumentCount, arguments, applicationFlags);

    // Store the result
    tuuvm_weakValueDictionary_atPut(context, (*functionObject)->memoizationTable, gcFrame.memoizationKey, gcFrame.result);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

TUUVM_API tuuvm_tuple_t tuuvm_ordinaryFunction_apply(tuuvm_context_t *context, tuuvm_tuple_t function, size_t argumentCount, tuuvm_tuple_t *arguments, uint32_t applicationFlags)
{
    if(tuuvm_function_isMemoized(context, function))
        return tuuvm_ordinaryFunction_memoizedApply(context, function, argumentCount, arguments, applicationFlags);
    return tuuvm_ordinaryFunction_directApply(context, function, argumentCount, arguments, applicationFlags);
}

TUUVM_API tuuvm_tuple_t tuuvm_function_apply(tuuvm_context_t *context, tuuvm_tuple_t function, size_t argumentCount, tuuvm_tuple_t *arguments, uint32_t applicationFlags)
{
    // Is this an ordinary function?
    if(tuuvm_tuple_isFunction(context, function))
        return tuuvm_ordinaryFunction_apply(context, function, argumentCount, arguments, applicationFlags);

    if(!function)
        tuuvm_error("Cannot apply nil as a function.");

    // Send the #() and #(): messages to the functional object.
    if(argumentCount == 0)
    {
        return tuuvm_tuple_send0(context, context->roots.applyWithoutArgumentsSelector, function);
    }
    else
    {
        tuuvm_tuple_t argumentsArray = tuuvm_array_create(context, argumentCount);
        for(size_t i = 0; i < argumentCount; ++i)
            tuuvm_array_atPut(argumentsArray, i, arguments[i]);

        return tuuvm_tuple_send1(context, context->roots.applyWithArgumentsSelector, function, argumentsArray);
    }
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_send(tuuvm_context_t *context, tuuvm_tuple_t selector, size_t argumentCount, tuuvm_tuple_t *arguments, uint32_t applicationFlags)
{
    TUUVM_ASSERT(argumentCount > 0); // We need a receiver for performing the lookup.
    struct {
        tuuvm_tuple_t method;
        tuuvm_tuple_t receiverType;
    } gcFrame = {
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.receiverType = tuuvm_tuple_getType(context, arguments[0]);
    gcFrame.method = tuuvm_type_lookupSelector(context, gcFrame.receiverType, selector);
    if(!gcFrame.method)
    {
        tuuvm_tuple_t receiverTypeName = tuuvm_tuple_printString(context, gcFrame.receiverType);
        tuuvm_errorWithMessageTuple(tuuvm_string_concat(context, tuuvm_string_createWithCString(context, "Message notUnderstood by "), receiverTypeName));
    }
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    
    return tuuvm_function_apply(context, gcFrame.method, argumentCount, arguments, applicationFlags);
}

TUUVM_API void tuuvm_functionCallFrameStack_begin(tuuvm_context_t *context, tuuvm_functionCallFrameStack_t *callFrameStack, tuuvm_tuple_t function, size_t argumentCount)
{
    callFrameStack->gcRoots.function = function;
    callFrameStack->isVariadic = tuuvm_function_isVariadic(context, callFrameStack->gcRoots.function);
    callFrameStack->expectedArgumentCount = tuuvm_function_getArgumentCount(context, callFrameStack->gcRoots.function);
    callFrameStack->argumentIndex = 0;
    callFrameStack->variadicArgumentIndex = 0;

    size_t requiredArgumentCount = callFrameStack->expectedArgumentCount;
    if(callFrameStack->isVariadic)
    {
        if(requiredArgumentCount == 0)
            tuuvm_error("Variadic functions require at least a single argument.");
        callFrameStack->variadicArgumentIndex = requiredArgumentCount - 1;

        --requiredArgumentCount;
        if(argumentCount < requiredArgumentCount)
            tuuvm_error("Missing required argument count.");

        size_t variadicArgumentCount = argumentCount - requiredArgumentCount;
        callFrameStack->gcRoots.applicationArguments[callFrameStack->variadicArgumentIndex] = tuuvm_array_create(context, variadicArgumentCount);
    }
    else
    {
        if(argumentCount != requiredArgumentCount)
            tuuvm_error("Function call does not receive the required number of arguments.");
    }

    if(argumentCount > TUUVM_MAX_FUNCTION_ARGUMENTS && !callFrameStack->isVariadic)
        tuuvm_error("Function application direct arguments exceeds the max argument count.");
    
}

TUUVM_API void tuuvm_functionCallFrameStack_push(tuuvm_functionCallFrameStack_t *callFrameStack, tuuvm_tuple_t argument)
{
    if(!callFrameStack->isVariadic || callFrameStack->argumentIndex < callFrameStack->variadicArgumentIndex)
    {
        callFrameStack->gcRoots.applicationArguments[callFrameStack->argumentIndex++] = argument;
        return;
    }

    tuuvm_array_atPut(callFrameStack->gcRoots.applicationArguments[callFrameStack->variadicArgumentIndex], callFrameStack->argumentIndex - callFrameStack->variadicArgumentIndex, argument);
    ++callFrameStack->argumentIndex;
}

TUUVM_API tuuvm_tuple_t tuuvm_functionCallFrameStack_finish(tuuvm_context_t *context, tuuvm_functionCallFrameStack_t *callFrameStack, uint32_t applicationFlags)
{
    return tuuvm_function_apply(context, callFrameStack->gcRoots.function, callFrameStack->expectedArgumentCount, callFrameStack->gcRoots.applicationArguments, applicationFlags);
}

static tuuvm_tuple_t tuuvm_function_primitive_apply(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *function = &arguments[0];
    tuuvm_tuple_t *argumentList = &arguments[1];

    size_t variadicArgumentCount = tuuvm_array_getSize(*argumentList);
    size_t argumentListSize = 0;
    size_t callArgumentCount = 0;
    if(variadicArgumentCount > 0)
    {
        argumentListSize = tuuvm_array_getSize(tuuvm_array_at(*argumentList, variadicArgumentCount - 1));
        callArgumentCount = variadicArgumentCount - 1 + argumentListSize;
    }

    tuuvm_functionCallFrameStack_t callFrameStack = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    tuuvm_functionCallFrameStack_begin(context, &callFrameStack, *function, callArgumentCount);
    if(variadicArgumentCount > 0)
    {
        for(size_t i = 0; i < variadicArgumentCount - 1; ++i)
            tuuvm_functionCallFrameStack_push(&callFrameStack, tuuvm_array_at(*argumentList, i));
        
        tuuvm_tuple_t argumentArray = tuuvm_array_at(*argumentList, variadicArgumentCount - 1);
        for(size_t i = 0; i < argumentListSize; ++i)
            tuuvm_functionCallFrameStack_push(&callFrameStack, tuuvm_array_at(argumentArray, i));
    }

    TUUVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    return tuuvm_functionCallFrameStack_finish(context, &callFrameStack, 0);
}

static tuuvm_tuple_t tuuvm_function_primitive_isCorePrimitive(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_tuple_t *function = &arguments[0];
    if(!tuuvm_tuple_isFunction(context, *function))
        return TUUVM_FALSE_TUPLE;
    
    tuuvm_function_t *functionObject = (tuuvm_function_t*)*function;
    size_t flags = tuuvm_tuple_bitflags_decode(functionObject->flags);
    return tuuvm_tuple_boolean_encode((flags & TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE) != 0);
}

static tuuvm_tuple_t tuuvm_function_primitive_adoptDefinitionOf(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *function = &arguments[0];
    tuuvm_tuple_t *definitionFunction = &arguments[1];
    if(!tuuvm_tuple_isFunction(context, *function)) tuuvm_error("Expected a function.");
    if(!tuuvm_tuple_isFunction(context, *definitionFunction)) tuuvm_error("Expected a function.");
    
    tuuvm_function_t **functionObject = (tuuvm_function_t**)function;
    tuuvm_function_t **definitionFunctionObject = (tuuvm_function_t**)definitionFunction;

    (*functionObject)->definition = (*definitionFunctionObject)->definition;
    (*functionObject)->captureVector = (*definitionFunctionObject)->captureVector;
    if((*definitionFunctionObject)->primitiveName)
        (*functionObject)->primitiveName = (*definitionFunctionObject)->primitiveName;
    tuuvm_tuple_setType((tuuvm_object_tuple_t*)*functionObject, tuuvm_tuple_getType(context, (tuuvm_tuple_t)*definitionFunctionObject));
    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_function_primitive_recompileAndOptimize(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *function = &arguments[0];
    if(!tuuvm_tuple_isFunction(context, *function)) tuuvm_error("Expected a function.");
    
    tuuvm_function_t **functionObject = (tuuvm_function_t**)function;
    if((*functionObject)->definition && tuuvm_array_getSize((*functionObject)->captureVector) > 0)
        return tuuvm_interpreter_recompileAndOptimizeFunction(context, functionObject);
    
    return *function;
}

bool tuuvm_function_shouldOptimizeLookup(tuuvm_context_t *context, tuuvm_tuple_t function, tuuvm_tuple_t receiverType, bool hasLiteralReceiver)
{
    size_t functionFlags = tuuvm_function_getFlags(context, function);
    return
        hasLiteralReceiver ||
        (tuuvm_type_getFlags(receiverType) & (TUUVM_TYPE_FLAGS_FINAL)) != TUUVM_TYPE_FLAGS_NONE ||
        (functionFlags & (TUUVM_FUNCTION_FLAGS_MACRO | TUUVM_FUNCTION_FLAGS_FINAL)) != TUUVM_FUNCTION_FLAGS_NONE;
}

void tuuvm_function_registerPrimitives(void)
{
    tuuvm_primitiveTable_registerFunction(tuuvm_function_primitive_apply, "Function::apply");
    tuuvm_primitiveTable_registerFunction(tuuvm_function_primitive_adoptDefinitionOf, "Function::adoptDefinitionOf");
    tuuvm_primitiveTable_registerFunction(tuuvm_function_primitive_isCorePrimitive, "Function::isCorePrimitive");
    tuuvm_primitiveTable_registerFunction(tuuvm_function_primitive_recompileAndOptimize, "Function::recompileAndOptimize");
}

void tuuvm_function_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "apply", context->roots.functionType, "applyWithArguments:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_VARIADIC | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_function_primitive_apply);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Function::adoptDefinitionOf:", context->roots.functionType, "adoptDefinitionOf:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_function_primitive_adoptDefinitionOf);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Function::isCorePrimitive", context->roots.functionType, "isCorePrimitive", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_function_primitive_isCorePrimitive);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Function::recompileAndOptimize", context->roots.functionType, "recompileAndOptimize", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_function_primitive_recompileAndOptimize);

    // Export the function. This is used by the bootstraping algorithm for creating the accessors.
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "Function::Layout::flags", tuuvm_tuple_integer_encodeSmall(TUUVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(tuuvm_function_t, flags)));

    // Export the function flags.
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::None", tuuvm_tuple_bitflags_encode(TUUVM_FUNCTION_FLAGS_NONE));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::Macro", tuuvm_tuple_bitflags_encode(TUUVM_FUNCTION_FLAGS_MACRO));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::Variadic", tuuvm_tuple_bitflags_encode(TUUVM_FUNCTION_FLAGS_VARIADIC));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::CorePrimitive", tuuvm_tuple_bitflags_encode(TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::Pure", tuuvm_tuple_bitflags_encode(TUUVM_FUNCTION_FLAGS_PURE));

    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::Final", tuuvm_tuple_bitflags_encode(TUUVM_FUNCTION_FLAGS_FINAL));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::Virtual", tuuvm_tuple_bitflags_encode(TUUVM_FUNCTION_FLAGS_VIRTUAL));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::Abstract", tuuvm_tuple_bitflags_encode(TUUVM_FUNCTION_FLAGS_ABSTRACT));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::Override", tuuvm_tuple_bitflags_encode(TUUVM_FUNCTION_FLAGS_OVERRIDE));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::Static", tuuvm_tuple_bitflags_encode(TUUVM_FUNCTION_FLAGS_STATIC));

    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::Memoized", tuuvm_tuple_bitflags_encode(TUUVM_FUNCTION_FLAGS_MEMOIZED));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::Template", tuuvm_tuple_bitflags_encode(TUUVM_FUNCTION_FLAGS_TEMPLATE));

    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::NoTypecheckArguments", tuuvm_tuple_bitflags_encode(TUUVM_FUNCTION_FLAGS_NO_TYPECHECK_ARGUMENTS));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::AllowReferenceInReceiver", tuuvm_tuple_bitflags_encode(TUUVM_FUNCTION_FLAGS_ALLOW_REFERENCE_IN_RECEIVER));

    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::GetterFlags", tuuvm_tuple_bitflags_encode(TUUVM_FUNCTION_FLAGS_GETTER_FLAGS));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::SetterFlags", tuuvm_tuple_bitflags_encode(TUUVM_FUNCTION_FLAGS_SETTER_FLAGS));

    // Export the function application flags.
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionApplicationFlags::None", tuuvm_tuple_bitflags_encode(TUUVM_FUNCTION_APPLICATION_FLAGS_NONE));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionApplicationFlags::NoTypecheck", tuuvm_tuple_bitflags_encode(TUUVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionApplicationFlags::PassThroughReferences", tuuvm_tuple_bitflags_encode(TUUVM_FUNCTION_APPLICATION_FLAGS_PASS_THROUGH_REFERENCES));
}
