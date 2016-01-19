#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include "symbolTable.h"
#include "offsetInAR.h"
#include "manageRegister.h"
//#include "printSourceFile.h"

FILE* g_codeGenOutputFp = NULL;
char* g_currentFunctionName = NULL;

char* modifyStringWith000(char* string);
int getLabelNumber();
int codeGenConstantLabel(C_type constantType, void* valuePtr);
void codeGenGetBoolOfFloat(int boolRegIndex, int floatRegIndex);
void codeGenPrepareRegister(ProcessorType processorType, int regIndex, int isAddr, int needToBeLoaded, int workRegIndexIfPseudo, char** regName);
void codeGenSaveToMemoryIfPsuedoRegister(ProcessorType processorType, int regIndex, char* regName);
void codeGenFloatCompInstruction(char *instruction, int dstRegIndex, int srcReg1Index, int srcReg2Index);
void codeGenLogicalInstruction(ProcessorType processorType, char *instruction, int dstRegIndex, int srcReg1Index, int srcReg2Index);
//reg1 is dst
void codeGen2RegInstruction(ProcessorType processorType, char* instruction, int reg1Index, int reg2Index);
//reg1 is dst
void codeGen3RegInstruction(ProcessorType processorType, char* instruction, int reg1Index, int reg2Index, int reg3Index);
void codeGen2Reg1ImmInstruction(ProcessorType processorType, char* instruction, int reg1Index, int reg2Index, void* imm);
int codeGenConvertFromIntToFloat(int intRegIndex);
int codeGenConvertFromFloatToInt(int floatRegIndex);
//*************************

void codeGenProgramNode(AST_NODE *programNode);
void codeGenGlobalVariable(AST_NODE *varaibleDeclListNode);
void codeGenFunctionDeclaration(AST_NODE *functionDeclNode);
void codeGenGeneralNode(AST_NODE* node);
void codeGenStmtNode(AST_NODE* stmtNode);
void codeGenBlockNode(AST_NODE* blockNode);
void codeGenWhileStmt(AST_NODE* whileStmtNode);
void codeGenForStmt(AST_NODE* forStmtNode);
void codeGenIfStmt(AST_NODE* ifStmtNode);
void codeGenReturnStmt(AST_NODE* returnStmtNode);
void codeGenAssignOrExpr(AST_NODE* testNode);
void codeGenAssignmentStmt(AST_NODE* assignmentStmtNode);
void codeGenExprRelatedNode(AST_NODE* exprRelatedNode);
void codeGenExprNode(AST_NODE* exprNode);
void codeGenFunctionCall(AST_NODE* functionCallNode);
void codeGenVariableReference(AST_NODE* idNode);
void codeGenConstantReference(AST_NODE* constantNode);
int codeGenCalcArrayElemenetAddress(AST_NODE* idNode);
void codeGenStoreArgument(AST_NODE *traverseParameter, Parameter* formalParameter);
int codeGenCalcArrayPointerAddress(AST_NODE *idNode);

char* modifyStringWith000(char* originalString){
    const int lengthOfOldString = (int)strlen(originalString);
    const int lengthOfNewString = (int)strlen(originalString) + 4;
    char* newString = (char*)malloc(lengthOfNewString * sizeof(char));
    strcpy(newString, originalString);
    newString[lengthOfOldString-1] = '\\';
    newString[lengthOfOldString] = '0';
    newString[lengthOfOldString+1] = '0';
    newString[lengthOfOldString+2] = '0';
    newString[lengthOfOldString+3] = '"';
    newString[lengthOfOldString+4] = '\0';
    return newString;
}
int getLabelNumber()
{
    static int labelNumber = 0; // only executed once
    return labelNumber++;
}


int codeGenConstantLabel(C_type constantType, void* valuePtr)
{
    int labelNumber = getLabelNumber();
    
    fprintf(g_codeGenOutputFp, ".data\n");

    if(constantType == INTEGERC)
    {
        int* val = (int*)valuePtr;
        fprintf(g_codeGenOutputFp, "_CONSTANT_%d: .word %d\n", labelNumber, *val);
        fprintf(g_codeGenOutputFp, ".align 3\n");
    }
    else if(constantType == FLOATC)
    {
        float* val = (float*)valuePtr;
        fprintf(g_codeGenOutputFp, "_CONSTANT_%d: .float %f\n", labelNumber, *val);
        fprintf(g_codeGenOutputFp, ".align 3\n");
    }
    else if(constantType == STRINGC)
    {
        char* val = (char*)valuePtr;
        fprintf(g_codeGenOutputFp, "_CONSTANT_%d: .ascii %s\n", labelNumber, val);
        fprintf(g_codeGenOutputFp, ".align 3\n");
    }

    fprintf(g_codeGenOutputFp, ".text\n");

    return labelNumber;
}

// ignore float first
void codeGenGetBoolOfFloat(int boolRegIndex, int floatRegIndex)
{
    float zero = 0.0f;
    int constantZeroLabelNumber = codeGenConstantLabel(FLOATC, &zero);

    char* tmpZeroRegName = floatWorkRegisterName[0];
    fprintf(g_codeGenOutputFp, "l.s %s, _CONSTANT_%d\n", tmpZeroRegName, constantZeroLabelNumber);
    char* origFloatRegName = NULL;
    codeGenPrepareRegister(FLOAT_REG, floatRegIndex,0, 1, 1, &origFloatRegName); // need think for isAddr
    fprintf(g_codeGenOutputFp, "c.eq.s %s, %s\n", tmpZeroRegName, origFloatRegName);
    
    char* boolRegName = NULL;
    codeGenPrepareRegister(INT_REG, boolRegIndex,0, 0, 0, &boolRegName); // need think for isAddr
    int tmpLabelIndex = getLabelNumber();
    fprintf(g_codeGenOutputFp, "bc1t _setFalse_%d\n", tmpLabelIndex);
    fprintf(g_codeGenOutputFp, "li %s, %d\n", boolRegName, 1);
    fprintf(g_codeGenOutputFp, "j _setBoolEnd_%d\n", tmpLabelIndex);
    fprintf(g_codeGenOutputFp, "_setFalse_%d:\n", tmpLabelIndex);
    fprintf(g_codeGenOutputFp, "li %s, %d\n", boolRegName, 0);
    fprintf(g_codeGenOutputFp, "_setBoolEnd_%d:\n", tmpLabelIndex);

    codeGenSaveToMemoryIfPsuedoRegister(INT_REG, boolRegIndex, boolRegName);
}


void codeGenPrepareRegister(ProcessorType processorType, int regIndex, int isAddr, int needToBeLoaded, int workRegIndexIfPseudo, char** regName)
{
    int realRegisterCount = (processorType == INT_REG) ? INT_REGISTER_COUNT : FLOAT_REGISTER_COUNT;
    char** realRegisterName;
    char** workRegisterName;
    if (processorType == INT_REG) {
        if (isAddr) {
            realRegisterName = addrRegisterName;
            workRegisterName = addrWorkRegisterName;
        }
        else{
            realRegisterName = intRegisterName;
            workRegisterName = intWorkRegisterName;
        }
    }
    else{
        if (isAddr) {
            printf("floatRegisters can't be used as addrRegisters\n");
            exit(EXIT_FAILURE);
        }
        realRegisterName = floatRegisterName;
        workRegisterName = floatWorkRegisterName;
    }
    char* loadInstruction = "ldr";

    if(regIndex >= realRegisterCount)
    {
        //pseudo register
        int pseudoIndex = regIndex - realRegisterCount;
        *regName = workRegisterName[workRegIndexIfPseudo];
        if(needToBeLoaded)
        {
            fprintf(g_codeGenOutputFp, "%s %s, [x29, #%d]\n", loadInstruction, *regName, getPseudoRegisterCorrespondingOffset(pseudoIndex));
        }
    }
    else
    {
        *regName = realRegisterName[regIndex];
    }
}

// consider addr?
void codeGenSaveToMemoryIfPsuedoRegister(ProcessorType processorType, int regIndex, char* regName)
{
    int realRegisterCount = (processorType == INT_REG) ? INT_REGISTER_COUNT : FLOAT_REGISTER_COUNT;
    char* saveInstruction = "str";

    if(regIndex >= realRegisterCount)
    {
        //pseudo register
        int pseudoIndex = regIndex - realRegisterCount;
        fprintf(g_codeGenOutputFp, "%s %s, [x29, #%d]\n", saveInstruction, regName, getPseudoRegisterCorrespondingOffset(pseudoIndex));
    }
}

// ignore float first
void codeGenFloatCompInstruction(char *Instruction, int dstRegIndex, int srcReg1Index, int srcReg2Index)
{
    char* srcReg1Name = NULL;
    codeGenPrepareRegister(FLOAT_REG, srcReg1Index,0, 1, 0, &srcReg1Name);

    char* srcReg2Name = NULL;
    codeGenPrepareRegister(FLOAT_REG, srcReg2Index,0, 1, 1, &srcReg2Name);

    char* dstRegName = NULL;
    codeGenPrepareRegister(INT_REG, dstRegIndex,0, 0, 0, &dstRegName);
    
    fprintf(g_codeGenOutputFp, "fcmp %s, %s\n", srcReg1Name, srcReg2Name);
    fprintf(g_codeGenOutputFp, "cset %s, %s\n", dstRegName, Instruction);
    
    codeGenSaveToMemoryIfPsuedoRegister(INT_REG, dstRegIndex, dstRegName);
}

// wait for used
void codeGenLogicalInstruction(ProcessorType processorType, char *instruction, int dstRegIndex, int srcReg1Index, int srcReg2Index)
{
    int boolReg1Index = -1;
    int boolReg2Index = -1;

    if(processorType == FLOAT_REG)
    {
        // float logic
        boolReg1Index = getRegister(INT_REG);
        boolReg2Index = getRegister(INT_REG);
        codeGenGetBoolOfFloat(boolReg1Index, srcReg1Index);
        codeGenGetBoolOfFloat(boolReg2Index, srcReg2Index);
    }
    else if(processorType == INT_REG)
    {
        // integer logic
        int zero = 0;
        boolReg1Index = srcReg1Index;
        boolReg2Index = srcReg2Index;
        codeGen2Reg1ImmInstruction(INT_REG, "sne", boolReg1Index, srcReg1Index, &zero);
        codeGen2Reg1ImmInstruction(INT_REG, "sne", boolReg2Index, srcReg2Index, &zero);
    }

    codeGen3RegInstruction(INT_REG, instruction, dstRegIndex, boolReg1Index, boolReg2Index);
    
    if(processorType == FLOAT_REG)
    {
        freeRegister(INT_REG, boolReg1Index);
        freeRegister(INT_REG, boolReg2Index);
    }
}


