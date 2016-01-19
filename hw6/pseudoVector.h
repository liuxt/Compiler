//
//  pseudoVector.h
//  Semantic
//
//  Created by liuxt on 15/12/25.
//  Copyright (c) 2015年 liuxt. All rights reserved.
//

#ifndef __pseudoVector_H__
#define __pseudoVector_H__
typedef struct PseudoVector{
    int* data;
    int capacity;
    int size;
} PseudoVector;

PseudoVector* getInitialPseudoVector(int capacity);
void pushIntoPseudoVector(PseudoVector* pseudovector, int val);
int popFromPseudoVector(PseudoVector* pseudovector);



#endif
