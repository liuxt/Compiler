//
//  manageRegister.h
//  Semantic
//
//  Created by liuxt on 15/12/24.
//  Copyright (c) 2015年 liuxt. All rights reserved.
//

#ifndef __MY_REGISTER_H__
#define __MY_REGISTER_H__
#include <stdio.h>
#include "pseudoVector.h"

#define INT_REGISTER_COUNT 6
#define INT_WORK_REGISTER_COUNT 2
#define FLOAT_REGISTER_COUNT 6
#define FLOAT_WORK_REGISTER_COUNT 2

extern char* addrRegisterName[];

extern char* addrWorkRegisterName[];

//extern char* zeroRegisterName;

extern char* intRegisterName[];

extern char* intWorkRegisterName[];

extern char* floatRegisterName[];

extern char* floatWorkRegisterName[];


typedef struct IntRegisterTable
{
    int isAllocated[INT_REGISTER_COUNT];
} IntRegisterTable;

typedef struct FloatRegisterTable
{
    int isAllocated[FLOAT_REGISTER_COUNT];
} FloatRegisterTable;

typedef struct PseudoRegisterTable
{
    PseudoVector* isAllocatedVector;
} PseudoRegisterTable;

typedef enum ProcessorType
{
    INT_REG,
    FLOAT_REG
} ProcessorType;

extern IntRegisterTable g_intRegisterTable;
extern FloatRegisterTable g_floatRegisterTable;
extern PseudoRegisterTable g_pseudoRegisterTable;
extern int g_pseudoRegisterBeginOffset;

void initializeRegisterTable();

void resetRegisterTable(int maxLocalVariableOffset);

int getRegister(ProcessorType processorType);
void freeRegister(ProcessorType processorType, int registerIndex);
void printStoreRegister(FILE* codeGenOutputFp);
void printRestoreRegister(FILE* codeGenOutputFp);
int getPseudoRegisterCorrespondingOffset(int pseudoRegisterIndex);

#endif