void codeGen2RegInstruction(ProcessorType processorType, char* instruction, int reg1Index, int reg2Index)
{
    char* reg1Name = NULL;
    codeGenPrepareRegister(processorType, reg1Index,0, 0, 0, &reg1Name); // need think for isAddr
    
    char* reg2Name = NULL;
    codeGenPrepareRegister(processorType, reg2Index,0, 1, 1, &reg2Name);// need think for isAddr
    
    fprintf(g_codeGenOutputFp, "%s %s, %s\n", instruction, reg1Name, reg2Name);

    codeGenSaveToMemoryIfPsuedoRegister(processorType, reg1Index, reg1Name);
}


void codeGen3RegInstruction(ProcessorType processorType, char* instruction, int reg1Index, int reg2Index, int reg3Index)
{
    char* reg1Name = NULL;
    codeGenPrepareRegister(processorType, reg1Index,0, 0, 0, &reg1Name);
    
    char* reg2Name = NULL;
    codeGenPrepareRegister(processorType, reg2Index,0, 1, 0, &reg2Name);
    
    char* reg3Name = NULL;
    codeGenPrepareRegister(processorType, reg3Index,0, 1, 1, &reg3Name);
    
    fprintf(g_codeGenOutputFp, "%s %s, %s, %s\n", instruction, reg1Name, reg2Name, reg3Name);

    codeGenSaveToMemoryIfPsuedoRegister(processorType, reg1Index, reg1Name);
}

void codeGenIntCompInstruction(ProcessorType processorType, char* instruction, int reg1Index, int reg2Index, int reg3Index)
{
    char* reg1Name = NULL;
    codeGenPrepareRegister(processorType, reg1Index,0, 0, 0, &reg1Name);
    
    char* reg2Name = NULL;
    codeGenPrepareRegister(processorType, reg2Index,0, 1, 0, &reg2Name);
    
    char* reg3Name = NULL;
    codeGenPrepareRegister(processorType, reg3Index,0, 1, 1, &reg3Name);
    
    fprintf(g_codeGenOutputFp, "cmp %s, %s\n", reg2Name, reg3Name);
    fprintf(g_codeGenOutputFp, "cset %s, %s\n", reg1Name, instruction);
    
    codeGenSaveToMemoryIfPsuedoRegister(processorType, reg1Index, reg1Name);
}

void codeGen2Reg1ImmInstruction(ProcessorType processorType, char* instruction, int reg1Index, int reg2Index, void* imm)
{
    char* reg1Name = NULL;
    codeGenPrepareRegister(processorType, reg1Index,0, 0, 0, &reg1Name);// need think for isAddr
    
    char* reg2Name = NULL;
    codeGenPrepareRegister(processorType, reg2Index,0, 1, 0, &reg2Name);// need think for isAddr
    
    if(processorType == INT_REG)
    {
        int* val = (int*)imm;
        fprintf(g_codeGenOutputFp, "%s %s, %s, %d\n", instruction, reg1Name, reg2Name, *val);
    }
    else if(processorType == FLOAT_REG)
    {
        float* val = (float*)imm;
        fprintf(g_codeGenOutputFp, "%s %s, %s, %f\n", instruction, reg1Name, reg2Name, *val);
    }

    codeGenSaveToMemoryIfPsuedoRegister(processorType, reg1Index, reg1Name);
}

void codeGen2Reg1ImmInstruction_64(ProcessorType processorType, char* instruction, int reg1Index, int reg2Index, void* imm)
{
    char* reg1Name = NULL;
    codeGenPrepareRegister(processorType, reg1Index, 1, 0, 0, &reg1Name);
    
    char* reg2Name = NULL;
    codeGenPrepareRegister(processorType, reg2Index, 1, 1, 1, &reg2Name);
    
    if(processorType == INT_REG)
    {
        int* val = (int*)imm;
        fprintf(g_codeGenOutputFp, "%s %s, %s, #%d\n", instruction, reg1Name, reg2Name, *val);
    }
    else if(processorType == FLOAT_REG)
    {
        float* val = (float*)imm;
        fprintf(g_codeGenOutputFp, "%s %s, %s, #%f\n", instruction, reg1Name, reg2Name, *val);
    }
    
    codeGenSaveToMemoryIfPsuedoRegister(processorType, reg1Index, reg1Name);
}



void codeGen2Reg1ImmFloatLogicalInstruction(char* instruction, int reg1Index, int reg2Index, void* fimm)
{
    char* reg1Name = NULL;
    codeGenPrepareRegister(INT_REG, reg1Index,0, 0, 0, &reg1Name);// need think for isAddr
    
    char* reg2Name = NULL;
    codeGenPrepareRegister(FLOAT_REG, reg2Index,0, 1, 0, &reg2Name);// need think for isAddr
    
    float* val = (float*)fimm;
    fprintf(g_codeGenOutputFp, "fcmp %s, #%f", reg2Name, *val);
    fprintf(g_codeGenOutputFp, "cset %s, %s", reg1Name, instruction);
    
    codeGenSaveToMemoryIfPsuedoRegister(INT_REG, reg1Index, reg1Name);
}
// useful
void codeGen2Reg1ImmIntLogicalInstruction(char* instruction, int reg1Index, int reg2Index, void* imm)
{
    char* reg1Name = NULL;
    codeGenPrepareRegister(INT_REG, reg1Index,0, 0, 0, &reg1Name);// need think for isAddr
    
    char* reg2Name = NULL;
    codeGenPrepareRegister(INT_REG, reg2Index,0, 1, 0, &reg2Name);// need think for isAddr
    
    int* val = (int*)imm;
    fprintf(g_codeGenOutputFp, "cmp %s, #%d", reg2Name, *val);
    fprintf(g_codeGenOutputFp, "cset %s, %s", reg1Name, instruction);
    
    codeGenSaveToMemoryIfPsuedoRegister(INT_REG, reg1Index, reg1Name);
}



int codeGenConvertFromIntToFloat(int intRegIndex)
{
    /*TODO*/
    char *intRegisterName, *floatRegisterName;
    int floatRegisterIndex = getRegister(FLOAT_REG);
    codeGenPrepareRegister(INT_REG, intRegIndex,0, 1, 0, &intRegisterName);// need think for isAddr
    codeGenPrepareRegister(FLOAT_REG, floatRegisterIndex,0, 0, 0, &floatRegisterName);// need think for isAddr

    fprintf(g_codeGenOutputFp, "scvtf %s, %s\n", floatRegisterName, intRegisterName);
//    fprintf(g_codeGenOutputFp, "mtc1 %s, %s\n", intRegisterName, floatRegisterName);
//    fprintf(g_codeGenOutputFp, "cvt.s.w %s, %s\n", floatRegisterName, floatRegisterName);

    freeRegister(INT_REG, intRegIndex);

    return floatRegisterIndex;
}

// no need for this phase
int codeGenConvertFromFloatToInt(int floatRegIndex)
{
    /*TODO*/
    char *intRegisterName, *floatRegisterName;
    int intRegisterIndex = getRegister(INT_REG);
    codeGenPrepareRegister(INT_REG, intRegisterIndex,0, 1, 0, &intRegisterName); // need think isAddr
    codeGenPrepareRegister(FLOAT_REG, floatRegIndex,0, 0, 0, &floatRegisterName);// need think for isAddr

    fprintf(g_codeGenOutputFp, "fcvtzs %s, %s\n", intRegisterName, floatRegisterName);
//    fprintf(g_codeGenOutputFp, "cvt.w.s %s, %s\n", floatRegisterName, floatRegisterName);
//    fprintf(g_codeGenOutputFp, "mfc1 %s, %s\n", intRegisterName, floatRegisterName);

    freeRegister(FLOAT_REG, floatRegIndex);

    return intRegisterIndex;
}


void codeGenerate(AST_NODE *root)
{
    char* outputfileName = "output.s";
    g_codeGenOutputFp = fopen(outputfileName, "w");
    if(!g_codeGenOutputFp)
    {
        printf("Cannot open file \"%s\"", outputfileName);
        exit(EXIT_FAILURE);
    }

    codeGenProgramNode(root);
}


void codeGenProgramNode(AST_NODE *programNode)
{
    AST_NODE *traverseDeclaration = programNode->child;
    while(traverseDeclaration)
    {
        if(traverseDeclaration->nodeType == VARIABLE_DECL_LIST_NODE)
        {
            fprintf(g_codeGenOutputFp, ".data\n");
            codeGenGlobalVariable(traverseDeclaration);
            fprintf(g_codeGenOutputFp, ".text\n");
        }
        else if(traverseDeclaration->nodeType == DECLARATION_NODE)
        {
            codeGenFunctionDeclaration(traverseDeclaration);
        }
        traverseDeclaration = traverseDeclaration->rightSibling;
    }
    return;
}


void codeGenGlobalVariable(AST_NODE* varaibleDeclListNode)
{
    AST_NODE *traverseDeclaration = varaibleDeclListNode->child;
    while(traverseDeclaration)
    {
        if(traverseDeclaration->semantic_value.declSemanticValue.kind == VARIABLE_DECL)
        {
            AST_NODE *typeNode = traverseDeclaration->child;
            AST_NODE *idNode = typeNode->rightSibling;
            while(idNode)
            {
                /*TODO initial*/
                void *val;
                int intZero = 0;
                float floatZero = 0.0f;
                // scalar variable
                if(idNode->semantic_value.identifierSemanticValue.kind == WITH_INIT_ID) {
                    AST_NODE *constValueNode = idNode->child;
                    if(typeNode->dataType == INT_TYPE) 
                        val = &constValueNode->semantic_value.exprSemanticValue.constEvalValue.iValue;
                    else if(typeNode->dataType == FLOAT_TYPE) 
                        val = &constValueNode->semantic_value.exprSemanticValue.constEvalValue.fValue;
                }
                else {
                    if(typeNode->dataType == INT_TYPE) 
                        val = &intZero;
                    else if(typeNode->dataType == FLOAT_TYPE) 
                        val = &floatZero;
                }

                SymbolTableEntry* idSymbolTableEntry = idNode->semantic_value.identifierSemanticValue.symbolTableEntry;
                TypeDescriptor* idTypeDescriptor = idSymbolTableEntry->attribute->attr.typeDescriptor;
                if(idTypeDescriptor->kind == SCALAR_TYPE_DESCRIPTOR)
                {
                    if(idTypeDescriptor->properties.dataType == INT_TYPE)
                    {
                        fprintf(g_codeGenOutputFp, "_g_%s: .word %d\n", idSymbolTableEntry->name, *(int*)val);
                    }
                    else if(idTypeDescriptor->properties.dataType == FLOAT_TYPE)
                    {
                        fprintf(g_codeGenOutputFp, "_g_%s: .float %f\n", idSymbolTableEntry->name, *(float*)val);
                    }
                }
                // not for this phase
                else if(idTypeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR)
                {
                    int variableSize = getVariableSize(idTypeDescriptor);
                    fprintf(g_codeGenOutputFp, "_g_%s: .space %d\n", idSymbolTableEntry->name, variableSize);
                }
                idNode = idNode->rightSibling;
            }
        }
        traverseDeclaration = traverseDeclaration->rightSibling;
    }
    return;
}


