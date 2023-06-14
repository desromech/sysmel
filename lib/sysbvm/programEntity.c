#include "sysbvm/programEntity.h"
#include "sysbvm/errors.h"
#include "sysbvm/function.h"
#include "internal/context.h"

SYSBVM_API void sysbvm_programEntity_recordBindingWithOwnerAndName(sysbvm_context_t *context, sysbvm_tuple_t programEntity, sysbvm_tuple_t owner, sysbvm_tuple_t name)
{
    (void)context;
    if(!sysbvm_tuple_isKindOf(context, programEntity, context->roots.programEntityType)) return;
    if(owner == programEntity) return;

    sysbvm_programEntity_t *programEntityObject = (sysbvm_programEntity_t*)programEntity;
    if(!programEntityObject->owner && !programEntityObject->name)
    {
        programEntityObject->owner = owner;
        programEntityObject->name = name;
    }
}

static sysbvm_tuple_t sysbvm_programEntity_primitive_recordBindingWithOwnerAndName(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_programEntity_recordBindingWithOwnerAndName(context, arguments[0], arguments[1], arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

void sysbvm_programEntity_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_programEntity_primitive_recordBindingWithOwnerAndName, "ProgramEntity::recordBindingWithOwner:andName:");
}

void sysbvm_programEntity_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "ProgramEntity::recordBindingWithOwner:andName:", context->roots.programEntityType, "recordBindingWithOwner:andName:", 3, SYSBVM_FUNCTION_FLAGS_VIRTUAL, NULL, sysbvm_programEntity_primitive_recordBindingWithOwnerAndName);
}
