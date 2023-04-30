#include "sysbvm/function.h"
#include "sysbvm/array.h"
#include "sysbvm/assert.h"
#include "sysbvm/bytecode.h"
#include "sysbvm/bytecodeCompiler.h"
#include "sysbvm/dictionary.h"
#include "sysbvm/errors.h"
#include "sysbvm/interpreter.h"
#include "sysbvm/stackFrame.h"
#include "sysbvm/string.h"
#include "sysbvm/type.h"
#include "internal/context.h"
#include <stdlib.h>
#include <stdio.h>

#define PRIMITIVE_TABLE_CAPACITY 1024

static bool sysbvm_primitiveTableIsComputed = false;
static uint32_t sysbvm_primitiveTableSize = 0;
static bool sysbvm_function_useBytecodeInterpreter = true;

typedef struct sysbvm_primitiveTableEntry_s
{
    sysbvm_functionEntryPoint_t entryPoint;
    const char *name;
} sysbvm_primitiveTableEntry_t;

static sysbvm_primitiveTableEntry_t sysbvm_primitiveTable[PRIMITIVE_TABLE_CAPACITY];

extern void sysbvm_context_registerPrimitives(void);
extern sysbvm_tuple_t sysbvm_interpreter_recompileAndOptimizeFunction(sysbvm_context_t *context, sysbvm_function_t **functionObject);

void sysbvm_primitiveTable_registerFunction(sysbvm_functionEntryPoint_t primitiveEntryPoint, const char *primitiveName)
{
    for(size_t i = 0; i < sysbvm_primitiveTableSize; ++i)
    {
        if(sysbvm_primitiveTable[i].entryPoint == primitiveEntryPoint)
            return;
    }

    SYSBVM_ASSERT(sysbvm_primitiveTableSize < PRIMITIVE_TABLE_CAPACITY);
    sysbvm_primitiveTable[sysbvm_primitiveTableSize].entryPoint = primitiveEntryPoint;
    sysbvm_primitiveTable[sysbvm_primitiveTableSize].name = primitiveName;
    ++sysbvm_primitiveTableSize;
}

static void sysbvm_primitiveTable_ensureIsComputed(void)
{
    if(sysbvm_primitiveTableIsComputed)
        return;

    sysbvm_primitiveTableIsComputed = true;
    sysbvm_context_registerPrimitives();
}