void codeGenFunctionDeclaration(AST_NODE *functionDeclNode)
{
    AST_NODE* functionIdNode = functionDeclNode->child->rightSibling;
    
    g_currentFunctionName = functionIdNode->semantic_value.identifierSemanticValue.identifierName;

    fprintf(g_codeGenOutputFp, ".text\n");
    if (strcmp(functionIdNode->semantic_value.identifierSemanticValue.identifierName, "main") != 0) {
        fprintf(g_codeGenOutputFp, "_start_%s:\n", functionIdNode->semantic_value.identifierSemanticValue.identifierName);
    } else {
        // for main -> _start_MAIN:
        fprintf(g_codeGenOutputFp, "_start_MAIN:\n");
    }
    
    //prologue
    fprintf(g_codeGenOutputFp, "str x30, [sp, #0]\n");
    fprintf(g_codeGenOutputFp, "str x29, [sp, #-8]\n");
    fprintf(g_codeGenOutputFp, "add x29, sp, #-8\n");
    fprintf(g_codeGenOutputFp, "add sp, sp, #-16\n");
    fprintf(g_codeGenOutputFp, "ldr x30, =_frameSize_%s\n", functionIdNode->semantic_value.identifierSemanticValue.identifierName);
    fprintf(g_codeGenOutputFp, "ldr x30, [x30, #0]\n");
    fprintf(g_codeGenOutputFp, "sub sp, sp, w30\n");
    printStoreRegister(g_codeGenOutputFp);
    
    //reset register
    resetRegisterTable(functionIdNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->offsetInAR);

    AST_NODE* blockNode = functionIdNode->rightSibling->rightSibling;
    AST_NODE *traverseListNode = blockNode->child;
    while(traverseListNode)
    {
        codeGenGeneralNode(traverseListNode);
        traverseListNode = traverseListNode->rightSibling;
    }

    //epilogue
    fprintf(g_codeGenOutputFp, "_end_%s:\n", g_currentFunctionName);
    printRestoreRegister(g_codeGenOutputFp);
    fprintf(g_codeGenOutputFp, "ldr x30, [x29, #8]\n");
    fprintf(g_codeGenOutputFp, "mov sp, x29\n");
    fprintf(g_codeGenOutputFp, "add sp, sp, #8\n");
    fprintf(g_codeGenOutputFp, "ldr x29, [x29,#0]\n");
    fprintf(g_codeGenOutputFp, "RET x30\n");
    fprintf(g_codeGenOutputFp, ".data\n");
    // magic number 4: one float allocated with 8 bytes
    int frameSize = abs(functionIdNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->offsetInAR) + 
        (INT_REGISTER_COUNT*8 + INT_WORK_REGISTER_COUNT*8 + FLOAT_REGISTER_COUNT*4 + FLOAT_WORK_REGISTER_COUNT*4 + 4) +
        g_pseudoRegisterTable.isAllocatedVector->size * 8;
    fprintf(g_codeGenOutputFp, "_frameSize_%s: .word %d\n", functionIdNode->semantic_value.identifierSemanticValue.identifierName, frameSize);
    return;
}


void codeGenBlockNode(AST_NODE* blockNode)
{
    AST_NODE *traverseListNode = blockNode->child;
    while(traverseListNode)
    {
        codeGenGeneralNode(traverseListNode);
        traverseListNode = traverseListNode->rightSibling;
    }
}


