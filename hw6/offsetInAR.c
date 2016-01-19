//
//  offsetInAR.c
//  Semantic
//
//  Created by liuxt on 15/12/25.
//  Copyright (c) 2015年 liuxt. All rights reserved.
//
#include <stdio.h>
#include "symbolTable.h"
#include "offsetInAR.h"

int g_offsetInARAux = 0;
int g_deepestBlockVariableOffset = 0;


void resetOffsetCalculation(){
    g_offsetInARAux = 0;
    g_deepestBlockVariableOffset = 0;
}

void setOffsetAndUpdateGlobalOffset(SymbolAttribute* attribute){
    int variableSize =getVariableSize(attribute->attr.typeDescriptor);
    g_offsetInARAux = g_offsetInARAux - variableSize;
    attribute->offsetInAR = g_offsetInARAux;
}
