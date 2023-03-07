GCROOT_TYPES = [
    'tuuvm_tuple_t', 'tuuvm_stuple_t'
]
FUNCTION_BLACKLIST = [
    #'tuuvm_interpreter_applyClosureASTFunction',
]

class StackRootVariable:
    def __init__(self, type, name, address, function, line):
        self.type = type
        self.name = name
        self.address = address
        self.function = function
        self.line = line

    def __str__(self) -> str:
        return '0x%08x: %s %s %s:%s' % (self.address, self.type, self.name, self.function, self.line)

def dumpRecordedStackRoots():
    gdb.execute('call tuuvm_stackFrame_dumpStackGCRootsToFileNamed("stackRoots.dmp")')
    with open('stackRoots.dmp') as f:
        return list(map(lambda line: int(line, 0), f.read().splitlines()))

def findGCRootsInStack():
    gcroots = []
    currentFrame = gdb.newest_frame()
    while currentFrame is not None:
        currentFrameBlock = currentFrame.block()
        functionName = str(currentFrame.function())
        if functionName not in FUNCTION_BLACKLIST:
            for local in currentFrameBlock:
                if local.is_argument:
                    continue

                type, name, address, line = local.type, local.name, int(str(local.value(currentFrame).address), 0), local.line
                if str(type) in GCROOT_TYPES:
                    gcroots.append(StackRootVariable(type, name, address, functionName, line))

        currentFrame = currentFrame.older()
        
    return gcroots

class ValidateGCStackRoots(gdb.Command):
    def __init__(self):
        super(ValidateGCStackRoots, self).__init__("validategcstackroots", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        dumpedStackRoots = dumpRecordedStackRoots()
        dumpedStackRootSet = set(dumpedStackRoots)
        declaredGCRoots = findGCRootsInStack()

        for root in declaredGCRoots:
            if root.address not in dumpedStackRootSet:
                print(str(root))

ValidateGCStackRoots()
gdb.Breakpoint('tuuvm_gc_debugStackValidationHook').commands = """
validategcstackroots
continue
"""
