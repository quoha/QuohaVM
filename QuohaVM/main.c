//
//  QuohaVM/main.c
//
//  Created by Michael Henderson on 4/29/14.
//  Copyright (c) 2014 Maloquacious Productions, LLC. All rights reserved.
//

#include <stdio.h>

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

struct VM {
    Cell  *programCounter;
    Cell  *dataStack;
    Cell  *returnStack;
    Cell  *endOfCore;
    Cell  *endOfStack;
    Cell   core[1];
}; // struct VM

Cell *CellFromCell(Cell *cell);
Cell *CellFromInteger(long integer);
Cell *CellFromNull();
Cell *CellFromNumber(double number);
Cell *CellFromOffset(size_t offset);
Cell *CellFromText(const char *text);


int main(int argc, const char * argv[])
{
    // insert code here...
    printf("Hello, World! %ld -> %ld %ld %ld\n", sizeof(Cell), sizeof(VM), sizeof(unsigned long), sizeof(unsigned long long));
    return 0;
}

