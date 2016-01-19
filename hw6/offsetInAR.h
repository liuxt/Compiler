//
//  offsetInAR.h
//  Semantic
//
//  Created by liuxt on 15/12/24.
//  Copyright (c) 2015年 liuxt. All rights reserved.
//

#ifndef __OFFSET_IN_AR_H___
#define __OFFSET_IN_AR_H___

#include "symbolTable.h"

extern int g_offsetInARAux;
extern int g_deepestBlockVariableOffset;

void resetOffsetCalculation();
void setOffsetAndUpdateGlobalOffset(SymbolAttribute* attribute);

#endif