static bool sysbvm_primitiveTable_findEntryFor(sysbvm_functionEntryPoint_t primitiveEntryPoint, uint32_t *outEntryIndex)
{
    sysbvm_primitiveTable_ensureIsComputed();
    
    for(uint32_t i = 0; i < sysbvm_primitiveTableSize; ++i)
    {
        if(sysbvm_primitiveTable[i].entryPoint == primitiveEntryPoint)
        {
            *outEntryIndex = i;
            return true;
        }
    }
    return false;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionDefinition_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t flags, sysbvm_tuple_t argumentCount, sysbvm_tuple_t definitionEnvironment, sysbvm_tuple_t argumentNodes, sysbvm_tuple_t resultTypeNode, sysbvm_tuple_t body)
{
    sysbvm_functionDefinition_t *result = (sysbvm_functionDefinition_t*)sysbvm_context_allocatePointerTuple(context, context->roots.functionDefinitionType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_functionDefinition_t));
    result->sourcePosition = sourcePosition;
    result->flags = flags;
    result->argumentCount = argumentCount; 
    result->definitionEnvironment = definitionEnvironment;
    result->definitionArgumentNodes = argumentNodes;
    result->definitionResultTypeNode = resultTypeNode;
    result->definitionBodyNode = body;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_function_createPrimitive(sysbvm_context_t *context, size_t argumentCount, sysbvm_bitflags_t flags, void *userdata, sysbvm_functionEntryPoint_t entryPoint)
{
    sysbvm_function_t *result = (sysbvm_function_t*)sysbvm_context_allocatePointerTuple(context, context->roots.functionType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_function_t));
    result->argumentCount = sysbvm_tuple_size_encode(context, argumentCount);
    result->flags = sysbvm_tuple_bitflags_encode(flags);
    uint32_t primitiveEntryIndex = 0;
    if(!userdata && sysbvm_primitiveTable_findEntryFor(entryPoint, &primitiveEntryIndex))
    {
        result->primitiveTableIndex = sysbvm_tuple_uint32_encode(context, primitiveEntryIndex + 1);
        const char *primitiveName = sysbvm_primitiveTable[primitiveEntryIndex].name;
        if(primitiveName)
            result->primitiveName = sysbvm_symbol_internWithCString(context, primitiveName);
    }
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_function_createClosureWithCaptureVector(sysbvm_context_t *context, sysbvm_tuple_t functionDefinition, sysbvm_tuple_t captureVector)
{
    if(!sysbvm_tuple_isKindOf(context, functionDefinition, context->roots.functionDefinitionType))
        sysbvm_error("An actual function definition is required here.");

    sysbvm_functionDefinition_t *functionDefinitionObject = (sysbvm_functionDefinition_t*)functionDefinition;
    SYSBVM_ASSERT(functionDefinitionObject->analyzedType);

    sysbvm_function_t *result = (sysbvm_function_t*)sysbvm_context_allocatePointerTuple(context, functionDefinitionObject->analyzedType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_function_t));
    result->flags = functionDefinitionObject->flags;
    result->argumentCount = functionDefinitionObject->argumentCount; 
    result->captureVector = captureVector;
    result->definition = functionDefinition;
    result->primitiveName = functionDefinitionObject->analyzedPrimitiveName;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_function_createClosureWithCaptureEnvironment(sysbvm_context_t *context, sysbvm_tuple_t functionDefinition, sysbvm_tuple_t captureEnviroment)
{
    if(!sysbvm_tuple_isKindOf(context, functionDefinition, context->roots.functionDefinitionType))
        sysbvm_error("An actual function definition is required here.");

    sysbvm_functionDefinition_t *functionDefinitionObject = (sysbvm_functionDefinition_t*)functionDefinition;
    sysbvm_tuple_t functionType = functionDefinitionObject->analyzedType ? functionDefinitionObject->analyzedType : context->roots.functionType;

    sysbvm_function_t *result = (sysbvm_function_t*)sysbvm_context_allocatePointerTuple(context, functionType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_function_t));
    result->flags = functionDefinitionObject->flags;
    result->argumentCount = functionDefinitionObject->argumentCount; 
    result->captureEnvironment = captureEnviroment;
    result->definition = functionDefinition;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API size_t sysbvm_function_getArgumentCount(sysbvm_context_t *context, sysbvm_tuple_t function)
{
    if(sysbvm_tuple_isFunction(context, function))
    {
        sysbvm_function_t *functionObject = (sysbvm_function_t*)function;
        return sysbvm_tuple_size_decode(functionObject->argumentCount);
    }

    return 0;
}

SYSBVM_API void sysbvm_function_setFlags(sysbvm_context_t *context, sysbvm_tuple_t function, sysbvm_bitflags_t flags)
{
    if(!sysbvm_tuple_isFunction(context, function))
        sysbvm_error("Expected a function.");

    sysbvm_function_t *functionObject = (sysbvm_function_t*)function;
    functionObject->flags = sysbvm_tuple_bitflags_encode(flags);
}

SYSBVM_API void sysbvm_function_addFlags(sysbvm_context_t *context, sysbvm_tuple_t function, sysbvm_bitflags_t flags)
{
    if(!sysbvm_tuple_isFunction(context, function))
        sysbvm_error("Expected a function.");

    sysbvm_function_t *functionObject = (sysbvm_function_t*)function;
    functionObject->flags = sysbvm_tuple_bitflags_encode(sysbvm_tuple_bitflags_decode(functionObject->flags) | flags);
}

SYSBVM_API sysbvm_tuple_t sysbvm_ordinaryFunction_nativeApply(sysbvm_context_t *context, sysbvm_tuple_t function, sysbvm_functionEntryPoint_t nativeEntryPoint, size_t argumentCount, sysbvm_tuple_t *arguments, sysbvm_bitflags_t applicationFlags)
{
    (void)applicationFlags;
    sysbvm_stackFrameGCRootsRecord_t argumentsRecord = {
        .type = SYSBVM_STACK_FRAME_RECORD_TYPE_GC_ROOTS,
        .rootCount = argumentCount,
        .roots = arguments
    };
    sysbvm_stackFrame_pushRecord((sysbvm_stackFrameRecord_t*)&argumentsRecord);

    sysbvm_tuple_t result = nativeEntryPoint(context, function, argumentCount, arguments);
        
    sysbvm_stackFrame_popRecord((sysbvm_stackFrameRecord_t*)&argumentsRecord);
    return result;
}

