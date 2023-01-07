#include "tuuvm/sourceCode.h"
#include "tuuvm/string.h"
#include "tuuvm/arrayList.h"
#include "tuuvm/arraySlice.h"
#include "internal/context.h"

TUUVM_API tuuvm_tuple_t tuuvm_sourceCode_create(tuuvm_context_t *context, tuuvm_tuple_t text, tuuvm_tuple_t name)
{
    tuuvm_sourceCode_t *result = (tuuvm_sourceCode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.sourceCodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_sourceCode_t));
    result->text = text;
    result->name = name;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_sourceCode_createWithCStrings(tuuvm_context_t *context, const char *text, const char *name)
{
    return tuuvm_sourceCode_create(context, tuuvm_string_createWithCString(context, text), tuuvm_string_createWithCString(context, name));
}

static tuuvm_tuple_t tuuvm_sourceCode_ensureLineStartIndexTableIsBuilt(tuuvm_context_t *context, tuuvm_tuple_t sourceCode)
{
    tuuvm_sourceCode_t *sourceCodeObject = (tuuvm_sourceCode_t*)sourceCode;
    if(sourceCodeObject->lineStartIndexTable)
        return sourceCodeObject->lineStartIndexTable;

    tuuvm_tuple_t arrayList = tuuvm_arrayList_create(context);
    size_t sourceCodeTextSize = tuuvm_tuple_getSizeInBytes(sourceCodeObject->text);
    uint8_t *sourceCodeData = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(sourceCodeObject->text)->bytes;

    tuuvm_arrayList_add(context, arrayList, tuuvm_tuple_size_encode(context, 0));
    for(size_t i = 0; i < sourceCodeTextSize; ++i)
    {
        if(sourceCodeData[i] == '\n')
            tuuvm_arrayList_add(context, arrayList, tuuvm_tuple_size_encode(context, i + 1));
    }

    sourceCodeObject->lineStartIndexTable = tuuvm_arrayList_asArraySlice(context, arrayList);
    return sourceCodeObject->lineStartIndexTable;
}

TUUVM_API void tuuvm_sourceCode_computeLineAndColumnForIndex(tuuvm_context_t *context, tuuvm_tuple_t sourceCode, tuuvm_tuple_t index, tuuvm_tuple_t *outLine, tuuvm_tuple_t *outColumn)
{
    // Make sure the line start index table is built.
    tuuvm_tuple_t indexTable = tuuvm_sourceCode_ensureLineStartIndexTableIsBuilt(context, sourceCode);

    // Perform a binary search.
    size_t indexTableSize = tuuvm_arraySlice_getSize(indexTable);
    size_t indexValue = tuuvm_tuple_size_decode(index);

    size_t left = 0;
    size_t right = indexTableSize;
    size_t bestSoFar = 0;
    size_t bestIndexSoFar = 0;
    while(left < right)
    {
        size_t middle = left + (right - left) / 2;
        size_t middleIndex = tuuvm_tuple_size_decode(tuuvm_arraySlice_at(indexTable, middle));
        if(middleIndex <= indexValue)
        {
            bestSoFar = middle;
            bestIndexSoFar = middleIndex;
            left = middle + 1;
        }
        else if(middleIndex > indexValue)
        {
            right = middle;
        }
    }

    size_t line = bestSoFar + 1;
    size_t column = indexValue - bestIndexSoFar + 1;

    // Emit the result.
    *outLine = tuuvm_tuple_size_encode(context, line);
    *outColumn = tuuvm_tuple_size_encode(context, column);
}

TUUVM_API tuuvm_tuple_t tuuvm_sourceCode_getText(tuuvm_tuple_t sourceCode)
{
    return ((tuuvm_sourceCode_t*)sourceCode)->text;
}

TUUVM_API tuuvm_tuple_t tuuvm_sourceCode_getName(tuuvm_tuple_t sourceCode)
{
    return ((tuuvm_sourceCode_t*)sourceCode)->name;
}
