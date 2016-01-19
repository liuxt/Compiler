//
//  pseudoVector.c
//  Semantic
//
//  Created by liuxt on 15/12/25.
//  Copyright (c) 2015年 liuxt. All rights reserved.
//

#include "pseudoVector.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

PseudoVector* getInitialPseudoVector(int capacity){
    if (capacity < 0) {
        printf("Failed to allocate with negative capacity \n");
        exit(EXIT_FAILURE);
    }
    else{
        PseudoVector* pseudoVector = (PseudoVector*)(malloc(sizeof(PseudoVector)));
        pseudoVector->capacity = capacity;
        pseudoVector->size = 0;
        if(capacity != 0){
            pseudoVector->data = (int*)malloc(capacity * sizeof(int));
        }
        else{
            pseudoVector->data = NULL;
        }
        return pseudoVector;
    }
}

void pushIntoPseudoVector(PseudoVector* pseudoVector, int val){
    if (pseudoVector->size == pseudoVector->capacity) {
        pseudoVector->capacity = 2 * pseudoVector->capacity + 1;
        int* oldData = pseudoVector->data;
        int* newData = (int*)malloc(pseudoVector->capacity * sizeof(int));
        memcpy(newData, oldData, sizeof(int) * pseudoVector->size);
        pseudoVector->data = newData;
        free(oldData);
    }
    pseudoVector->data[pseudoVector->size] = val;
    pseudoVector->size++;
}

int popFromPseudoVector(PseudoVector* pseudovector){
    if(pseudovector->size == 0){
        printf("Can't pop from empty PseudoVector\n");
        exit(EXIT_FAILURE);
    }
    int val = pseudovector->data[pseudovector->size - 1];
    pseudovector->size--;
    return val;
}