SYSBVM_API sysbvm_functionEntryPoint_t sysbvm_function_getNumberedPrimitiveEntryPoint(sysbvm_context_t *context, uint32_t primitiveNumber)
{
    (void)context;
    if(primitiveNumber > 0 && primitiveNumber <= sysbvm_primitiveTableSize)
        return sysbvm_primitiveTable[primitiveNumber - 1].entryPoint;
    return NULL;
}

SYSBVM_API void sysbvm_function_recordBindingWithOwnerAndName(sysbvm_context_t *context, sysbvm_tuple_t function, sysbvm_tuple_t owner, sysbvm_tuple_t name)
{
    (void)context;
    if(!sysbvm_tuple_isFunction(context, function)) return;

    sysbvm_function_t *functionObject = (sysbvm_function_t*)function;
    sysbvm_programEntity_recordBindingWithOwnerAndName(context, function, owner, name);

    // Record the owner also in the definition.
    if(functionObject->definition)
        sysbvm_programEntity_recordBindingWithOwnerAndName(context, functionObject->definition, owner, name);
}

SYSBVM_API sysbvm_tuple_t sysbvm_ordinaryFunction_directApply(sysbvm_context_t *context, sysbvm_tuple_t function, size_t argumentCount, sysbvm_tuple_t *arguments, sysbvm_bitflags_t applicationFlags)
{
    sysbvm_function_t *functionObject = (sysbvm_function_t*)function;

    // Find the entry point in the primitive table.
    if(functionObject->primitiveTableIndex)
    {
        sysbvm_primitiveTable_ensureIsComputed();
        uint32_t primitiveNumber = sysbvm_tuple_uint32_decode(functionObject->primitiveTableIndex);
        if(primitiveNumber > 0 && primitiveNumber <= sysbvm_primitiveTableSize)
            return sysbvm_ordinaryFunction_nativeApply(context, function, sysbvm_primitiveTable[primitiveNumber - 1].entryPoint, argumentCount, arguments, applicationFlags);
    }
    
    if(!functionObject->definition)
        sysbvm_error("Cannot apply a function without a proper definition.");

    // Attempt to use the bytecode.
    sysbvm_functionDefinition_t *definition = (sysbvm_functionDefinition_t*)functionObject->definition;
    if(sysbvm_function_useBytecodeInterpreter && definition->bytecode)
        return sysbvm_bytecodeInterpreter_apply(context, function, argumentCount, arguments);
    
    return sysbvm_interpreter_applyClosureASTFunction(context, function, argumentCount, arguments, applicationFlags);
}

