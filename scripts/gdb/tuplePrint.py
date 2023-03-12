ImmediateValueTable = ["false", "true", "void", "HashtableEmptyElement"]

class TuplePrinter(object):
    "Prints tuuvm_tuple_t"

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
                return 'Immediate %d' % untaggedValue
            
            ## Integers
            return str(untaggedValue)

        return '0x%016x' % pointerValue

def tupleLookupFunction(val):
    if str(val.type) in ['tuuvm_tuple_t', 'tuuvm_stuple_t']:
        return TuplePrinter(val)
    return None

gdb.pretty_printers.append(tupleLookupFunction)