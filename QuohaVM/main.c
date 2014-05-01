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

struct Cell {
    union {
        size_t        coreOffset;
        long          integer;
        double        number;
        const char   *text;
    } data;
    enum { code, integer, null, number, text } kind;
    enum {
        opNOOP = 0,
        opHALT,
    } op;
}; // struct Cell

// core
//  [ 0x0000 --> 0x???? ] endOfCore [ returnStack --> <-- dataStack ] endOfStack
//
struct VM {
    Cell  *programCounter;
    Cell  *dataStack;
    Cell  *returnStack;
    Cell  *endOfCore;
    Cell  *endOfStack;
    Cell   core[3];
}; // struct VM

Cell *CellFromCell(Cell *cell);
Cell *CellFromInteger(long integer);
Cell *CellFromNull();
Cell *CellFromNumber(double number);
Cell *CellFromOffset(size_t offset);
Cell *CellFromText(const char *text);

VM *NewVM(int kiloCells, int kiloStack);

VM *NewVM(int kiloCells, int kiloStack) {
    VM *vm = malloc(sizeof(*vm) + kiloCells * 1024 + kiloStack * 1024);
    vm->programCounter = 0;
    vm->endOfCore      = vm->core      + kiloCells * 1024;
    vm->endOfStack     = vm->endOfCore + kiloStack * 1024;
    vm->returnStack    = vm->endOfCore;
    vm->dataStack      = vm->endOfStack;
    return vm;
}

int main(int argc, const char * argv[])
{
    // insert code here...
    printf("Hello, World! %ld -> %ld %ld %ld\n", sizeof(Cell), sizeof(VM), sizeof(unsigned long), sizeof(unsigned long long));
    return 0;
}

