//
//  QuohaVM/main.c
//
//  Created by Michael Henderson on 4/29/14.
//  Copyright (c) 2014 Maloquacious Productions, LLC. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Cell Cell;
typedef struct VM   VM;

enum DATATYPE { dtAddress, dtInteger, dtNull, dtNumber, dtOffset, dtText };

enum OPCODE {
    opDATA = 0,
    opGOSUB,
    opHALT,
    opJMP,
    opJNZ,
    opNOOP,
    opPANIC,
    opRTRN,
    opYIELD,
};

enum VMRESULT {
    vmrOKAY = 0,
    vmrFAIL,
    vmrDSINVALIDCELL,
    vmrDSOVERFLOW,
    vmrDSUNDERFLOW,
    vmrRSOVERFLOW,
    vmrRSUNDERFLOW,
    vmrPANIC,
};

struct Cell {
    union {
        Cell         *address;
        long          integer;
        double        number;
        size_t        offset;
        const char   *text;
        unsigned char bytes[sizeof(double)/sizeof(unsigned char)];
    } data;
    enum DATATYPE type;
    enum OPCODE   op;
}; // struct Cell

// core
//  [ 0x0000 --> 0x???? ] endOfCore [ startOfDataStack --> <-- startOfReturnStack ] endOfStack
//
struct VM {
    Cell  *instructionPointer;
    Cell  *programCounter;
    Cell  *dataStack;
    Cell  *returnStack;
    Cell  *startOfCore;
    Cell  *endOfCore;
    Cell  *startOfDataStack;
    Cell  *startOfReturnStack;
    size_t sizeOfCore;
    size_t sizeOfStack;
    unsigned long ticks;
    Cell   core[3];
}; // struct VM

Cell *cell_from_address(Cell *address);
Cell *cell_from_cell(Cell *cell);
Cell *cell_from_double(double number);
Cell *cell_from_integer(long integer);
Cell *cell_from_null();
Cell *cell_from_offset(size_t offset);
Cell *cell_from_text(const char *text);
const char *cell_type_as_text(enum DATATYPE type);

VM           *vm_new(int kiloCells, int kiloDataStack, int kiloReturnStack);
void          vm_dump_state(VM *vm);
enum VMRESULT vm_pop(VM *vm, Cell *save);
enum VMRESULT vm_run(VM *vm);
enum VMRESULT vm_set_program_counter(VM *vm, size_t address);

struct {
    // consider gperf for going the other way
    enum OPCODE op;
    const char *text;
} opAsText[] = {
    {opDATA  , "data" },
    {opGOSUB , "gosub"},
    {opHALT  , "halt" },
    {opJMP   , "jmp"  },
    {opJNZ   , "jnz"  },
    {opNOOP  , "noop" },
    {opPANIC , "panic"},
    {opRTRN  , "rtrn" },
    {opYIELD , "yield"},
};

const char *cell_type_as_text(enum DATATYPE type) {
    static char buffer[8];
    switch (type) {
        case dtAddress: return "address";
        case dtInteger: return "integer";
        case dtNull:    return "null"   ;
        case dtNumber:  return "number" ;
        case dtOffset:  return "offset" ;
        case dtText:    return "text"   ;
    }
    sprintf(buffer, "0x%04x", type);
    return buffer;
}