SYSBVM_API sysbvm_tuple_t sysbvm_ordinaryFunction_memoizedApply(sysbvm_context_t *context, sysbvm_tuple_t function_, size_t argumentCount, sysbvm_tuple_t *arguments, sysbvm_bitflags_t applicationFlags)
{
    struct {
        sysbvm_tuple_t function;
        sysbvm_tuple_t memoizationKey;
        sysbvm_tuple_t result;
    } gcFrame = {
        .function = function_
    };
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    sysbvm_stackFrameGCRootsRecord_t argumentsRecord = {
        .type = SYSBVM_STACK_FRAME_RECORD_TYPE_GC_ROOTS,
        .rootCount = argumentCount,
        .roots = arguments
    };
    sysbvm_stackFrame_pushRecord((sysbvm_stackFrameRecord_t*)&argumentsRecord);

    sysbvm_function_t **functionObject = (sysbvm_function_t**)&gcFrame.function;

    // Make the memoization lookup key.
    if(argumentCount > 1)
    {
        gcFrame.memoizationKey = sysbvm_array_create(context, argumentCount);
        for(size_t i = 0; i < argumentCount; ++i)
            sysbvm_array_atPut(gcFrame.memoizationKey, i , arguments[i]);
    }
    else if(argumentCount == 1)
    {
        gcFrame.memoizationKey = arguments[0];
    }

    // Find the result in the memoization table
    if((*functionObject)->memoizationTable)
    {
        if(sysbvm_weakValueDictionary_find(context, (*functionObject)->memoizationTable, gcFrame.memoizationKey, &gcFrame.result))
        {
            if(gcFrame.result == SYSBVM_PENDING_MEMOIZATION_VALUE)
                sysbvm_error("Computing cyclic memoized value.");

            sysbvm_stackFrame_popRecord((sysbvm_stackFrameRecord_t*)&argumentsRecord);
            SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return gcFrame.result;
        }
        else
        {
            sysbvm_weakValueDictionary_atPut(context, (*functionObject)->memoizationTable, gcFrame.memoizationKey, SYSBVM_PENDING_MEMOIZATION_VALUE);
        }
    }
    else
    {
        (*functionObject)->memoizationTable = sysbvm_weakValueDictionary_create(context);
    }

    // Apply the actual function.
    sysbvm_stackFrame_popRecord((sysbvm_stackFrameRecord_t*)&argumentsRecord);
    gcFrame.result = sysbvm_ordinaryFunction_directApply(context, gcFrame.function, argumentCount, arguments, applicationFlags);

    // Store the result
    sysbvm_weakValueDictionary_atPut(context, (*functionObject)->memoizationTable, gcFrame.memoizationKey, gcFrame.result);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_ordinaryFunction_apply(sysbvm_context_t *context, sysbvm_tuple_t function, size_t argumentCount, sysbvm_tuple_t *arguments, sysbvm_bitflags_t applicationFlags)
{
    // Make sure the closure with lazy analysis is done and reified.
    sysbvm_function_t* functionObject = (sysbvm_function_t*)function;
    if(functionObject->captureEnvironment)
    {
        if(functionObject->captureEnvironment == SYSBVM_PENDING_MEMOIZATION_VALUE)
            sysbvm_error("Applying function with cyclic pending lazy analysis process.");

        struct {
            sysbvm_tuple_t function;
        } gcFrame = {
            .function = function
        };
        SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

        sysbvm_stackFrameGCRootsRecord_t argumentsRecord = {
            .type = SYSBVM_STACK_FRAME_RECORD_TYPE_GC_ROOTS,
            .rootCount = argumentCount,
            .roots = arguments
        };
        sysbvm_stackFrame_pushRecord((sysbvm_stackFrameRecord_t*)&argumentsRecord);

        sysbvm_function_ensureAnalysis(context, (sysbvm_function_t**)&gcFrame.function);

        function = gcFrame.function;
        sysbvm_stackFrame_popRecord((sysbvm_stackFrameRecord_t*)&argumentsRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    }

    if(sysbvm_function_isMemoized(context, function))
        return sysbvm_ordinaryFunction_memoizedApply(context, function, argumentCount, arguments, applicationFlags);
    return sysbvm_ordinaryFunction_directApply(context, function, argumentCount, arguments, applicationFlags);
}

SYSBVM_API sysbvm_tuple_t sysbvm_function_apply(sysbvm_context_t *context, sysbvm_tuple_t function, size_t argumentCount, sysbvm_tuple_t *arguments, sysbvm_bitflags_t applicationFlags)
{
    // Is this an ordinary function?
    if(sysbvm_tuple_isFunction(context, function))
        return sysbvm_ordinaryFunction_apply(context, function, argumentCount, arguments, applicationFlags);

    if(!function)
        sysbvm_error("Cannot apply nil as a function.");

    // Send the #() and #(): messages to the functional object.
    if(argumentCount == 0)
    {
        return sysbvm_tuple_send0(context, context->roots.applyWithoutArgumentsSelector, function);
    }
    else
    {
        sysbvm_tuple_t argumentsArray = sysbvm_array_create(context, argumentCount);
        for(size_t i = 0; i < argumentCount; ++i)
            sysbvm_array_atPut(argumentsArray, i, arguments[i]);

        return sysbvm_tuple_send1(context, context->roots.applyWithArgumentsSelector, function, argumentsArray);
    }
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_send(sysbvm_context_t *context, sysbvm_tuple_t selector, size_t argumentCount, sysbvm_tuple_t *arguments, uint32_t applicationFlags)
{
    SYSBVM_ASSERT(argumentCount > 0); // We need a receiver for performing the lookup.
    struct {
        sysbvm_tuple_t method;
        sysbvm_tuple_t receiverType;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.receiverType = sysbvm_tuple_getType(context, arguments[0]);
    gcFrame.method = sysbvm_type_lookupSelector(context, gcFrame.receiverType, selector);
    if(!gcFrame.method)
    {
        sysbvm_tuple_t receiverTypeName = sysbvm_tuple_printString(context, gcFrame.receiverType);
        sysbvm_errorWithMessageTuple(sysbvm_string_concat(context, sysbvm_string_createWithCString(context, "Message notUnderstood by "), receiverTypeName));
    }
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    
    return sysbvm_function_apply(context, gcFrame.method, argumentCount, arguments, applicationFlags);
}

SYSBVM_API void sysbvm_functionCallFrameStack_begin(sysbvm_context_t *context, sysbvm_functionCallFrameStack_t *callFrameStack, sysbvm_tuple_t function, size_t argumentCount)
{
    callFrameStack->gcRoots.function = function;
    callFrameStack->isVariadic = sysbvm_function_isVariadic(context, callFrameStack->gcRoots.function);
    callFrameStack->expectedArgumentCount = sysbvm_function_getArgumentCount(context, callFrameStack->gcRoots.function);
    callFrameStack->argumentIndex = 0;
    callFrameStack->variadicArgumentIndex = 0;

    size_t requiredArgumentCount = callFrameStack->expectedArgumentCount;
    if(callFrameStack->isVariadic)
    {
        if(requiredArgumentCount == 0)
            sysbvm_error("Variadic functions require at least a single argument.");
        callFrameStack->variadicArgumentIndex = requiredArgumentCount - 1;

        --requiredArgumentCount;
        if(argumentCount < requiredArgumentCount)
            sysbvm_error("Missing required argument count.");

        size_t variadicArgumentCount = argumentCount - requiredArgumentCount;
        callFrameStack->gcRoots.applicationArguments[callFrameStack->variadicArgumentIndex] = sysbvm_array_create(context, variadicArgumentCount);
    }
    else
    {
        if(argumentCount != requiredArgumentCount)
            sysbvm_error("Function call does not receive the required number of arguments.");
    }

    if(argumentCount > SYSBVM_MAX_FUNCTION_ARGUMENTS && !callFrameStack->isVariadic)
        sysbvm_error("Function application direct arguments exceeds the max argument count.");
    
}

SYSBVM_API void sysbvm_functionCallFrameStack_push(sysbvm_functionCallFrameStack_t *callFrameStack, sysbvm_tuple_t argument)
{
    if(!callFrameStack->isVariadic || callFrameStack->argumentIndex < callFrameStack->variadicArgumentIndex)
    {
        callFrameStack->gcRoots.applicationArguments[callFrameStack->argumentIndex++] = argument;
        return;
    }

    sysbvm_array_atPut(callFrameStack->gcRoots.applicationArguments[callFrameStack->variadicArgumentIndex], callFrameStack->argumentIndex - callFrameStack->variadicArgumentIndex, argument);
    ++callFrameStack->argumentIndex;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionCallFrameStack_finish(sysbvm_context_t *context, sysbvm_functionCallFrameStack_t *callFrameStack, sysbvm_bitflags_t applicationFlags)
{
    return sysbvm_function_apply(context, callFrameStack->gcRoots.function, callFrameStack->expectedArgumentCount, callFrameStack->gcRoots.applicationArguments, applicationFlags);
}

static sysbvm_tuple_t sysbvm_function_primitive_apply(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *function = &arguments[0];
    sysbvm_tuple_t *argumentList = &arguments[1];

    size_t variadicArgumentCount = sysbvm_array_getSize(*argumentList);
    size_t argumentListSize = 0;
    size_t callArgumentCount = 0;
    if(variadicArgumentCount > 0)
    {
        argumentListSize = sysbvm_array_getSize(sysbvm_array_at(*argumentList, variadicArgumentCount - 1));
        callArgumentCount = variadicArgumentCount - 1 + argumentListSize;
    }

    sysbvm_functionCallFrameStack_t callFrameStack = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    sysbvm_functionCallFrameStack_begin(context, &callFrameStack, *function, callArgumentCount);
    if(variadicArgumentCount > 0)
    {
        for(size_t i = 0; i < variadicArgumentCount - 1; ++i)
            sysbvm_functionCallFrameStack_push(&callFrameStack, sysbvm_array_at(*argumentList, i));
        
        sysbvm_tuple_t argumentArray = sysbvm_array_at(*argumentList, variadicArgumentCount - 1);
        for(size_t i = 0; i < argumentListSize; ++i)
            sysbvm_functionCallFrameStack_push(&callFrameStack, sysbvm_array_at(argumentArray, i));
    }

    SYSBVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    return sysbvm_functionCallFrameStack_finish(context, &callFrameStack, 0);
}

static sysbvm_tuple_t sysbvm_function_primitive_isCorePrimitive(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_tuple_t *function = &arguments[0];
    if(!sysbvm_tuple_isFunction(context, *function))
        return SYSBVM_FALSE_TUPLE;
    
    sysbvm_function_t *functionObject = (sysbvm_function_t*)*function;
    size_t flags = sysbvm_tuple_bitflags_decode(functionObject->flags);
    return sysbvm_tuple_boolean_encode((flags & SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE) != 0);
}

static sysbvm_tuple_t sysbvm_function_primitive_adoptDefinitionOf(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *function = &arguments[0];
    sysbvm_tuple_t *definitionFunction = &arguments[1];
    if(!sysbvm_tuple_isFunction(context, *function)) sysbvm_error("Expected a function.");
    if(!sysbvm_tuple_isFunction(context, *definitionFunction)) sysbvm_error("Expected a function.");
    
    sysbvm_function_t **functionObject = (sysbvm_function_t**)function;
    sysbvm_function_t **definitionFunctionObject = (sysbvm_function_t**)definitionFunction;

    (*functionObject)->definition = (*definitionFunctionObject)->definition;
    (*functionObject)->captureVector = (*definitionFunctionObject)->captureVector;
    if((*definitionFunctionObject)->primitiveName)
        (*functionObject)->primitiveName = (*definitionFunctionObject)->primitiveName;
    sysbvm_tuple_setType((sysbvm_object_tuple_t*)*functionObject, sysbvm_tuple_getType(context, (sysbvm_tuple_t)*definitionFunctionObject));
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_function_primitive_recompileAndOptimize(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_tuple_t *function = &arguments[0];
    if(!sysbvm_tuple_isFunction(context, *function)) sysbvm_error("Expected a function.");
    
    sysbvm_function_t **functionObject = (sysbvm_function_t**)function;
    if((*functionObject)->definition && sysbvm_array_getSize((*functionObject)->captureVector) > 0)
        return sysbvm_interpreter_recompileAndOptimizeFunction(context, functionObject);
    
    return *function;
}

static sysbvm_tuple_t sysbvm_function_primitive_recordBindingWithOwnerAndName(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_function_recordBindingWithOwnerAndName(context, arguments[0], arguments[1], arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

bool sysbvm_function_shouldOptimizeLookup(sysbvm_context_t *context, sysbvm_tuple_t function, sysbvm_tuple_t receiverType, bool hasLiteralReceiver)
{
    size_t functionFlags = sysbvm_function_getFlags(context, function);
    return
        hasLiteralReceiver ||
        (sysbvm_type_getFlags(receiverType) & (SYSBVM_TYPE_FLAGS_FINAL)) != SYSBVM_TYPE_FLAGS_NONE ||
        (functionFlags & (SYSBVM_FUNCTION_FLAGS_MACRO | SYSBVM_FUNCTION_FLAGS_FINAL)) != SYSBVM_FUNCTION_FLAGS_NONE;
}

void sysbvm_function_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_function_primitive_apply, "Function::apply");
    sysbvm_primitiveTable_registerFunction(sysbvm_function_primitive_adoptDefinitionOf, "Function::adoptDefinitionOf");
    sysbvm_primitiveTable_registerFunction(sysbvm_function_primitive_isCorePrimitive, "Function::isCorePrimitive");
    sysbvm_primitiveTable_registerFunction(sysbvm_function_primitive_recompileAndOptimize, "Function::recompileAndOptimize");
    sysbvm_primitiveTable_registerFunction(sysbvm_function_primitive_recordBindingWithOwnerAndName, "Function::recordBindingWithOwnerAndName");
}

void sysbvm_function_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "apply", context->roots.functionType, "applyWithArguments:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_VARIADIC | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_function_primitive_apply);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Function::adoptDefinitionOf:", context->roots.functionType, "adoptDefinitionOf:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_function_primitive_adoptDefinitionOf);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Function::isCorePrimitive", context->roots.functionType, "isCorePrimitive", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_function_primitive_isCorePrimitive);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Function::recompileAndOptimize", context->roots.functionType, "recompileAndOptimize", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_function_primitive_recompileAndOptimize);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Function::recordBindingWithOwnerAndName", context->roots.functionType, "recordBindingWithOwner:andName:", 3, SYSBVM_FUNCTION_FLAGS_NONE, NULL, sysbvm_function_primitive_recordBindingWithOwnerAndName);

    // Export the function. This is used by the bootstraping algorithm for creating the accessors.
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "Function::Layout::flags", sysbvm_tuple_integer_encodeSmall(SYSBVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(sysbvm_function_t, flags)));

    // Export the function flags.
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::None", sysbvm_tuple_bitflags_encode(SYSBVM_FUNCTION_FLAGS_NONE));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::Macro", sysbvm_tuple_bitflags_encode(SYSBVM_FUNCTION_FLAGS_MACRO));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::Variadic", sysbvm_tuple_bitflags_encode(SYSBVM_FUNCTION_FLAGS_VARIADIC));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::CorePrimitive", sysbvm_tuple_bitflags_encode(SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::Pure", sysbvm_tuple_bitflags_encode(SYSBVM_FUNCTION_FLAGS_PURE));

    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::Final", sysbvm_tuple_bitflags_encode(SYSBVM_FUNCTION_FLAGS_FINAL));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::Virtual", sysbvm_tuple_bitflags_encode(SYSBVM_FUNCTION_FLAGS_VIRTUAL));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::Abstract", sysbvm_tuple_bitflags_encode(SYSBVM_FUNCTION_FLAGS_ABSTRACT));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::Override", sysbvm_tuple_bitflags_encode(SYSBVM_FUNCTION_FLAGS_OVERRIDE));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::Static", sysbvm_tuple_bitflags_encode(SYSBVM_FUNCTION_FLAGS_STATIC));

    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::Memoized", sysbvm_tuple_bitflags_encode(SYSBVM_FUNCTION_FLAGS_MEMOIZED));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::Template", sysbvm_tuple_bitflags_encode(SYSBVM_FUNCTION_FLAGS_TEMPLATE));

    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::NoTypecheckArguments", sysbvm_tuple_bitflags_encode(SYSBVM_FUNCTION_FLAGS_NO_TYPECHECK_ARGUMENTS));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::AllowReferenceInReceiver", sysbvm_tuple_bitflags_encode(SYSBVM_FUNCTION_FLAGS_ALLOW_REFERENCE_IN_RECEIVER));

    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::GetterFlags", sysbvm_tuple_bitflags_encode(SYSBVM_FUNCTION_FLAGS_GETTER_FLAGS));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionFlags::SetterFlags", sysbvm_tuple_bitflags_encode(SYSBVM_FUNCTION_FLAGS_SETTER_FLAGS));

    // Export the function application flags.
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionApplicationFlags::None", sysbvm_tuple_bitflags_encode(SYSBVM_FUNCTION_APPLICATION_FLAGS_NONE));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionApplicationFlags::NoTypecheck", sysbvm_tuple_bitflags_encode(SYSBVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionApplicationFlags::PassThroughReferences", sysbvm_tuple_bitflags_encode(SYSBVM_FUNCTION_APPLICATION_FLAGS_PASS_THROUGH_REFERENCES));
}
