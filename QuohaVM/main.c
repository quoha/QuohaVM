//
//  QuohaVM/main.c
//
//  Created by Michael Henderson on 4/29/14.
//  Copyright (c) 2014 Maloquacious Productions, LLC. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

typedef struct Cell Cell;
typedef struct VM   VM;

enum CELLTYPE { ctAddress, ctInteger, ctNull, ctNumber, ctOpCode, ctText };

enum OPCODE {
    opNOOP = 0,
    opHALT,
    opPANIC,
    opYIELD,
};

enum VMRESULT {
    vmrOKAY = 0,
    vmrFAIL,
    vmrPANIC,
};

struct Cell {
    union {
        size_t        address;
        long          integer;
        double        number;
        const char   *text;
        unsigned char bytes[sizeof(double)/sizeof(unsigned char)];
    } data;
    enum CELLTYPE type;
    enum OPCODE   op;
}; // struct Cell

// core
//  [ 0x0000 --> 0x???? ] endOfCore [ returnStack --> <-- dataStack ] endOfStack
//
struct VM {
    Cell  *programCounter;
    Cell  *dataStack;
    Cell  *returnStack;
    Cell  *endOfCore;
    Cell  *endOfDataStack;
    Cell  *endOfReturnStack;
    size_t sizeOfCore;
    size_t sizeOfDataStack;
    size_t sizeOfReturnStack;
    Cell   core[3];
}; // struct VM

Cell *cell_from_address(size_t address);
Cell *cell_from_cell(Cell *cell);
Cell *cell_from_double(double number);
Cell *cell_from_integer(long integer);
Cell *cell_from_null();
Cell *cell_from_text(const char *text);
const char *cell_type_as_text(enum CELLTYPE type);


VM           *vm_new(int kiloCells, int kiloDataStack, int kiloReturnStack);
enum VMRESULT vm_run(VM *vm);
enum VMRESULT vm_set_program_counter(VM *vm, size_t address);

const char *cell_type_as_text(enum CELLTYPE type) {
    static char buffer[8];
    switch (type) {
        case ctAddress: return "address";
        case ctInteger: return "integer";
        case ctNull:    return "null"   ;
        case ctNumber:  return "number" ;
        case ctOpCode:  return "opcode" ;
        case ctText:    return "text"   ;
    }
    sprintf(buffer, "0x%04x", type);
    return buffer;
}

const char *opcode_as_text(enum OPCODE op) {
    static char buffer[8];
    switch (op) {
        case opHALT:  return "halt";
        case opNOOP:  return "noop";
        case opPANIC: return "panic";
        case opYIELD: return "yield";
    }
    sprintf(buffer, "0x%04x", op);
    return buffer;
}

VM *vm_new(int kiloCells, int kiloDataStack, int kiloReturnStack) {
    size_t sizeOfCore        = kiloCells       * 1024;
    size_t sizeOfDataStack   = kiloDataStack   * 1024;
    size_t sizeOfReturnStack = kiloReturnStack * 1024;
    VM *vm = malloc(sizeof(*vm) + sizeOfCore + sizeOfDataStack + sizeOfReturnStack);
    vm->programCounter   = vm->core;
    vm->endOfCore        = vm->core           + sizeOfCore;
    vm->dataStack        = vm->endOfCore;
    vm->endOfDataStack   = vm->dataStack      + sizeOfDataStack;
    vm->returnStack      = vm->endOfDataStack;
    vm->endOfReturnStack = vm->returnStack    + sizeOfReturnStack;

    // for safety
    //
    for (Cell *cell = vm->core; cell < vm->endOfReturnStack; cell++) {
        cell->type = ctNull;
        cell->type = ctOpCode;
        cell->op   = opYIELD;
        cell->data.text = 0;
        cell->op   = opPANIC;
        cell->data.text = "a blast from the past!";
    }
    return vm;
}

enum VMRESULT vm_run(VM *vm) {
    for (;;) {
        Cell *instructionPointer = vm->programCounter++;
        if (instructionPointer >= vm->endOfCore) {
            printf("panic:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
            printf(". . .:\tinvalid instruction pointer (out of bounds)\n");
            printf(". . .:\t%-18s == %8ld\n", "address"   , instructionPointer - vm->core);
            printf(". . .:\t%-18s == %8ld\n", "sizeOfCore", vm->sizeOfCore);
            exit(2);
        }
        switch (instructionPointer->type) {
            case ctOpCode:
                break;
            case ctAddress:
            case ctInteger:
            case ctNull:
            case ctNumber:
            case ctText:
                printf("panic:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                printf(". . .:\tinvalid instruction pointer (invalid contents)\n");
                printf(". . .:\t%-18s == %8ld\n", "address", instructionPointer - vm->core);
                printf(". . .:\t%-18s == %s\n", "cellType", cell_type_as_text(instructionPointer->type));
                exit(2);
                break;
        }
        switch (instructionPointer->op) {
            case opHALT:
                continue;
            case opNOOP:
                continue;
            case opPANIC:
                printf("panic:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                printf(". . .:\t%-18s == %8ld\n", "address", instructionPointer - vm->core);
                printf(". . .:\t%-18s == %s\n", "opCode", opcode_as_text(instructionPointer->op));
                if (instructionPointer->data.text) {
                    printf(". . .:\t%-18s == %s\n", "userMessage", instructionPointer->data.text);
                }
                return vmrPANIC;
            case opYIELD:
                continue;
        }
        printf("panic:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        printf(". . .:\tinvalid instruction pointer (invalid opcode)\n");
        printf(". . .:\t%-18s == %8ld\n", "address", instructionPointer - vm->core);
        printf(". . .:\t%-18s == %s\n", "opCode", opcode_as_text(instructionPointer->op));
        return vmrPANIC;
    }
    // NOT REACHED
    return vmrOKAY;
}

enum VMRESULT vm_set_program_counter(VM *vm, size_t address) {
    if (address >= vm->sizeOfCore) {
        printf("panic:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        printf(". . .:\tinvalid program counter setting\n");
        printf(". . .:\t%-18s == %8ld\n", "address"   , address);
        printf(". . .:\t%-18s == %8ld\n", "sizeOfCore", vm->sizeOfCore);
        exit(2);
    }
    return vmrOKAY;
}

int main(int argc, const char * argv[])
{
    Cell c;
    // insert code here...
    printf("Hello, World! %ld -> %ld %ld\n", sizeof(Cell), sizeof(VM), sizeof(c.data.bytes));

    VM *vm = vm_new(1, 1, 1);
    vm_run(vm);

    return 0;
}

