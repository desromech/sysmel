# Tuple Micro Virtual Machine
### A minimalistic embeddable virtual machine based around tuple

## Object Model
The object model is centered around having two different kinds of tuples: GC pointer tuples, bytes tuples. They have the following object layout:

- Type Pointer. Offset at word size * -2. 16 byte aligned pointer. 4 Low order bits are used for object metadata.
    Bit 0: bytes object.
    Bit 1: immutable object.
    Bit 2-3: reserved.
- Size in bytes. Offset at word size * -1. High bit set means that this is a free block.
- Data. Offset 0.