void codeGenExprNode(AST_NODE* exprNode)
{
    if(exprNode->semantic_value.exprSemanticValue.kind == BINARY_OPERATION &&
       exprNode->semantic_value.exprSemanticValue.op.binaryOp != BINARY_OP_AND &&
       exprNode->semantic_value.exprSemanticValue.op.binaryOp != BINARY_OP_OR)
    {
        AST_NODE* leftOp = exprNode->child;
        AST_NODE* rightOp = leftOp->rightSibling;
        codeGenExprRelatedNode(leftOp);
        codeGenExprRelatedNode(rightOp);
        // not in this phase
        if(leftOp->dataType == FLOAT_TYPE || rightOp->dataType == FLOAT_TYPE)
        {
            if(leftOp->dataType == INT_TYPE)
            {
                leftOp->registerIndex = codeGenConvertFromIntToFloat(leftOp->registerIndex);
            }
            //else if
            if(rightOp->dataType == INT_TYPE)
            {
                rightOp->registerIndex = codeGenConvertFromIntToFloat(rightOp->registerIndex);
            }
            
            switch(exprNode->semantic_value.exprSemanticValue.op.binaryOp)
            {
            case BINARY_OP_ADD:
                exprNode->registerIndex = leftOp->registerIndex;
                codeGen3RegInstruction(FLOAT_REG, "fadd", exprNode->registerIndex, leftOp->registerIndex, rightOp->registerIndex);
                break;
            case BINARY_OP_SUB:
                exprNode->registerIndex = leftOp->registerIndex;
                codeGen3RegInstruction(FLOAT_REG, "fsub", exprNode->registerIndex, leftOp->registerIndex, rightOp->registerIndex);
                break;
            case BINARY_OP_MUL:
                exprNode->registerIndex = leftOp->registerIndex;
                codeGen3RegInstruction(FLOAT_REG, "fmul", exprNode->registerIndex, leftOp->registerIndex, rightOp->registerIndex);
                break;
            case BINARY_OP_DIV:
                exprNode->registerIndex = leftOp->registerIndex;
                codeGen3RegInstruction(FLOAT_REG, "fdiv", exprNode->registerIndex, leftOp->registerIndex, rightOp->registerIndex);
                break;
            case BINARY_OP_EQ:
                exprNode->registerIndex = getRegister(INT_REG);
                codeGenFloatCompInstruction("eq", exprNode->registerIndex, leftOp->registerIndex, rightOp->registerIndex);
                freeRegister(FLOAT_REG, leftOp->registerIndex);
                break;
            case BINARY_OP_GE:
                exprNode->registerIndex = getRegister(INT_REG);
                codeGenFloatCompInstruction("ge", exprNode->registerIndex, leftOp->registerIndex, rightOp->registerIndex);
                freeRegister(FLOAT_REG, leftOp->registerIndex);
                break;
            case BINARY_OP_LE:
                exprNode->registerIndex = getRegister(INT_REG);
                codeGenFloatCompInstruction("le", exprNode->registerIndex, leftOp->registerIndex, rightOp->registerIndex);
                freeRegister(FLOAT_REG, leftOp->registerIndex);
                break;
            case BINARY_OP_NE:
                exprNode->registerIndex = getRegister(INT_REG);
                codeGenFloatCompInstruction("ne", exprNode->registerIndex, leftOp->registerIndex, rightOp->registerIndex);
                freeRegister(FLOAT_REG, leftOp->registerIndex);
                break;
            case BINARY_OP_GT:
                exprNode->registerIndex = getRegister(INT_REG);
                codeGenFloatCompInstruction("gt", exprNode->registerIndex, leftOp->registerIndex, rightOp->registerIndex);
                freeRegister(FLOAT_REG, leftOp->registerIndex);
                break;
            case BINARY_OP_LT:
                exprNode->registerIndex = getRegister(INT_REG);
                codeGenFloatCompInstruction("lt", exprNode->registerIndex, leftOp->registerIndex, rightOp->registerIndex);
                freeRegister(FLOAT_REG, leftOp->registerIndex);
                break;
            default:
                printf("Unhandled case in void evaluateExprValue(AST_NODE* exprNode)\n");
                break;
            }

            freeRegister(FLOAT_REG, rightOp->registerIndex);
        }//endif at least one float operand
        else if(exprNode->dataType == INT_TYPE)
        {
            exprNode->registerIndex = leftOp->registerIndex;
            switch(exprNode->semantic_value.exprSemanticValue.op.binaryOp)
            {
            case BINARY_OP_ADD:
                codeGen3RegInstruction(INT_REG, "add", exprNode->registerIndex, leftOp->registerIndex, rightOp->registerIndex);
                break;
            case BINARY_OP_SUB:
                codeGen3RegInstruction(INT_REG, "sub", exprNode->registerIndex, leftOp->registerIndex, rightOp->registerIndex);
                break;
            case BINARY_OP_MUL:
                codeGen3RegInstruction(INT_REG, "mul", exprNode->registerIndex, leftOp->registerIndex, rightOp->registerIndex);
                break;
            case BINARY_OP_DIV:
                codeGen3RegInstruction(INT_REG, "sdiv", exprNode->registerIndex, leftOp->registerIndex, rightOp->registerIndex);
                break;
            case BINARY_OP_EQ:
                codeGenIntCompInstruction(INT_REG, "eq", exprNode->registerIndex, leftOp->registerIndex, rightOp->registerIndex);
                break;
            case BINARY_OP_GE:
                codeGenIntCompInstruction(INT_REG, "ge", exprNode->registerIndex, leftOp->registerIndex, rightOp->registerIndex);
                break;
            case BINARY_OP_LE:
                codeGenIntCompInstruction(INT_REG, "le", exprNode->registerIndex, leftOp->registerIndex, rightOp->registerIndex);
                break;
            case BINARY_OP_NE:
                codeGenIntCompInstruction(INT_REG, "ne", exprNode->registerIndex, leftOp->registerIndex, rightOp->registerIndex);
                break;
            case BINARY_OP_GT:
                codeGenIntCompInstruction(INT_REG, "gt", exprNode->registerIndex, leftOp->registerIndex, rightOp->registerIndex);
                break;
            case BINARY_OP_LT:
                codeGenIntCompInstruction(INT_REG, "lt", exprNode->registerIndex, leftOp->registerIndex, rightOp->registerIndex);
                break;
            default:
                printf("Unhandled case in void evaluateExprValue(AST_NODE* exprNode)\n");
                break;
            }

            freeRegister(INT_REG, rightOp->registerIndex);
        }//endif 2 int operands
    }//endif BINARY_OPERATION
    else if(exprNode->semantic_value.exprSemanticValue.kind == BINARY_OPERATION) {
        AST_NODE* leftOp = exprNode->child;
        AST_NODE* rightOp = leftOp->rightSibling;
        int labelNumber = getLabelNumber();
        char *leftOpRegName, *rightOpRegName;

        if(leftOp->dataType == FLOAT_TYPE || rightOp->dataType == FLOAT_TYPE)
        {

            char *exprRegName;
            exprNode->registerIndex = getRegister(INT_REG);
            codeGenPrepareRegister(INT_REG, exprNode->registerIndex,0, 0, 0, &exprRegName);
            
            switch(exprNode->semantic_value.exprSemanticValue.op.binaryOp)
            {
            case BINARY_OP_AND:
                codeGenExprRelatedNode(leftOp);
                if(leftOp->dataType == INT_TYPE)
                    leftOp->registerIndex = codeGenConvertFromIntToFloat(leftOp->registerIndex);
                codeGenPrepareRegister(FLOAT_REG, leftOp->registerIndex,0, 1, 1, &leftOpRegName);
                fprintf(g_codeGenOutputFp, "fcmp %s, #0.0\n", leftOpRegName);
                fprintf(g_codeGenOutputFp, "beq _booleanFalse%d\n", labelNumber);
                codeGenExprRelatedNode(rightOp);
                if(rightOp->dataType == INT_TYPE)
                    rightOp->registerIndex = codeGenConvertFromIntToFloat(rightOp->registerIndex);
                codeGenPrepareRegister(FLOAT_REG, rightOp->registerIndex,0, 1, 1, &rightOpRegName);
                fprintf(g_codeGenOutputFp, "fcmp %s, #0.0\n", rightOpRegName);
                fprintf(g_codeGenOutputFp, "beq _booleanFalse%d\n", labelNumber);
                fprintf(g_codeGenOutputFp, "_booleanTrue%d:\n", labelNumber);
                fprintf(g_codeGenOutputFp, "mov %s, #%d\n", exprRegName, 1);
                fprintf(g_codeGenOutputFp, "b _booleanExit%d\n", labelNumber);
                fprintf(g_codeGenOutputFp, "_booleanFalse%d:\n", labelNumber);
                fprintf(g_codeGenOutputFp, "mov %s, #%d\n", exprRegName, 0);
                fprintf(g_codeGenOutputFp, "_booleanExit%d:\n", labelNumber);
                break;
            case BINARY_OP_OR:
                codeGenExprRelatedNode(leftOp);
                if(leftOp->dataType == INT_TYPE)
                    leftOp->registerIndex = codeGenConvertFromIntToFloat(leftOp->registerIndex);
                codeGenPrepareRegister(FLOAT_REG, leftOp->registerIndex,0, 1, 1, &leftOpRegName);// need think for isAddr
                fprintf(g_codeGenOutputFp, "fcmp %s, #0.0\n", leftOpRegName);
                fprintf(g_codeGenOutputFp, "bne _booleanTrue%d\n", labelNumber);
                codeGenExprRelatedNode(rightOp);
                if(rightOp->dataType == INT_TYPE)
                    rightOp->registerIndex = codeGenConvertFromIntToFloat(rightOp->registerIndex);
                codeGenPrepareRegister(FLOAT_REG, rightOp->registerIndex,0, 1, 1, &rightOpRegName);// need think for isAddr
                fprintf(g_codeGenOutputFp, "fcmp %s, #0.0\n", rightOpRegName);
                fprintf(g_codeGenOutputFp, "bne _booleanTrue%d\n", labelNumber);
                fprintf(g_codeGenOutputFp, "_booleanFalse%d:\n", labelNumber);
                fprintf(g_codeGenOutputFp, "mov %s, #%d\n", exprRegName, 0);
                fprintf(g_codeGenOutputFp, "b _booleanExit%d\n", labelNumber);
                fprintf(g_codeGenOutputFp, "_booleanTrue%d:\n", labelNumber);
                fprintf(g_codeGenOutputFp, "mov %s, #%d\n", exprRegName, 1);
                fprintf(g_codeGenOutputFp, "_booleanExit%d:\n", labelNumber);
                break;
            default:
                printf("Unhandled case in void evaluateExprValue(AST_NODE* exprNode)\n");
                break;
            }

            freeRegister(FLOAT_REG, leftOp->registerIndex);
            freeRegister(FLOAT_REG, rightOp->registerIndex);
        }//endif at least one float operand
        else if(exprNode->dataType == INT_TYPE)
        {
            switch(exprNode->semantic_value.exprSemanticValue.op.binaryOp)
            {
            case BINARY_OP_AND:
                codeGenExprRelatedNode(leftOp);
                codeGenPrepareRegister(INT_REG, leftOp->registerIndex,0, 1, 0, &leftOpRegName);
                fprintf(g_codeGenOutputFp, "cmp %s, #0\n", leftOpRegName);
                fprintf(g_codeGenOutputFp, "beq _booleanFalse%d\n", labelNumber);
                codeGenExprRelatedNode(rightOp);
                codeGenPrepareRegister(INT_REG, rightOp->registerIndex,0, 1, 0, &rightOpRegName);
                fprintf(g_codeGenOutputFp, "cmp %s, #0\n", rightOpRegName);
                fprintf(g_codeGenOutputFp, "beq _booleanFalse%d\n", labelNumber);
                fprintf(g_codeGenOutputFp, "_booleanTrue%d:\n", labelNumber);
                fprintf(g_codeGenOutputFp, "mov %s, #%d\n", leftOpRegName, 1);
                fprintf(g_codeGenOutputFp, "b _booleanExit%d\n", labelNumber);
                fprintf(g_codeGenOutputFp, "_booleanFalse%d:\n", labelNumber);
                fprintf(g_codeGenOutputFp, "mov %s, #%d\n", leftOpRegName, 0);
                fprintf(g_codeGenOutputFp, "_booleanExit%d:\n", labelNumber);
                break;
            case BINARY_OP_OR:
                codeGenExprRelatedNode(leftOp);
                codeGenPrepareRegister(INT_REG, leftOp->registerIndex,0, 1, 0, &leftOpRegName);
                fprintf(g_codeGenOutputFp, "cmp %s, #0\n", leftOpRegName);
                fprintf(g_codeGenOutputFp, "bne _booleanTrue%d\n", labelNumber);
                codeGenExprRelatedNode(rightOp);
                codeGenPrepareRegister(INT_REG, rightOp->registerIndex,0, 1, 0, &rightOpRegName);
                fprintf(g_codeGenOutputFp, "cmp %s, #0\n", rightOpRegName);
                fprintf(g_codeGenOutputFp, "bne _booleanTrue%d\n", labelNumber);
                fprintf(g_codeGenOutputFp, "_booleanFalse%d:\n", labelNumber);
                fprintf(g_codeGenOutputFp, "mov %s, #%d\n", leftOpRegName, 0);
                fprintf(g_codeGenOutputFp, "b _booleanExit%d\n", labelNumber);
                fprintf(g_codeGenOutputFp, "_booleanTrue%d:\n", labelNumber);
                fprintf(g_codeGenOutputFp, "mov %s, #%d\n", leftOpRegName, 1);
                fprintf(g_codeGenOutputFp, "_booleanExit%d:\n", labelNumber);
                break;
            default:
                printf("Unhandled case in void evaluateExprValue(AST_NODE* exprNode)\n");
                break;
            }
            exprNode->registerIndex = leftOp->registerIndex;

            freeRegister(INT_REG, rightOp->registerIndex);
        }//endif 2 int operands
    }
    else if(exprNode->semantic_value.exprSemanticValue.kind == UNARY_OPERATION)
    {
        int tmpZero = 0;
        AST_NODE* operand = exprNode->child;
        codeGenExprRelatedNode(operand);
        if(operand->dataType == FLOAT_TYPE)
        {
            // set for 0.0
            float zero = 0.0f;
            int constantZeroLabelNumber = codeGenConstantLabel(FLOATC, &zero);
            int zeroIndex = getRegister(FLOAT_REG);
            char *zeroRegName;
            codeGenPrepareRegister(FLOAT_REG, zeroIndex,0, 0, 0, &zeroRegName);
            fprintf(g_codeGenOutputFp, "ldr %s, =_CONSTANT_%d\n", addrWorkRegisterName[0], constantZeroLabelNumber);
            fprintf(g_codeGenOutputFp, "ldr %s, [%s, #0]\n", zeroRegName, addrWorkRegisterName[0]);
            
            
            
            switch(exprNode->semantic_value.exprSemanticValue.op.unaryOp)
            {
            case UNARY_OP_POSITIVE:
                exprNode->registerIndex = operand->registerIndex;
                break;
            case UNARY_OP_NEGATIVE:
                exprNode->registerIndex = operand->registerIndex;
                char* reg1Name = NULL;
                codeGenPrepareRegister(FLOAT_REG, exprNode->registerIndex,0, 0, 0, &reg1Name);
                char* reg2Name = NULL;
                codeGenPrepareRegister(FLOAT_REG, exprNode->registerIndex,0, 1, 1, &reg2Name);
                fprintf(g_codeGenOutputFp, "fsub %s, %s, %s\n", reg1Name, zeroRegName, reg2Name);
                codeGenSaveToMemoryIfPsuedoRegister(FLOAT_REG, exprNode->registerIndex, reg1Name);
                break;
            case UNARY_OP_LOGICAL_NEGATION:
                exprNode->registerIndex = getRegister(INT_REG);
                //codeGenGetBoolOfFloat(exprNode->registerIndex, operand->registerIndex);
                codeGen2Reg1ImmFloatLogicalInstruction("eq", exprNode->registerIndex, operand->registerIndex, &zero);
                freeRegister(FLOAT_REG, operand->registerIndex);
                break;
            default:
                printf("Unhandled case in void evaluateExprValue(AST_NODE* exprNode)\n");
                break;
            }
            // free 0.0
            freeRegister(FLOAT_REG, zeroIndex);
        }
        else if(operand->dataType == INT_TYPE)
        {
            // set for 0
            int zero = 0;
            int constantZeroLabelNumber = codeGenConstantLabel(INTEGERC, &zero);
            int zeroIndex = getRegister(INT_REG);
            char *zeroRegName;
            codeGenPrepareRegister(INT_REG, zeroIndex,0, 0, 0, &zeroRegName);
            fprintf(g_codeGenOutputFp, "ldr %s, =_CONSTANT_%d\n", addrWorkRegisterName[0], constantZeroLabelNumber);
            fprintf(g_codeGenOutputFp, "ldr %s, [%s, #0]\n", zeroRegName, addrWorkRegisterName[0]);
            
            switch(exprNode->semantic_value.exprSemanticValue.op.unaryOp)
            {
            case UNARY_OP_POSITIVE:
                exprNode->registerIndex = operand->registerIndex;
                break;
            case UNARY_OP_NEGATIVE:
                exprNode->registerIndex = operand->registerIndex;
                char* reg1Name = NULL;
                codeGenPrepareRegister(INT_REG, exprNode->registerIndex,0, 0, 0, &reg1Name);
                char* reg2Name = NULL;
                codeGenPrepareRegister(INT_REG, exprNode->registerIndex,0, 1, 1, &reg2Name);
                fprintf(g_codeGenOutputFp, "sub %s, %s, %s\n", reg1Name, zeroRegName, reg2Name);
                codeGenSaveToMemoryIfPsuedoRegister(INT_REG, exprNode->registerIndex, reg1Name);
                break;
            case UNARY_OP_LOGICAL_NEGATION:
                exprNode->registerIndex = operand->registerIndex;
                codeGen2Reg1ImmIntLogicalInstruction("eq", exprNode->registerIndex, operand->registerIndex, &zero);
                break;
            default:
                printf("Unhandled case in void evaluateExprValue(AST_NODE* exprNode)\n");
                break;
            }
            freeRegister(INT_REG, zeroIndex);
        }
    }
}


