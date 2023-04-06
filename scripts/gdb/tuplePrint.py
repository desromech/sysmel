ImmediateTypeName = [
    'nil',
    'integer',
    'int8', 'int16', 'int32', 'int64',
    'char8', 'uint8',
    'char16', 'uint16',
    'char32', 'uint32',
    'uint64',
    'float32', 'float64',
    'trivial'
]
ImmediateValueTable = ["false", "true", "void", "HashtableEmptyElement"]

class TuplePrinter(object):
    "Prints sysbvm_tuple_t"

    def __init__(self, val):
        self.val = val

    def to_string(self):
        pointerValue = int(self.val)
        if pointerValue == 0:
            return 'nil'
        pointerTag = pointerValue & 15
        if pointerTag != 0:
            untaggedValue = pointerValue >> 4
            ## Immediate
            if pointerTag == 15:
                if untaggedValue < len(ImmediateValueTable):
                    return ImmediateValueTable[untaggedValue]
            
            ## Integers
            return '%s %d' % (ImmediateTypeName[pointerTag], untaggedValue)

        return '0x%016x' % pointerValue

def tupleLookupFunction(val):
    if str(val.type) in ['sysbvm_tuple_t', 'sysbvm_stuple_t']:
        return TuplePrinter(val)
    return None

gdb.pretty_printers.append(tupleLookupFunction)