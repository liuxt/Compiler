//
//  manageRegister.c
//  Semantic
//
//  Created by liuxt on 15/12/25.
//  Copyright (c) 2015年 liuxt. All rights reserved.
//

#include "manageRegister.h"
#include <stdlib.h>
#include <string.h>
#include "pseudoVector.h"


//char* zeroRegisterName = "$0";

char* addrRegisterName[] = {
    "x9", "x10", "x11", "x12", "x13", "x8"
};
// 1 for pseudo, 0 for other
char* addrWorkRegisterName[] = {
    "x14", "x15"
};
char* intRegisterName[] = {
    "w9", "w10", "w11", "w12", "w13", "w8"
};

char* intWorkRegisterName[] = {"w14", "w15"};

char* floatRegisterName[] = {
    "s16", "s17", "s18", "s19", "s20", "s21"
};

char* floatWorkRegisterName[] = {"s22", "s23"};

// bind with addrRegisterTable
IntRegisterTable g_intRegisterTable;
FloatRegisterTable g_floatRegisterTable;
PseudoRegisterTable g_pseudoRegisterTable;
int g_pseudoRegisterBeginOffset = -8; // change from 4 to 8

void initializeRegisterTable()
{
    g_pseudoRegisterTable.isAllocatedVector = getInitialPseudoVector(10);
}


void resetRegisterTable(int maxLocalVariableOffset)
{
    memset(g_intRegisterTable.isAllocated, 0, sizeof(g_intRegisterTable.isAllocated));
    memset(g_floatRegisterTable.isAllocated, 0, sizeof(g_floatRegisterTable.isAllocated));
    memset(g_pseudoRegisterTable.isAllocatedVector->data, 0, sizeof(int) * g_pseudoRegisterTable.isAllocatedVector->capacity);
    g_pseudoRegisterTable.isAllocatedVector->size = 0;
    g_pseudoRegisterBeginOffset = maxLocalVariableOffset - 8;//  change from 4 to 8
}


int getRegister(ProcessorType processorType)
{
    int realTableIndex = 0;
    int realRegisterCount = (processorType == INT_REG) ? INT_REGISTER_COUNT : FLOAT_REGISTER_COUNT;
    int* realRegIsAllocated = (processorType == INT_REG) ? g_intRegisterTable.isAllocated : g_floatRegisterTable.isAllocated;
    for(realTableIndex= 0; realTableIndex < realRegisterCount; ++realTableIndex)
    {
        if(!realRegIsAllocated[realTableIndex])
        {
            realRegIsAllocated[realTableIndex] = 1;
            return realTableIndex;
        }
    }
    
    int pseudoTableIndex = 0;
    for(pseudoTableIndex = 0; pseudoTableIndex < g_pseudoRegisterTable.isAllocatedVector->size; ++pseudoTableIndex)
    {
        if(!g_pseudoRegisterTable.isAllocatedVector->data[pseudoTableIndex])
        {
            g_pseudoRegisterTable.isAllocatedVector->data[pseudoTableIndex] = 1;
            return (processorType == INT_REG) ? (INT_REGISTER_COUNT + pseudoTableIndex) : (FLOAT_REGISTER_COUNT + pseudoTableIndex);
        }
    }
    
    pushIntoPseudoVector(g_pseudoRegisterTable.isAllocatedVector, 1);
    
    return (processorType == INT_REG) ? (INT_REGISTER_COUNT + pseudoTableIndex) : (FLOAT_REGISTER_COUNT + pseudoTableIndex);
}

void freeRegister(ProcessorType processorType, int registerIndex)
{
    int realRegisterCount = (processorType == INT_REG) ? INT_REGISTER_COUNT : FLOAT_REGISTER_COUNT;
    int* realRegIsAllocated = (processorType == INT_REG) ? g_intRegisterTable.isAllocated : g_floatRegisterTable.isAllocated;
    
    if(registerIndex < realRegisterCount)
    {
        //free real register
        realRegIsAllocated[registerIndex] = 0;
    }
    else
    {
        //free pseudo register
        int pseudoTableIndex = registerIndex - realRegisterCount;
        g_pseudoRegisterTable.isAllocatedVector->data[pseudoTableIndex] = 0;
    }
}


void printStoreRegister(FILE* codeGenOutputFp)
{
    int index = 0;
    int tmpOffset = 8;
    for(index = 0; index < INT_REGISTER_COUNT; ++index)
    {
        fprintf(codeGenOutputFp, "str %s, [sp, #%d]\n", addrRegisterName[index], tmpOffset);
        tmpOffset += 8;
    }
    for(index = 0; index < INT_WORK_REGISTER_COUNT; ++index)
    {
        fprintf(codeGenOutputFp, "str %s, [sp, #%d]\n", addrWorkRegisterName[index], tmpOffset);
        tmpOffset += 8;
    }
    
    for(index = 0; index < FLOAT_REGISTER_COUNT; ++index)
    {
        fprintf(codeGenOutputFp, "str %s, [sp, #%d]\n", floatRegisterName[index], tmpOffset);
        tmpOffset += 4;
    }
    for(index = 0; index < FLOAT_WORK_REGISTER_COUNT; ++index)
    {
        fprintf(codeGenOutputFp, "str %s, [sp, #%d]\n", floatWorkRegisterName[index], tmpOffset);
        tmpOffset += 4;
    }
}


void printRestoreRegister(FILE* codeGenOutputFp)
{
    int index = 0;
    int tmpOffset = 8;
    for(index = 0; index < INT_REGISTER_COUNT; ++index)
    {
        fprintf(codeGenOutputFp, "ldr %s, [sp, #%d]\n", addrRegisterName[index], tmpOffset);
        tmpOffset += 8;
    }
    for(index = 0; index < INT_WORK_REGISTER_COUNT; ++index)
    {
        fprintf(codeGenOutputFp, "ldr %s, [sp, #%d]\n", addrWorkRegisterName[index], tmpOffset);
        tmpOffset += 8;
    }
    
    for(index = 0; index < FLOAT_REGISTER_COUNT; ++index)
    {
        fprintf(codeGenOutputFp, "ldr %s, [sp, #%d]\n", floatRegisterName[index], tmpOffset);
        tmpOffset += 4;
    }
    for(index = 0; index < FLOAT_WORK_REGISTER_COUNT; ++index)
    {
        fprintf(codeGenOutputFp, "ldr %s, [sp, #%d]\n", floatWorkRegisterName[index], tmpOffset);
        tmpOffset += 4;
    }
}

int getPseudoRegisterCorrespondingOffset(int pseudoRegisterIndex)
{
    return g_pseudoRegisterBeginOffset - pseudoRegisterIndex * 8;
}