int codeGenCalcArrayPointerAddress(AST_NODE *idNode) {
    AST_NODE* traverseDim = idNode->child;
    int* sizeInEachDimension = idNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension;
    int dimension = idNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor->properties.arrayProperties.dimension;
            
    codeGenExprRelatedNode(traverseDim);
    int linearIdxRegisterIndex = traverseDim->registerIndex;
    traverseDim = traverseDim->rightSibling;

    int dimIndex = 1;
    int dimRegIndex = getRegister(INT_REG);
    char *totalRegName, *oneDimRegName, *dimConstRegName;
    /*TODO multiple dimensions*/
    while(traverseDim)
    {
        codeGenPrepareRegister(INT_REG, linearIdxRegisterIndex,0, 1, 0, &totalRegName);// need think for isAddr
        codeGenPrepareRegister(INT_REG, dimRegIndex,0, 0, 1, &dimConstRegName);// need think for isAddr
        fprintf(g_codeGenOutputFp, "mov %s, #%d\n", dimConstRegName, sizeInEachDimension[dimIndex]);
        fprintf(g_codeGenOutputFp, "mul %s, %s, %s\n", totalRegName, totalRegName, dimConstRegName);
        codeGenExprRelatedNode(traverseDim);
        codeGenPrepareRegister(INT_REG, traverseDim->registerIndex,0, 1, 1, &oneDimRegName);// need think for isAddr
        fprintf(g_codeGenOutputFp, "add %s, %s, %s\n", totalRegName, totalRegName, oneDimRegName);

        freeRegister(INT_REG, traverseDim->registerIndex);
        dimIndex++;
        traverseDim = traverseDim->rightSibling;
    }
    while(dimIndex < dimension) {
        codeGenPrepareRegister(INT_REG, linearIdxRegisterIndex,0, 1, 0, &totalRegName);// need think for isAddr
        codeGenPrepareRegister(INT_REG, dimRegIndex,0, 0, 1, &dimConstRegName);// need think for isAddr
        fprintf(g_codeGenOutputFp, "mov %s, #%d\n", dimConstRegName, sizeInEachDimension[dimIndex]);
        fprintf(g_codeGenOutputFp, "mul %s, %s, %s\n", totalRegName, totalRegName, dimConstRegName);

        dimIndex++;
    }
    freeRegister(INT_REG, dimRegIndex);
    
    int shiftLeftTwoBits = 2;
    codeGen2Reg1ImmInstruction_64(INT_REG, "lsl", linearIdxRegisterIndex, linearIdxRegisterIndex, &shiftLeftTwoBits);
    
    char* linearOffsetRegName = NULL;
    if(!isGlobalVariable(idNode->semantic_value.identifierSemanticValue.symbolTableEntry))
    {
        int baseOffset = idNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->offsetInAR;
        codeGen2Reg1ImmInstruction_64(INT_REG, "add", linearIdxRegisterIndex, linearIdxRegisterIndex, &baseOffset);
        codeGenPrepareRegister(INT_REG, linearIdxRegisterIndex,1, 1, 0, &linearOffsetRegName);// need think for isAddr
        fprintf(g_codeGenOutputFp, "add %s, %s, x29\n", linearOffsetRegName, linearOffsetRegName);
    }
    else
    {
        fprintf(g_codeGenOutputFp, "ldr %s, =_g_%s\n", addrWorkRegisterName[0], idNode->semantic_value.identifierSemanticValue.identifierName);
        codeGenPrepareRegister(INT_REG, linearIdxRegisterIndex,1, 1, 1, &linearOffsetRegName);// need think for isAddr
        fprintf(g_codeGenOutputFp, "add %s, %s, %s\n", linearOffsetRegName, linearOffsetRegName, addrWorkRegisterName[0]);
    }

    codeGenSaveToMemoryIfPsuedoRegister(INT_REG, linearIdxRegisterIndex, linearOffsetRegName);

    return linearIdxRegisterIndex;
}


void codeGenStoreArgument(AST_NODE *traverseParameter, Parameter* formalParameter) {
    if(traverseParameter->rightSibling)
        codeGenStoreArgument(traverseParameter->rightSibling, formalParameter->next);

    char* parameterRegName = NULL;
    if(traverseParameter->dataType == INT_TYPE) {
        codeGenExprRelatedNode(traverseParameter);
        
        if(formalParameter->type->properties.dataType == FLOAT_TYPE) {// transfer int -> float
            int floatIndex = codeGenConvertFromIntToFloat(traverseParameter->registerIndex);
            codeGenPrepareRegister(FLOAT_REG, floatIndex,0, 1, 0, &parameterRegName);// need think for isAddr
            fprintf(g_codeGenOutputFp, "str %s, [sp, #0]\n", parameterRegName);
            freeRegister(FLOAT_REG, floatIndex);
        }
        else {
            codeGenPrepareRegister(INT_REG, traverseParameter->registerIndex,0, 1, 0, &parameterRegName);// need think for isAddr
            fprintf(g_codeGenOutputFp, "str %s, [sp, #0]\n", parameterRegName);
            freeRegister(INT_REG, traverseParameter->registerIndex);
        }
    }
    else if(traverseParameter->dataType == FLOAT_TYPE) {
        codeGenExprRelatedNode(traverseParameter);

        if(formalParameter->type->properties.dataType == INT_TYPE) {
            int intIndex = codeGenConvertFromFloatToInt(traverseParameter->registerIndex);
            codeGenPrepareRegister(INT_REG, intIndex,0, 1, 0, &parameterRegName);// need think for isAddr
            fprintf(g_codeGenOutputFp, "str %s, [sp, #0]\n", parameterRegName);
            freeRegister(INT_REG, intIndex);
        }
        else {
            codeGenPrepareRegister(FLOAT_REG, traverseParameter->registerIndex,0, 1, 0, &parameterRegName);// need think for isAddr
            fprintf(g_codeGenOutputFp, "str %s, [sp, #0]\n", parameterRegName);
            freeRegister(FLOAT_REG, traverseParameter->registerIndex);
        }
    }
    else if(traverseParameter->dataType == INT_PTR_TYPE || traverseParameter->dataType == FLOAT_PTR_TYPE) {
        int offsetIndex;
        char *offsetRegName;
        if(traverseParameter->semantic_value.identifierSemanticValue.kind == ARRAY_ID) {
            offsetIndex = codeGenCalcArrayPointerAddress(traverseParameter);
            codeGenPrepareRegister(INT_REG, offsetIndex,1, 0, 0, &offsetRegName);// need think for isAddr
        }
        else {
            offsetIndex = getRegister(INT_REG);
            codeGenPrepareRegister(INT_REG, offsetIndex,1, 0, 0, &offsetRegName);// need think for isAddr

            if(!isGlobalVariable(traverseParameter->semantic_value.identifierSemanticValue.symbolTableEntry)) {
                fprintf(g_codeGenOutputFp, "mov %s, #%d\n", offsetRegName, traverseParameter->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->offsetInAR);
                fprintf(g_codeGenOutputFp, "add %s, %s, x29\n", offsetRegName, offsetRegName);
            }
            else {
                fprintf(g_codeGenOutputFp, "ldr %s, =_g_%s\n", offsetRegName, traverseParameter->semantic_value.identifierSemanticValue.identifierName);
            }
        }
        fprintf(g_codeGenOutputFp, "str %s, [sp, #0]\n", offsetRegName);

        freeRegister(INT_REG, offsetIndex);
    }
    fprintf(g_codeGenOutputFp, "add sp, sp, -8\n");
}


void codeGenFunctionCall(AST_NODE* functionCallNode)
{
    AST_NODE* functionIdNode = functionCallNode->child;
    AST_NODE* parameterList = functionIdNode->rightSibling;
    if(strcmp(functionIdNode->semantic_value.identifierSemanticValue.identifierName, "write") == 0)
    {
        AST_NODE* firstParameter = parameterList->child;
        codeGenExprRelatedNode(firstParameter);
        char* parameterRegName = NULL;
        switch(firstParameter->dataType)
        {
        case INT_TYPE:
            //fprintf(g_codeGenOutputFp, "li $v0, 1\n");
            codeGenPrepareRegister(INT_REG, firstParameter->registerIndex,0, 1, 0, &parameterRegName);
            fprintf(g_codeGenOutputFp, "mov w0, %s\n", parameterRegName);
            fprintf(g_codeGenOutputFp, "bl _write_int\n");
            freeRegister(INT_REG, firstParameter->registerIndex);
            break;
        case FLOAT_TYPE:
            //fprintf(g_codeGenOutputFp, "li $v0, 2\n");
            codeGenPrepareRegister(FLOAT_REG, firstParameter->registerIndex,0, 1, 0, &parameterRegName);
            fprintf(g_codeGenOutputFp, "fmov s0, %s\n", parameterRegName);
            fprintf(g_codeGenOutputFp, "bl _write_float\n");
            freeRegister(FLOAT_REG, firstParameter->registerIndex);
            break;
        case CONST_STRING_TYPE:
            //fprintf(g_codeGenOutputFp, "li $v0, 4\n");
            codeGenPrepareRegister(INT_REG, firstParameter->registerIndex,1, 1, 0, &parameterRegName);
            fprintf(g_codeGenOutputFp, "mov x0, %s\n", parameterRegName);
            fprintf(g_codeGenOutputFp, "bl _write_str\n");
            freeRegister(INT_REG, firstParameter->registerIndex);
            break;
        default:
            printf("Unhandled case in void codeGenFunctionCall(AST_NODE* functionCallNode)\n");
            printf("firstParameter->registerIndex was not free\n");
            break;
        }
        //fprintf(g_codeGenOutputFp, "syscall\n");
        return;
    }


    if(strcmp(functionIdNode->semantic_value.identifierSemanticValue.identifierName, "read") == 0)
    {
        fprintf(g_codeGenOutputFp, "bl _read_int\n");
    }
    else if(strcmp(functionIdNode->semantic_value.identifierSemanticValue.identifierName, "fread") == 0)
    {
        fprintf(g_codeGenOutputFp, "bl _read_float\n");
    }
    else
    {
        if (strcmp(functionIdNode->semantic_value.identifierSemanticValue.identifierName, "main") != 0) {
            AST_NODE* traverseParameter = parameterList->child;
           
            if(traverseParameter) {
                codeGenStoreArgument(traverseParameter, functionIdNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.functionSignature->parameterList);
            }
            fprintf(g_codeGenOutputFp, "bl _start_%s\n", functionIdNode->semantic_value.identifierSemanticValue.identifierName);
            while(traverseParameter) {
                fprintf(g_codeGenOutputFp, "add sp, sp, 8\n");
                traverseParameter = traverseParameter->rightSibling;
            }
        } else {
            // main function
            // do nothing!
        }
    }




    if (functionIdNode->semantic_value.identifierSemanticValue.symbolTableEntry) {
        if(functionIdNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.functionSignature->returnType == INT_TYPE)
        {
            functionCallNode->registerIndex = getRegister(INT_REG);
            char* returnIntRegName = NULL;
            codeGenPrepareRegister(INT_REG, functionCallNode->registerIndex,0, 0, 0, &returnIntRegName);

            fprintf(g_codeGenOutputFp, "mov %s, w0\n", returnIntRegName);

            codeGenSaveToMemoryIfPsuedoRegister(INT_REG, functionCallNode->registerIndex, returnIntRegName);
        }
        else if(functionIdNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.functionSignature->returnType == FLOAT_TYPE)
        {
            functionCallNode->registerIndex = getRegister(FLOAT_REG);
            char* returnfloatRegName = NULL;
            codeGenPrepareRegister(FLOAT_REG, functionCallNode->registerIndex,0, 0, 0, &returnfloatRegName);

            fprintf(g_codeGenOutputFp, "fmov %s, s0\n", returnfloatRegName);

            codeGenSaveToMemoryIfPsuedoRegister(INT_REG, functionCallNode->registerIndex, returnfloatRegName);
        }
    }
}