const char *opcode_as_text(enum OPCODE op) {
    static char buffer[8];
    if (opDATA <= op && op <= opYIELD) {
        if (opAsText[op].op != op) {
            printf("panic:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
            printf(". . .:\tenum OPCODE not in sync with opAsText\n");
            printf(". . .:\tinvalid op is %d\n", op);
            printf(". . .:\tarray   op is %d\n", opAsText[op].op);
            exit(2);
        }
        return opAsText[op].text;
    }
    printf(".warn:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    printf(". . .:\tenum OPCODE not in sync with opAsText\n");
    printf(". . .:\tinvalid op is %d\n", op);
    sprintf(buffer, "0x%04x", op);
    return buffer;
}

void vm_dump_state(VM *vm) {
    printf(". . .:\t%-18s == %8lu\n", "ticks", vm->ticks);
    printf(". . .:\t%-18s == %8ld\n", "ip", vm->instructionPointer - vm->core);
    //printf(". . .:\t%-18s == %8ld\n", "sizeOfCore", vm->sizeOfCore);
    //printf(". . .:\t%-18s == %8ld\n", "sizeOfStack", vm->sizeOfStack);
    printf(". . .:\t%-18s == %p\n", "startOfCore", vm->startOfCore);
    printf(". . .:\t%-18s == %p\n", "ip", vm->instructionPointer);
    printf(". . .:\t%-18s == %p\n", "startOfDataStack", vm->startOfDataStack);
    printf(". . .:\t%-18s == %p\n", "dataStack", vm->dataStack);
    printf(". . .:\t%-18s == %p\n", "returnStack", vm->returnStack);
    printf(". . .:\t%-18s == %p\n", "startOfReturnStack", vm->startOfReturnStack);
    printf(". . .:\t%-18s == %s\n", "opCode", opcode_as_text(vm->instructionPointer->op));
    printf(". . .:\t%-18s == %s\n", "opData", cell_type_as_text(vm->instructionPointer->type));
    switch (vm->instructionPointer->type) {
        case dtAddress:
            printf(". . .:\t%-18s == %p\n", "opValue", vm->instructionPointer->data.address);
            break;
        case dtInteger:
            printf(". . .:\t%-18s == %ld\n", "opValue", vm->instructionPointer->data.integer);
            break;
        case dtNull:
            printf(". . .:\t%-18s == %s\n", "opValue", "null");
            break;
        case dtNumber:
            printf(". . .:\t%-18s == %g\n", "opValue", vm->instructionPointer->data.number);
            break;
        case dtOffset:
            printf(". . .:\t%-18s == %ld\n", "opValue", vm->instructionPointer->data.offset);
            break;
        case dtText:
            printf(". . .:\t%-18s == %p\n", "opValue", vm->instructionPointer->data.text);
            break;
    }

}
VM *vm_new(int kiloCells, int kiloDataStack, int kiloReturnStack) {
    size_t sizeOfCore        = kiloCells       * 1024;
    size_t sizeOfStack       = kiloDataStack   * 1024;

    VM *vm = malloc(sizeof(*vm) + sizeOfCore + sizeOfStack);

    vm->ticks              = 0;

    vm->sizeOfCore         = sizeOfCore;
    vm->startOfCore        = vm->core;
    vm->endOfCore          = vm->startOfCore        + sizeOfCore;

    vm->sizeOfStack        = sizeOfStack;

    vm->startOfDataStack   = vm->endOfCore;
    vm->dataStack          = vm->startOfDataStack - 1;

    vm->startOfReturnStack = vm->startOfDataStack + sizeOfStack;
    vm->returnStack        = vm->startOfReturnStack + 1;

    vm->programCounter     = vm->core;
    vm->instructionPointer = vm->core;

    // initialize all cells for safety
    //
    for (Cell *cell = vm->core; cell < vm->endOfCore; cell++) {
        cell->op           = opPANIC;
        cell->type         = dtText;
        cell->data.text    = "a blast from the past!";
    }
    for (Cell *cell = vm->startOfDataStack; cell <= vm->startOfReturnStack; cell++) {
        cell->op           = opDATA;
        cell->type         = dtAddress;
        cell->data.address = 0;
    }

    return vm;
}

// always leave dataStack pointing at the top of the stack
//
enum VMRESULT vm_pop(VM *vm, Cell *save) {
    if (vm->dataStack < vm->startOfDataStack) {
        printf("panic:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        printf(". . .:\tstack underflow (data)\n");
        return vmrDSUNDERFLOW;
    } else if (save) {
        memcpy(save, vm->dataStack, sizeof(*save));
    }

    vm->dataStack--;

    return vmrOKAY;
}

// always leave dataStack pointing at the top of the stack
//
enum VMRESULT vm_push(VM *vm, Cell *cell) {
    // is there room for another cell on the stack?
    //
    vm->dataStack++;
    if (vm->dataStack >=  vm->returnStack) {
        vm->dataStack--;
        printf("panic:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        printf(". . .:\tstack overflow (data)\n");
        return vmrDSOVERFLOW;
    } else if (cell) {
        memcpy(vm->dataStack, cell, sizeof(*cell));
    } else {
        vm->dataStack->type = dtNull;
    }

    return vmrOKAY;
}

// always leave returnStack pointing to the top of the stack
//
enum VMRESULT vm_rspush(VM *vm, Cell *address) {
    vm->returnStack--;
    if (vm->returnStack <= vm->dataStack) {
        vm->returnStack++;
        printf("panic:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        printf(". . .:\tstack overflow (return stack)\n");
        vm_dump_state(vm);
        return vmrPANIC;
    }
    vm->returnStack->op           = opDATA;
    vm->returnStack->type         = dtAddress;
    vm->returnStack->data.address = address;
    return vmrOKAY;
}

enum VMRESULT vm_run(VM *vm) {
    for (;;) {
        vm->ticks++;
        if (vm->ticks > 5) {
            printf("panic:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
            printf(". . .:\ttimer expired\n");
            vm_dump_state(vm);
            return vmrPANIC;
        }

        vm->instructionPointer = vm->programCounter++;
        if (vm->instructionPointer >= vm->endOfCore) {
            printf("panic:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
            printf(". . .:\tinvalid instruction pointer (out of bounds)\n");
            vm_dump_state(vm);
            return vmrPANIC;
        }

        Cell *c[3];
        Cell save[3];
        switch (vm->instructionPointer->op) {
            case opDATA:
                printf("panic:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                printf(". . .:\tinvalid instruction pointer (points to data)\n");
                vm_dump_state(vm);
                return vmrPANIC;
            case opGOSUB:
                if (vm->instructionPointer->type != dtAddress) {
                    printf("panic:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                    printf(". . .:\tinvalid instruction (opType s/b address)\n");
                    vm_dump_state(vm);
                    return vmrPANIC;
                } else if (vm->instructionPointer->data.address < vm->startOfCore || vm->instructionPointer->data.address >= vm->endOfCore) {
                    printf("panic:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                    printf(". . .:\tinvalid instruction (jump out of bounds)\n");
                    vm_dump_state(vm);
                    return vmrPANIC;
                }
                if (!vm_rspush(vm, vm->programCounter)) {
                    vm_dump_state(vm);
                    return vmrPANIC;
                }
                // and jump to the address in the instruction
                vm->programCounter = vm->instructionPointer->data.address;
                continue;
            case opHALT:
                continue;
            case opJMP:
                if (vm->instructionPointer->type != dtOffset) {
                    printf("panic:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                    printf(". . .:\tinvalid instructon (opType s/b offset)\n");
                    vm_dump_state(vm);
                    return vmrPANIC;
                }
                vm->programCounter = vm->core + vm->instructionPointer->data.offset;
                continue;
            case opJNZ:
                c[0] = vm_pop(vm, save + 0);
                if (!c[0]) {
                    printf("panic:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                    printf(". . .:\tstack underflow (data)\n");
                    vm_dump_state(vm);
                    return vmrPANIC;
                } else if (vm->instructionPointer->type != dtAddress) {
                    printf("panic:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                    printf(". . .:\tinvalid instructon (opType s/b offset)\n");
                    vm_dump_state(vm);
                    return vmrPANIC;
                }
                switch (save->type) {
                    case dtAddress:
                        if (save->data.address) {
                            vm->programCounter = vm->core + vm->instructionPointer->data.offset;
                        }
                        break;
                    case dtInteger:
                        if (save->data.integer) {
                            vm->programCounter = vm->core + vm->instructionPointer->data.offset;
                        }
                        break;
                    case dtNull:
                        break;
                    case dtNumber:
                        if (save->data.number) {
                            vm->programCounter = vm->core + vm->instructionPointer->data.offset;
                        }
                        break;
                    case dtOffset:
                        if (save->data.offset) {
                            vm->programCounter = vm->core + vm->instructionPointer->data.offset;
                        }
                        break;
                    case dtText:
                        if (save->data.text) {
                            vm->programCounter = vm->core + vm->instructionPointer->data.offset;
                        }
                        break;
                }
                continue;
            case opNOOP:
                continue;
            case opPANIC:
                printf("panic:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                if (vm->instructionPointer->type == dtText) {
                    if (vm->instructionPointer->data.text) {
                        printf(". . .:\t%-18s == %s\n", "userMessage", vm->instructionPointer->data.text);
                    }
                }
                vm_dump_state(vm);
                return vmrPANIC;
            case opRTRN:
                if (vm->returnStack < vm->startOfReturnStack) {
                    printf("panic:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                    vm_dump_state(vm);
                    return vmrPANIC;
                }
                vm->returnStack->op = opDATA;
                vm->returnStack->type = dtAddress;
                vm->returnStack->data.address = vm->programCounter;
                continue;
                continue;
            case opYIELD:
                continue;
        }

        // if we fall through to here, the op code wasn't coded for. that
        // implies we failed to properly add a new op code or the cell
        // is invalid. either way, bail.
        //
        printf("panic:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        printf(". . .:\tinvalid instruction pointer (unhandled opcode)\n");
        vm_dump_state(vm);
        return vmrPANIC;
    }
    // NOT REACHED
    return vmrOKAY;
}

enum VMRESULT vm_set_program_counter(VM *vm, size_t offset) {
    if (offset >= vm->sizeOfCore) {
        printf("panic:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        printf(". . .:\tinvalid program counter setting\n");
        printf(". . .:\t%-18s == %8ld\n", "address"   , offset);
        printf(". . .:\t%-18s == %8ld\n", "sizeOfCore", vm->sizeOfCore);
        return vmrPANIC;
    }
    vm->programCounter = vm->core + offset;
    return vmrOKAY;
}

int main(int argc, const char * argv[])
{
    Cell c;
    // insert code here...
    printf("Hello, World! %ld -> %ld %ld %ld %ld\n", sizeof(Cell), sizeof(VM), sizeof(c.data.bytes), sizeof(double), sizeof(int));

    VM *vm = vm_new(1, 1, 1);
    vm_run(vm);

    return 0;
}