int codeGenCalcArrayElemenetAddress(AST_NODE* idNode)
{
    AST_NODE* traverseDim = idNode->child;
    int* sizeInEachDimension = idNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension;
            
    codeGenExprRelatedNode(traverseDim);
    int linearIdxRegisterIndex = traverseDim->registerIndex;
    traverseDim = traverseDim->rightSibling;

    int dimIndex = 1;
    int dimRegIndex = getRegister(INT_REG);
    char *totalRegName, *oneDimRegName, *dimConstRegName;
    /*TODO multiple dimensions*/
    while(traverseDim)
    {
        codeGenPrepareRegister(INT_REG, linearIdxRegisterIndex,0 , 1, 0, &totalRegName);
        codeGenPrepareRegister(INT_REG, dimRegIndex,0 , 0, 1, &dimConstRegName);
        fprintf(g_codeGenOutputFp, "mov %s, #%d\n", dimConstRegName, sizeInEachDimension[dimIndex]);
        fprintf(g_codeGenOutputFp, "mul %s, %s, %s\n", totalRegName, totalRegName, dimConstRegName);
        codeGenExprRelatedNode(traverseDim);
        codeGenPrepareRegister(INT_REG, traverseDim->registerIndex,0 , 1, 1, &oneDimRegName);
        fprintf(g_codeGenOutputFp, "add %s, %s, %s\n", totalRegName, totalRegName, oneDimRegName);

        freeRegister(INT_REG, traverseDim->registerIndex);
        dimIndex++;
        traverseDim = traverseDim->rightSibling;
    }
    freeRegister(INT_REG, dimRegIndex);
    
    int shiftLeftTwoBits = 2;
    codeGen2Reg1ImmInstruction_64(INT_REG, "lsl", linearIdxRegisterIndex, linearIdxRegisterIndex, &shiftLeftTwoBits);
    
    char* linearOffsetRegName = NULL;
    if(!isGlobalVariable(idNode->semantic_value.identifierSemanticValue.symbolTableEntry))
    {
        int baseOffset = idNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->offsetInAR;
        if(baseOffset > 0) {
            int realBaseOffsetIndex = getRegister(INT_REG);
            char *realBaseOffsetRegName;
            codeGenPrepareRegister(INT_REG, realBaseOffsetIndex,1, 1, 0, &realBaseOffsetRegName);// need think for isAddr
            codeGenPrepareRegister(INT_REG, linearIdxRegisterIndex,1, 1, 1, &linearOffsetRegName);// need think for isAddr
            fprintf(g_codeGenOutputFp, "ldr %s, [x29, %d]\n", realBaseOffsetRegName, baseOffset);
            fprintf(g_codeGenOutputFp, "add %s, %s, %s\n", linearOffsetRegName, linearOffsetRegName, realBaseOffsetRegName);
        }
        else {
            codeGen2Reg1ImmInstruction_64(INT_REG, "add", linearIdxRegisterIndex, linearIdxRegisterIndex, &baseOffset);
            codeGenPrepareRegister(INT_REG, linearIdxRegisterIndex,1, 1, 0, &linearOffsetRegName);// need think for isAddr
            fprintf(g_codeGenOutputFp, "add %s, %s, x29\n", linearOffsetRegName, linearOffsetRegName);
        }
    }
    else
    {
        fprintf(g_codeGenOutputFp, "ldr %s, =_g_%s\n", addrWorkRegisterName[0], idNode->semantic_value.identifierSemanticValue.identifierName);
        codeGenPrepareRegister(INT_REG, linearIdxRegisterIndex,1, 1, 1, &linearOffsetRegName);// need think for isAddr
        fprintf(g_codeGenOutputFp, "add %s, %s, %s\n", linearOffsetRegName, linearOffsetRegName, addrWorkRegisterName[0]);
    }

    codeGenSaveToMemoryIfPsuedoRegister(INT_REG, linearIdxRegisterIndex, linearOffsetRegName);

    return linearIdxRegisterIndex;
}


void codeGenVariableReference(AST_NODE* idNode)
{
    SymbolAttribute *idAttribute = idNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute;
    if(idNode->semantic_value.identifierSemanticValue.kind == NORMAL_ID)
    {
        if(idNode->dataType == INT_TYPE)
        {
            idNode->registerIndex = getRegister(INT_REG);
            char* loadRegName = NULL;
            if(!isGlobalVariable(idNode->semantic_value.identifierSemanticValue.symbolTableEntry))
            {
                codeGenPrepareRegister(INT_REG, idNode->registerIndex,0, 0, 0, &loadRegName);
                fprintf(g_codeGenOutputFp, "ldr %s, [x29, #%d]\n", loadRegName, idAttribute->offsetInAR);
            }
            else
            {
                fprintf(g_codeGenOutputFp, "ldr %s, =_g_%s\n", addrWorkRegisterName[0], idNode->semantic_value.identifierSemanticValue.identifierName);
                codeGenPrepareRegister(INT_REG, idNode->registerIndex,0, 0, 1, &loadRegName);
                fprintf(g_codeGenOutputFp, "ldr %s, [%s, #0]\n", loadRegName, addrWorkRegisterName[0]);
            }
            codeGenSaveToMemoryIfPsuedoRegister(INT_REG, idNode->registerIndex, loadRegName);
        }
        else if(idNode->dataType == FLOAT_TYPE)
        {
            idNode->registerIndex = getRegister(FLOAT_REG);
            char* loadRegName = NULL;
            if(!isGlobalVariable(idNode->semantic_value.identifierSemanticValue.symbolTableEntry))
            {
                codeGenPrepareRegister(FLOAT_REG, idNode->registerIndex,0, 0, 0, &loadRegName);
                fprintf(g_codeGenOutputFp, "ldr %s, [x29, #%d]\n", loadRegName, idAttribute->offsetInAR);
            }
            else
            {
                fprintf(g_codeGenOutputFp, "ldr %s, =_g_%s\n", addrWorkRegisterName[0], idNode->semantic_value.identifierSemanticValue.identifierName);
                codeGenPrepareRegister(FLOAT_REG, idNode->registerIndex,0, 0, 0, &loadRegName);
                fprintf(g_codeGenOutputFp, "ldr %s, [%s, #0]\n", loadRegName, addrWorkRegisterName[0]);
            }
            codeGenSaveToMemoryIfPsuedoRegister(FLOAT_REG, idNode->registerIndex, loadRegName);
        }
    }
    
    else if(idNode->semantic_value.identifierSemanticValue.kind == ARRAY_ID)
    {
        if(idNode->dataType == INT_TYPE || idNode->dataType == FLOAT_TYPE)
        {
            int elementAddressRegIndex = codeGenCalcArrayElemenetAddress(idNode);
            char* elementAddressRegName = NULL;
            codeGenPrepareRegister(INT_REG, elementAddressRegIndex,1, 1, 0, &elementAddressRegName);// need think for isAddr
            
            if(idNode->dataType == INT_TYPE)
            {
                idNode->registerIndex = elementAddressRegIndex;
                fprintf(g_codeGenOutputFp, "ldr %s, [%s, #0]\n", elementAddressRegName, elementAddressRegName);
                codeGenSaveToMemoryIfPsuedoRegister(INT_REG, idNode->registerIndex, elementAddressRegName);
            }
            else if(idNode->dataType == FLOAT_TYPE)
            {
                idNode->registerIndex = getRegister(FLOAT_REG);
                char* dstRegName = NULL;
                codeGenPrepareRegister(FLOAT_REG, idNode->registerIndex,0, 0, 0, &dstRegName);// need think for isAddr
                
                char* elementAddressRegName = NULL;
                codeGenPrepareRegister(INT_REG, elementAddressRegIndex,1, 1, 0, &elementAddressRegName);// need think for isAddr
            
                fprintf(g_codeGenOutputFp, "ldr %s, [%s, #0]\n", dstRegName, elementAddressRegName);
                codeGenSaveToMemoryIfPsuedoRegister(FLOAT_REG, idNode->registerIndex, dstRegName);
            
                freeRegister(INT_REG, elementAddressRegIndex);
            }
        }
    }
}


void codeGenConstantReference(AST_NODE* constantNode)
{
    C_type cType = constantNode->semantic_value.const1->const_type;
    if(cType == INTEGERC)
    {
        int tmpInt = constantNode->semantic_value.const1->const_u.intval;
        int constantLabelNumber = codeGenConstantLabel(INTEGERC, &tmpInt);
        constantNode->registerIndex = getRegister(INT_REG);
        char* regName = NULL;
        codeGenPrepareRegister(INT_REG, constantNode->registerIndex,0 , 0, 0, &regName);
        fprintf(g_codeGenOutputFp, "ldr %s, _CONSTANT_%d\n", regName, constantLabelNumber);
        codeGenSaveToMemoryIfPsuedoRegister(INT_REG, constantNode->registerIndex, regName);
    }
    else if(cType == FLOATC)
    {
        float tmpFloat = constantNode->semantic_value.const1->const_u.fval;
        int constantLabelNumber = codeGenConstantLabel(FLOATC, &tmpFloat);
        constantNode->registerIndex = getRegister(FLOAT_REG);
        char* regName = NULL;
        codeGenPrepareRegister(FLOAT_REG, constantNode->registerIndex,0 , 0, 0, &regName);
        fprintf(g_codeGenOutputFp, "ldr %s, _CONSTANT_%d\n", regName, constantLabelNumber);
        codeGenSaveToMemoryIfPsuedoRegister(FLOAT_REG, constantNode->registerIndex, regName);
    }
    else if(cType == STRINGC)
    {
        char* tmpCharPtr = constantNode->semantic_value.const1->const_u.sc;
        char* tempCharPtr_with000 = modifyStringWith000(tmpCharPtr);
        //printf("test: the string is %s\n", tempCharPtr_with000);
        int constantLabelNumber = codeGenConstantLabel(STRINGC, tempCharPtr_with000);
        constantNode->registerIndex = getRegister(INT_REG);
        char* regName = NULL;
        codeGenPrepareRegister(INT_REG, constantNode->registerIndex,1 , 0, 0, &regName);
        fprintf(g_codeGenOutputFp, "ldr %s, =_CONSTANT_%d\n", regName, constantLabelNumber);
        codeGenSaveToMemoryIfPsuedoRegister(INT_REG, constantNode->registerIndex, regName);
    }
}


void codeGenExprRelatedNode(AST_NODE* exprRelatedNode)
{
    switch(exprRelatedNode->nodeType)
    {
    case EXPR_NODE:
        codeGenExprNode(exprRelatedNode);
        break;
    case STMT_NODE:
        codeGenFunctionCall(exprRelatedNode);
        break;
    case IDENTIFIER_NODE:
        codeGenVariableReference(exprRelatedNode);
        break;
    case CONST_VALUE_NODE:
        codeGenConstantReference(exprRelatedNode);
        break;
    default:
        printf("Unhandle case in void processExprRelatedNode(AST_NODE* exprRelatedNode)\n");
        exprRelatedNode->dataType = ERROR_TYPE;
        break;
    }
}


void codeGenAssignmentStmt(AST_NODE* assignmentStmtNode)
{
    AST_NODE* leftOp = assignmentStmtNode->child;
    AST_NODE* rightOp = leftOp->rightSibling;
    codeGenExprRelatedNode(rightOp);

    /* TODO type conversion */
    // for next phase
    if(leftOp->dataType == FLOAT_TYPE && rightOp->dataType == INT_TYPE)
    {
        rightOp->registerIndex = codeGenConvertFromIntToFloat(rightOp->registerIndex);
    }
    else if(leftOp->dataType == INT_TYPE && rightOp->dataType == FLOAT_TYPE)
    {
        rightOp->registerIndex = codeGenConvertFromFloatToInt(rightOp->registerIndex);
    }

    if(leftOp->semantic_value.identifierSemanticValue.kind == NORMAL_ID)
    {
        if(leftOp->dataType == INT_TYPE)
        {// for int
            char* rightOpRegName = NULL;
            codeGenPrepareRegister(INT_REG, rightOp->registerIndex,0, 1, 0, &rightOpRegName);
            if(!isGlobalVariable(leftOp->semantic_value.identifierSemanticValue.symbolTableEntry))
            {
                fprintf(g_codeGenOutputFp, "str %s, [x29, #%d]\n", rightOpRegName, leftOp->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->offsetInAR);
            }
            else
            {// modify by own
                fprintf(g_codeGenOutputFp, "ldr %s, =_g_%s\n", addrWorkRegisterName[0], leftOp->semantic_value.identifierSemanticValue.identifierName);
                fprintf(g_codeGenOutputFp, "str %s, [%s, #0]\n", rightOpRegName, addrWorkRegisterName[0]);
            }
            leftOp->registerIndex = rightOp->registerIndex;
        }
        else if(leftOp->dataType == FLOAT_TYPE)
        {
            char* rightOpRegName = NULL;
            codeGenPrepareRegister(FLOAT_REG, rightOp->registerIndex,0, 1, 0, &rightOpRegName);
            if(!isGlobalVariable(leftOp->semantic_value.identifierSemanticValue.symbolTableEntry))
            {
                fprintf(g_codeGenOutputFp, "str %s, [x29, #%d]\n", rightOpRegName, leftOp->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->offsetInAR);
            }
            else
            {
                printf(g_codeGenOutputFp, "ldr %s, =_g_%s\n", addrWorkRegisterName[0], leftOp->semantic_value.identifierSemanticValue.identifierName);
                fprintf(g_codeGenOutputFp, "str %s, [%s, #0]\n", rightOpRegName, addrWorkRegisterName[0]);
            }
            leftOp->registerIndex = rightOp->registerIndex;
        }
    }
    else if(leftOp->semantic_value.identifierSemanticValue.kind == ARRAY_ID)
    {
        int elementAddressRegIndex = codeGenCalcArrayElemenetAddress(leftOp);

        char* elementAddressRegName = NULL;
        codeGenPrepareRegister(INT_REG, elementAddressRegIndex,1, 1, 0, &elementAddressRegName);// need think for isAddr
        if(leftOp->dataType == INT_TYPE)
        {
            char* rightOpRegName = NULL;
            codeGenPrepareRegister(INT_REG, rightOp->registerIndex,0, 1, 1, &rightOpRegName);// need think for isAddr
            fprintf(g_codeGenOutputFp, "str %s, [%s, #0]\n", rightOpRegName, elementAddressRegName);
            
            leftOp->registerIndex = rightOp->registerIndex;
        }
        else if(leftOp->dataType == FLOAT_TYPE)
        {
            char* rightOpRegName = NULL;
            codeGenPrepareRegister(FLOAT_REG, rightOp->registerIndex,0, 1, 0, &rightOpRegName);// need think for isAddr
            
            fprintf(g_codeGenOutputFp, "str %s, [%s, #0]\n", rightOpRegName, elementAddressRegName);

            leftOp->registerIndex = rightOp->registerIndex;
        }

        freeRegister(INT_REG, elementAddressRegIndex);
    }
}


void codeGenAssignOrExpr(AST_NODE* testNode)
{
    if(testNode->nodeType == STMT_NODE)
    {
        if(testNode->semantic_value.stmtSemanticValue.kind == ASSIGN_STMT)
        {
            codeGenAssignmentStmt(testNode);
        }
        else if(testNode->semantic_value.stmtSemanticValue.kind == FUNCTION_CALL_STMT)
        {
            codeGenFunctionCall(testNode);
        }
    }
    else
    {
        codeGenExprRelatedNode(testNode);
    }
}


void codeGenWhileStmt(AST_NODE* whileStmtNode)
{
    AST_NODE* boolExpression = whileStmtNode->child;

//    int constantZeroLabelNumber = -1;
//    if(boolExpression->dataType == FLOAT_TYPE)
//    {
//        float zero = 0.0f;
//        constantZeroLabelNumber = codeGenConstantLabel(FLOATC, &zero);
//    }

    int labelNumber = getLabelNumber();
    fprintf(g_codeGenOutputFp, "_whileTestLabel_%d:\n", labelNumber);
    
    codeGenAssignOrExpr(boolExpression);

    if(boolExpression->dataType == INT_TYPE)
    {
        char* boolRegName = NULL;
        codeGenPrepareRegister(INT_REG, boolExpression->registerIndex,0, 1, 0, &boolRegName);
        fprintf(g_codeGenOutputFp, "cmp %s, #0\n", boolRegName);
        fprintf(g_codeGenOutputFp, "beq _whileExitLabel_%d\n", labelNumber);
        freeRegister(INT_REG, boolExpression->registerIndex);
    }
    else if(boolExpression->dataType == FLOAT_TYPE)
    {
        char* boolRegName = NULL;
        codeGenPrepareRegister(FLOAT_REG, boolExpression->registerIndex,0, 1, 1, &boolRegName);
        fprintf(g_codeGenOutputFp, "fcmp %s, #0.0\n", boolRegName);
        fprintf(g_codeGenOutputFp, "beq _whileExitLabel_%d\n", labelNumber);
        freeRegister(FLOAT_REG, boolExpression->registerIndex);
    }
    
    AST_NODE* bodyNode = boolExpression->rightSibling;
    codeGenStmtNode(bodyNode);

    fprintf(g_codeGenOutputFp, "b _whileTestLabel_%d\n", labelNumber);
    fprintf(g_codeGenOutputFp, "_whileExitLabel_%d:\n", labelNumber);
}


void codeGenForStmt(AST_NODE* forStmtNode)
{
    /*TODO*/
    AST_NODE* initialNode = forStmtNode->child;
    AST_NODE* conditionNode = initialNode->rightSibling;
    AST_NODE* incrementNode = conditionNode->rightSibling;
    AST_NODE* bodyNode = incrementNode->rightSibling;

    int constantZeroLabelNumber = -1;
//    if(conditionNode->dataType == FLOAT_TYPE)
//    {
//        float zero = 0.0f;
//        constantZeroLabelNumber = codeGenConstantLabel(FLOATC, &zero);
//    }

    codeGenGeneralNode(initialNode);
    if(initialNode->dataType == INT_TYPE)
        freeRegister(INT_REG, initialNode->registerIndex);
    else if(initialNode->dataType == FLOAT_TYPE)
        freeRegister(FLOAT_REG, initialNode->registerIndex);

    int labelNumber = getLabelNumber();
    fprintf(g_codeGenOutputFp, "_forTestLabel_%d:\n", labelNumber);

    codeGenGeneralNode(conditionNode);

    if(conditionNode->dataType == INT_TYPE)
    {
        char* boolRegName = NULL;
        codeGenPrepareRegister(INT_REG, conditionNode->registerIndex,0 , 1, 0, &boolRegName);
        fprintf(g_codeGenOutputFp, "cmp %s, #0\n", boolRegName);
        fprintf(g_codeGenOutputFp, "beq _forExitLabel_%d\n", labelNumber);
        freeRegister(INT_REG, conditionNode->registerIndex);
    }
    else if(conditionNode->dataType == FLOAT_TYPE)
    {
//        fprintf(g_codeGenOutputFp, "l.s %s, _CONSTANT_%d\n", floatWorkRegisterName[0], constantZeroLabelNumber);
        char* boolRegName = NULL;
        codeGenPrepareRegister(FLOAT_REG, conditionNode->registerIndex,0, 1, 1, &boolRegName);
        fprintf(g_codeGenOutputFp, "fcmp %s, #0.0\n", boolRegName);
        fprintf(g_codeGenOutputFp, "beq _forExitLabel_%d\n", labelNumber);
        freeRegister(FLOAT_REG, conditionNode->registerIndex);
    }

    fprintf(g_codeGenOutputFp, "b _forBodyLabel_%d\n", labelNumber);
    fprintf(g_codeGenOutputFp, "_forIncLabel_%d:\n", labelNumber);

    codeGenGeneralNode(incrementNode);
    if(incrementNode->dataType == INT_TYPE)
        freeRegister(INT_REG, incrementNode->registerIndex);
    else if(incrementNode->dataType == FLOAT_TYPE)
        freeRegister(FLOAT_REG, incrementNode->registerIndex);

    fprintf(g_codeGenOutputFp, "b _forTestLabel_%d\n", labelNumber);
    fprintf(g_codeGenOutputFp, "_forBodyLabel_%d:\n", labelNumber);

    codeGenStmtNode(bodyNode);

    fprintf(g_codeGenOutputFp, "b _forIncLabel_%d\n", labelNumber);
    fprintf(g_codeGenOutputFp, "_forExitLabel_%d:\n", labelNumber);
}


void codeGenIfStmt(AST_NODE* ifStmtNode)
{
    AST_NODE* boolExpression = ifStmtNode->child;

//    int constantZeroLabelNumber = -1;
//    if(boolExpression->dataType == FLOAT_TYPE)
//    {
//        float zero = 0.0f;
//        constantZeroLabelNumber = codeGenConstantLabel(FLOATC, &zero);
//    }

    // for recursive call
    int labelNumber = getLabelNumber();

    codeGenAssignOrExpr(boolExpression);

    if(boolExpression->dataType == INT_TYPE)
    {
        char* boolRegName = NULL;
        codeGenPrepareRegister(INT_REG, boolExpression->registerIndex,0, 1, 0, &boolRegName);
        fprintf(g_codeGenOutputFp, "cmp %s, #0\n", boolRegName);
        fprintf(g_codeGenOutputFp, "beq _elseLabel_%d\n", labelNumber);
        freeRegister(INT_REG, boolExpression->registerIndex);
    }
    else if(boolExpression->dataType == FLOAT_TYPE)
    {
        char* boolRegName = NULL;
        codeGenPrepareRegister(FLOAT_REG, boolExpression->registerIndex,0, 1, 1, &boolRegName);
        fprintf(g_codeGenOutputFp, "fcmp %s, #0.0\n", boolRegName);
        fprintf(g_codeGenOutputFp, "beq _elseLabel_%d\n", labelNumber);
        freeRegister(FLOAT_REG, boolExpression->registerIndex);
    }

    AST_NODE* ifBodyNode = boolExpression->rightSibling;
    codeGenStmtNode(ifBodyNode);
    
    fprintf(g_codeGenOutputFp, "b _ifExitLabel_%d\n", labelNumber);
    fprintf(g_codeGenOutputFp, "_elseLabel_%d:\n", labelNumber);
    AST_NODE* elsePartNode = ifBodyNode->rightSibling;
    codeGenStmtNode(elsePartNode);
    fprintf(g_codeGenOutputFp, "_ifExitLabel_%d:\n", labelNumber);
}


void codeGenReturnStmt(AST_NODE* returnStmtNode)
{
    AST_NODE* returnVal = returnStmtNode->child;
    if(returnVal->nodeType != NUL_NODE)
    {
        codeGenExprRelatedNode(returnVal);
        /* TODO type conversion */
        if(returnStmtNode->dataType == FLOAT_TYPE && returnVal->dataType == INT_TYPE)
        {
            returnVal->registerIndex = codeGenConvertFromIntToFloat(returnVal->registerIndex);
        }
        else if(returnStmtNode->dataType == INT_TYPE && returnVal->dataType == FLOAT_TYPE)
        {
            returnVal->registerIndex = codeGenConvertFromFloatToInt(returnVal->registerIndex);
        }

        char* returnValRegName = NULL;
        /*if (returnVal->dataType == INT_TYPE)*/
        if (returnStmtNode->dataType == INT_TYPE)
        {
            codeGenPrepareRegister(INT_REG, returnVal->registerIndex,0, 1, 0, &returnValRegName);// need think for isAddr
            fprintf(g_codeGenOutputFp, "mov w0, %s\n", returnValRegName);
            freeRegister(INT_REG, returnVal->registerIndex);
        }
        /*else if(returnVal->dataType == FLOAT_TYPE)*/
        else if(returnStmtNode->dataType == FLOAT_TYPE)
        {
            codeGenPrepareRegister(FLOAT_REG, returnVal->registerIndex,0, 1, 0, &returnValRegName);// need think for isAddr
            fprintf(g_codeGenOutputFp, "fmov s0, %s\n", returnValRegName);
            freeRegister(FLOAT_REG, returnVal->registerIndex);
        }
    }
    fprintf(g_codeGenOutputFp, "bl _end_%s\n", g_currentFunctionName);
}


void codeGenStmtNode(AST_NODE* stmtNode)
{
    //printSourceFile(g_codeGenOutputFp, stmtNode->linenumber);

    if(stmtNode->nodeType == NUL_NODE)
    {
        return;
    }
    else if(stmtNode->nodeType == BLOCK_NODE)
    {
        codeGenBlockNode(stmtNode);
    }
    else
    {
        switch(stmtNode->semantic_value.stmtSemanticValue.kind)
        {
        case WHILE_STMT:
            codeGenWhileStmt(stmtNode);
            break;
        case FOR_STMT:
            codeGenForStmt(stmtNode);
            break;
        case ASSIGN_STMT:
            codeGenAssignmentStmt(stmtNode);
            // remember to free
            if(stmtNode->child->dataType == INT_TYPE)
            {
                freeRegister(INT_REG, stmtNode->child->registerIndex);
            }
            // remember to free
            else if(stmtNode->child->dataType == FLOAT_TYPE)
            {
                freeRegister(FLOAT_REG, stmtNode->child->registerIndex);
            }
            break;
        case IF_STMT:
            codeGenIfStmt(stmtNode);
            break;
        case FUNCTION_CALL_STMT:
            codeGenFunctionCall(stmtNode);
            if(stmtNode->registerIndex != -1)
            {
                if(stmtNode->dataType == INT_TYPE)
                {
                    freeRegister(INT_REG, stmtNode->registerIndex);
                }
                else if(stmtNode->dataType == FLOAT_TYPE)
                {
                    freeRegister(FLOAT_REG, stmtNode->registerIndex);
                }
            }
            break;
        case RETURN_STMT:
            codeGenReturnStmt(stmtNode);
            break;
        default:
            printf("Unhandle case in void processStmtNode(AST_NODE* stmtNode)\n");
            break;
        }
    }
}


void codeGenGeneralNode(AST_NODE* node)
{
    AST_NODE *traverseChildren = node->child;
    AST_NODE *lastChild = NULL;
    switch(node->nodeType)
    {
    case VARIABLE_DECL_LIST_NODE:
        /*TODO initial*/
        while(traverseChildren)
        {
            AST_NODE *typeNode = traverseChildren->child;
            AST_NODE *idNode = typeNode->rightSibling;

            while(idNode) {
                if(idNode->semantic_value.identifierSemanticValue.kind == WITH_INIT_ID) {
                    AST_NODE *constValueNode = idNode->child;

                    if(typeNode->dataType == INT_TYPE) {
                        int constValueIndex = getRegister(INT_REG);
                        char *constValueRegName;
                        int constantLabelNumber = codeGenConstantLabel(INTEGERC, &constValueNode->semantic_value.exprSemanticValue.constEvalValue.iValue);

                        codeGenPrepareRegister(INT_REG, constValueIndex, 0, 0, 0, &constValueRegName);
                        fprintf(g_codeGenOutputFp, "ldr %s, _CONSTANT_%d\n", constValueRegName, constantLabelNumber);
                        fprintf(g_codeGenOutputFp, "str %s, [x29, #%d]\n", constValueRegName, idNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->offsetInAR);
                        freeRegister(INT_REG, constValueIndex);
                    }
                    else if(typeNode->dataType == FLOAT_TYPE) {
                        int constValueIndex = getRegister(FLOAT_REG);
                        char *constValueRegName;
                        int constantLabelNumber = codeGenConstantLabel(FLOATC, &constValueNode->semantic_value.exprSemanticValue.constEvalValue.fValue);

                        codeGenPrepareRegister(FLOAT_REG, constValueIndex,0, 0, 0, &constValueRegName);
                        fprintf(g_codeGenOutputFp, "ldr %s, _CONSTANT_%d\n", constValueRegName, constantLabelNumber);
                        fprintf(g_codeGenOutputFp, "str %s, [x29, #%d]\n", constValueRegName, idNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->offsetInAR);
                        freeRegister(FLOAT_REG, constValueIndex);
                    }
                }
                idNode = idNode->rightSibling;
            }
            traverseChildren = traverseChildren->rightSibling;
        }
        break;
    case STMT_LIST_NODE:
        while(traverseChildren)
        {
            codeGenStmtNode(traverseChildren);
            traverseChildren = traverseChildren->rightSibling;
        }
        break;
    case NONEMPTY_ASSIGN_EXPR_LIST_NODE:
        while(traverseChildren)
        {
            codeGenAssignOrExpr(traverseChildren);
            if(traverseChildren->rightSibling)
            {
                if(traverseChildren->dataType == INT_TYPE)
                {
                    freeRegister(INT_REG, traverseChildren->registerIndex);
                }
                else if(traverseChildren->dataType == FLOAT_TYPE)
                {
                    freeRegister(FLOAT_REG, traverseChildren->registerIndex);
                }
            }
            lastChild = traverseChildren;
            traverseChildren = traverseChildren->rightSibling;
        }
        if (lastChild) {
            node->registerIndex = lastChild->registerIndex;
        }
        break;
    case NONEMPTY_RELOP_EXPR_LIST_NODE:
        while(traverseChildren)
        {
            codeGenExprRelatedNode(traverseChildren);
            if(traverseChildren->rightSibling)
            {
                if(traverseChildren->dataType == INT_TYPE)
                {
                    freeRegister(INT_REG, traverseChildren->registerIndex);
                }
                else if(traverseChildren->dataType == FLOAT_TYPE)
                {
                    freeRegister(FLOAT_REG, traverseChildren->registerIndex);
                }
            }
            lastChild = traverseChildren;
            traverseChildren = traverseChildren->rightSibling;
        }
        if (lastChild) {
            node->registerIndex = lastChild->registerIndex;
        }
        break;
    case NUL_NODE:
        break;
    default:
        printf("Unhandle case in void processGeneralNode(AST_NODE *node)\n");
        node->dataType = ERROR_TYPE;
        break;
    }
}
