#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "header.h"


int main( int argc, char *argv[] )
{
    FILE *source, *target;
    Program program;
    SymbolTable symtab;

    if( argc == 3){
        source = fopen(argv[1], "r");
        target = fopen(argv[2], "w");
        if( !source ){
            printf("can't open the source file\n");
            exit(2);
        }
        else if( !target ){
            printf("can't open the target file\n");
            exit(2);
        }
        else{
            program = parser(source);
            fclose(source);
            symtab = build(program);
            check(&program, &symtab);
            gencode(program, target);
        }
    }
    else{
        printf("Usage: %s source_file target_file\n", argv[0]);
    }


    return 0;
}


/********************************************* 
  Scanning 
 *********************************************/
Token getNumericToken( FILE *source, char c )
{
    Token token;
    int i = 0;

    while( isdigit(c) ) {
        token.tok[i++] = c;
        c = fgetc(source);
    }

    if( c != '.' ){
        ungetc(c, source);
        token.tok[i] = '\0';
        token.type = IntValue;
        return token;
    }

    token.tok[i++] = '.';

    c = fgetc(source);
    if( !isdigit(c) ){
        ungetc(c, source);
        printf("Expect a digit : %c\n", c);
        exit(1);
    }

    while( isdigit(c) ){
        token.tok[i++] = c;
        c = fgetc(source);
    }

    ungetc(c, source);
    token.tok[i] = '\0';
    token.type = FloatValue;
    return token;
}
Token getIdentifer( FILE *source, char c){
    Token token;
    int i = 0;
    while (islower(c)) {
        if (c == 'p' || c == 'f' || c == 'i') {
            printf("Can't use character i, p or i as an idetifier");
            exit(1);
        }
        if (i > IDLENGTH) {
            printf("variable name exceeds 64 characters");
            exit(1);
        }
        token.tok[i++] = c;
        c = fgetc(source);
    }
    ungetc(c, source);
    token.tok[i] = '\0';
    token.type = Alphabet;
    return token;
}
void ungetIdentifier( FILE *source, char *tok ){
    int i = strlen(tok);
    char c;
    while (i >= 1) {
        c = tok[--i];
        ungetc(c, source);
    }
}
Token scanner( FILE *source )
{
    char c;
    Token token;

    while( !feof(source) ){
        c = fgetc(source);

        while( isspace(c) ) c = fgetc(source);

        if( isdigit(c) )
            return getNumericToken(source, c);

        if( islower(c) ){
            if (c == 'f') {
                token.type = FloatDeclaration;
                token.tok[0] = c;
                token.tok[1] = '\0';
                return token;
            }
            else if (c == 'i') {
                token.type = IntegerDeclaration;
                token.tok[0] = c;
                token.tok[1] = '\0';
                return token;
            }
            else if (c == 'p') {
                token.type = PrintOp;
                token.tok[0] = c;
                token.tok[1] = '\0';
                return token;
            }
            else {
                token = getIdentifer(source, c);
                return token;
            }
        }
        token.tok[0] = c;
        token.tok[1] = '\0';
        switch(c){
            case '=':
                token.type = AssignmentOp;
                return token;
            case '+':
                token.type = PlusOp;
                return token;
            case '-':
                token.type = MinusOp;
                return token;
            case '*':
                token.type = MulOp;
                return token;
            case '/':
                token.type = DivOp;
                return token;
            case EOF:
                token.type = EOFsymbol;
                token.tok[0] = '\0';
                return token;
            default:
                printf("Invalid character : %c\n", c);
                exit(1);
        }
    }
    //printf("here");
    token.tok[0] = '\0';
    token.type = EOFsymbol;
    return token;
}


/********************************************************
  Parsing
 *********************************************************/
Declaration parseDeclaration( FILE *source, Token token )
{
    Token token2;
     //printf("this is tk: %s\n", token.tok);
    switch(token.type){
        case FloatDeclaration:
        case IntegerDeclaration:
            token2 = scanner(source);
            //printf("this is tk: %s\n", token.tok);
            //printf("this is tk2: %s\n", token2.tok);
            if (strcmp(token2.tok, "f") == 0 ||
                    strcmp(token2.tok, "i") == 0 ||
                    strcmp(token2.tok, "p") == 0) {
                printf("Syntax Error: %s cannot be used as id\n", token2.tok);
                exit(1);
            }
            return makeDeclarationNode( token, token2 );
        default:
            printf("Syntax Error: Expect Declaration %s\n", token.tok);
            exit(1);
    }
}

Declarations *parseDeclarations( FILE *source )
{
    Token token = scanner(source);
    Declaration decl;
    Declarations *decls;
    switch(token.type){
        case FloatDeclaration:
        case IntegerDeclaration:
            decl = parseDeclaration(source, token);
            decls = parseDeclarations(source);
            return makeDeclarationTree( decl, decls );
        case PrintOp:
            ungetc(token.tok[0], source);
            return NULL;
        case Alphabet:
            ungetIdentifier(source, token.tok);
            return NULL;
        case EOFsymbol:
            return NULL;
        default:
            printf("Syntax Error: Expect declarations %s\n", token.tok);
            exit(1);
    }
}

Expression *parseValue( FILE *source )
{
    Token token = scanner(source);
    Expression *value = (Expression *)malloc( sizeof(Expression) );
    value->leftOperand = value->rightOperand = NULL;

    switch(token.type){
        case Alphabet:
            (value->v).type = Identifier;
            strcpy((value->v).val.id, token.tok);
            break;
        case IntValue:
            (value->v).type = IntConst;
            (value->v).val.ivalue = atoi(token.tok);
            printf("this is %d\n", (value->v).val.ivalue);
            break;
        case FloatValue:
            (value->v).type = FloatConst;
            (value->v).val.fvalue = atof(token.tok);
            break;
        default:
            printf("Syntax Error: Expect Identifier or a Number %s\n", token.tok);
            exit(1);
    }

    return value;
}

Expression *parseExpressionTail( FILE *source, Expression *lvalue )
{
    Token token = scanner(source);
    Expression *expr;

    switch(token.type){
        case PlusOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = PlusNode;
            (expr->v).val.op = Plus;
            expr->leftOperand = lvalue;
            expr->rightOperand = parseTerm(source);
            return parseExpressionTail(source, expr);
        case MinusOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = MinusNode;
            (expr->v).val.op = Minus;
            expr->leftOperand = lvalue;
            expr->rightOperand = parseTerm(source);
            return parseExpressionTail(source, expr);
        case Alphabet:
            ungetIdentifier(source, token.tok);
            return lvalue;
        case PrintOp:
            ungetc(token.tok[0], source);
            return lvalue;
        case EOFsymbol:
            return lvalue;
        default:
            printf("Syntax Error: Expect a numeric value or an identifier %s\n", token.tok);
            exit(1);
    }
}

Expression *parseExpression( FILE *source, Expression *lvalue )
{
    Token token = scanner(source);
    Expression *expr;

    switch(token.type){
        case PlusOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = PlusNode;
            (expr->v).val.op = Plus;
            expr->leftOperand = lvalue;
            expr->rightOperand = parseTerm(source);
            return parseExpressionTail(source, expr);
        case MinusOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = MinusNode;
            (expr->v).val.op = Minus;
            expr->leftOperand = lvalue;
            expr->rightOperand = parseTerm(source);
            return parseExpressionTail(source, expr);
        case Alphabet:
            ungetIdentifier(source, token.tok);
            return NULL;
        case PrintOp:
            ungetc(token.tok[0], source);
            return NULL;
        case EOFsymbol:
            return NULL;
        default:
            printf("Syntax Error: Expect a numeric value or an identifier %s\n", token.tok);
            exit(1);
    }
}

Expression *parseTermTail(FILE *source, Expression *lvalue){
    Expression *term;
    Token token = scanner(source);
    switch (token.type) {
        case MulOp:
            term = (Expression *)malloc(sizeof(Expression));
            (term->v).type = MulNode;
            (term->v).val.op = Mul;
            term->leftOperand = lvalue;
            term->rightOperand = parseValue(source);
            return parseTermTail(source, term);
            break;
        case DivOp:
            term = (Expression *)malloc(sizeof(Expression));
            (term->v).type = DivNode;
            (term->v).val.op = Div;
            term->leftOperand = lvalue;
            term->rightOperand = parseValue(source);
            return parseTermTail(source, term);
            break;
        case Alphabet:
            ungetIdentifier(source, token.tok);
            return lvalue;
        case PrintOp:
        case PlusOp:
        case MinusOp:
            ungetc(token.tok[0], source);
            return lvalue;
        case EOFsymbol:
            return lvalue;
        default:
            printf("Syntax Error: Expect a numeric value or an identifier %s\n", token.tok);
            exit(1);
    }
}

Expression *parseTerm(FILE *source){
    Expression *term;
    term = parseValue(source);
    return parseTermTail(source, term);
}

Statement parseStatement( FILE *source, Token token )
{
    Token next_token;
    Expression *value, *expr;

    switch(token.type){
        case Alphabet:
            next_token = scanner(source);
            if(next_token.type == AssignmentOp){
                value = parseTerm(source);
                expr = parseExpression(source, value);
                return makeAssignmentNode(token.tok, value, expr);
            }
            else{
                printf("Syntax Error: Expect an assignment op %s\n", next_token.tok);
                exit(1);
            }
        case PrintOp:
            next_token = scanner(source);
            if(next_token.type == Alphabet)
                return makePrintNode(next_token.tok);
            else{
                printf("Syntax Error: Expect an identifier %s\n", next_token.tok);
                exit(1);
            }
            break;
        default:
            printf("Syntax Error: Expect a statement %s\n", token.tok);
            exit(1);
    }
}

Statements *parseStatements( FILE * source )
{

    Token token = scanner(source);
    Statement stmt;
    Statements *stmts;

    switch(token.type){
        case Alphabet:
        case PrintOp:
            stmt = parseStatement(source, token);
            stmts = parseStatements(source);
            return makeStatementTree(stmt , stmts);
        case EOFsymbol:
            return NULL;
        default:
            printf("Syntax Error: Expect statements %s\n", token.tok);
            exit(1);
    }
}


/*********************************************************************
  Build AST
 **********************************************************************/
Declaration makeDeclarationNode( Token declare_type, Token identifier )
{
    Declaration tree_node;
    //printf("1:%s %s\n", declare_type.tok, identifier.tok);
    switch(declare_type.type){
        case FloatDeclaration:
            tree_node.type = Float;
            break;
        case IntegerDeclaration:
            tree_node.type = Int;
            break;
        default:
            break;
    }
    //printf("2:%s %s\n", declare_type.tok, identifier.tok);
    strcpy(tree_node.name,identifier.tok);

    return tree_node;
}

Declarations *makeDeclarationTree( Declaration decl, Declarations *decls )
{
    Declarations *new_tree = (Declarations *)malloc( sizeof(Declarations) );
    new_tree->first = decl;
    new_tree->rest = decls;

    return new_tree;
}


Statement makeAssignmentNode( char *id, Expression *v, Expression *expr_tail )
{
    Statement stmt;
    AssignmentStatement assign;

    stmt.type = Assignment;
    strcpy(assign.id, id);
    if(expr_tail == NULL)
        assign.expr = v;
    else
        assign.expr = expr_tail;
    assign.expr = fold_constant(assign.expr);// constant-folding
    stmt.stmt.assign = assign;

    return stmt;
}

Statement makePrintNode( char *id )
{
    Statement stmt;
    stmt.type = Print;
    strcpy(stmt.stmt.variable, id);

    return stmt;
}

Statements *makeStatementTree( Statement stmt, Statements *stmts )
{
    Statements *new_tree = (Statements *)malloc( sizeof(Statements) );
    new_tree->first = stmt;
    new_tree->rest = stmts;

    return new_tree;
}

/* parser */
Program parser( FILE *source )
{
    Program program;

    program.declarations = parseDeclarations(source);
    program.statements = parseStatements(source);

    return program;
}
/********************************************************
 Constant folding
 *********************************************************/
Expression *fold_constant(Expression *expr){
    if (expr == NULL || (expr->v).type == IntConst || (expr->v).type == FloatConst || (expr->v).type == Identifier) {
        return expr;
    }
    if ((expr->leftOperand->v).type == PlusNode || (expr->leftOperand->v).type == MinusNode || (expr->leftOperand->v).type == MulNode || (expr->leftOperand->v).type == DivNode) {
        expr->leftOperand = fold_constant(expr->leftOperand);
    }
    if ((expr->rightOperand->v).type == PlusNode || (expr->rightOperand->v).type == MinusNode || (expr->rightOperand->v).type == MulNode || (expr->rightOperand->v).type == DivNode) {
        expr->rightOperand = fold_constant(expr->rightOperand);
    }
    if ((expr->leftOperand->v).type == IntConst && (expr->rightOperand->v).type == IntConst) {
        return fold_intNode(expr->leftOperand, expr->rightOperand, (expr->v).type);
    }
    else if( (expr->leftOperand->v).type == FloatConst && (expr->rightOperand->v).type == FloatConst ){
        return fold_floatNode(expr->leftOperand, expr->rightOperand, (expr->v).type);
    }
    else if( (expr->leftOperand->v).type == IntConst && (expr->rightOperand->v).type == FloatConst){
        return fold_intfloatNode(expr->leftOperand, expr->rightOperand, (expr->v).type);
    }
    else if((expr->leftOperand->v).type == FloatConst && (expr->rightOperand->v).type == IntConst){
        return fold_floatintNode(expr->leftOperand, expr->rightOperand, (expr->v).type);
    }
    else
        return expr;
}

Expression *fold_intNode(Expression *left, Expression *right, ValueType t){
    Expression *temp = (Expression *)malloc(sizeof(Expression));
    (temp->v).type = IntConst;
    int val;
    int leftval = (left->v).val.ivalue;
    int rightval = (right->v).val.ivalue;
    temp->leftOperand = NULL;
    temp->rightOperand = NULL;
    switch (t) {
        case PlusNode:
            val = leftval + rightval;
            break;
        case MinusNode:
            val = leftval - rightval;
            break;
        case DivNode:
            val = leftval / rightval;
            break;
        case MulNode:
            val = leftval * rightval;
            break;
        default:
            printf("Error: illegal op");
            exit(3);
            break;
    }
    (temp->v).val.ivalue = val;
    return  temp;
}
Expression *fold_floatNode(Expression *left, Expression *right, ValueType t){
    Expression *temp = (Expression *)malloc(sizeof(Expression));
    (temp->v).type = FloatConst;
    float val;
    float leftval = (left->v).val.fvalue;
    float rightval = (right->v).val.fvalue;
    temp->leftOperand = NULL;
    temp->rightOperand = NULL;
    switch (t) {
        case PlusNode:
            val = leftval + rightval;
            break;
        case MinusNode:
            val = leftval - rightval;
            break;
        case DivNode:
            val = leftval / rightval;
            break;
        case MulNode:
            val = leftval * rightval;
            break;
        default:
            printf("Error: illegal op");
            exit(3);
            break;
    }
    (temp->v).val.fvalue = val;
    return  temp;
}
Expression *fold_intfloatNode(Expression *left, Expression *right, ValueType t){
    Expression *temp = (Expression *)malloc(sizeof(Expression));
    (temp->v).type = FloatConst;
    float val;
    int leftval = (left->v).val.ivalue;
    float rightval = (right->v).val.fvalue;
    temp->leftOperand = NULL;
    temp->rightOperand = NULL;
    switch (t) {
        case PlusNode:
            val = leftval + rightval;
            break;
        case MinusNode:
            val = leftval - rightval;
            break;
        case DivNode:
            val = leftval / rightval;
            break;
        case MulNode:
            val = leftval * rightval;
            break;
        default:
            printf("Error: illegal op");
            exit(3);
            break;
    }
    (temp->v).val.fvalue = val;
    return  temp;
}
Expression *fold_floatintNode(Expression *left, Expression *right, ValueType t){
    Expression *temp = (Expression *)malloc(sizeof(Expression));
    (temp->v).type = FloatConst;
    float val;
    int leftval = (left->v).val.fvalue;
    float rightval = (right->v).val.ivalue;
    temp->leftOperand = NULL;
    temp->rightOperand = NULL;
    switch (t) {
        case PlusNode:
            val = leftval + rightval;
            break;
        case MinusNode:
            val = leftval - rightval;
            break;
        case DivNode:
            val = leftval / rightval;
            break;
        case MulNode:
            val = leftval * rightval;
            break;
        default:
            printf("Error: illegal op");
            exit(3);
            break;
    }
    (temp->v).val.fvalue = val;
    return  temp;
}



/********************************************************
  Hash function
 *********************************************************/
static inline long int hashString(char * str)
{
    unsigned long hash = 5381;
    int c;
    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}
static inline char * copystring(char * value)
{
    char * copy = (char *)malloc(strlen(value)+1);
    if(!copy) {
        printf("Unable to allocate string value %s\n",value);
        abort();
    }
    strcpy(copy,value);
    return copy;
}

void add2table( SymbolTable *table, char *str, DataType t ){
    int index = (int)(hashString(str) % REGNUM);
    HashPair cur_pair = table->table[index];
    while (cur_pair.type != Notype) {
        if (strcmp(str, cur_pair.str) == 0) {
            printf("Error : id %s has been declared\n", str);//error
        }
        index = (index + 1) % REGNUM;
        cur_pair = table->table[index];
    }
    cur_pair.type = t;
    strcpy(cur_pair.str, str);
    table->table[index] = cur_pair;
}

DataType lookup_hashpair(SymbolTable *table, char *str){
    int index = (int)(hashString(str) % REGNUM);
    HashPair cur_pair = table->table[index];
    while (cur_pair.type != Notype) {
        if (strcmp(str, cur_pair.str) == 0) {
            return cur_pair.type;
        }
        index = (index + 1) % REGNUM;
        cur_pair = table->table[index];
    }
    printf("Error : identifier %s is not declared\n", str);//error
    return cur_pair.type;
}



/********************************************************
  Build symbol table
 *********************************************************/
void InitializeTable( SymbolTable *table )
{
    int i;

    for(i = 0 ; i < REGNUM; i++)
        table->table[i].type = Notype;
}

/*void add_table( SymbolTable *table, char *c, DataType t )
{
    int index = (int)(*c - 'a');

    if(table->table[index] != Notype)
        printf("Error : id %s has been declared\n", c);//error
    table->table[index] = t;
}*/

SymbolTable build( Program program )
{
    SymbolTable table;
    Declarations *decls = program.declarations;
    Declaration current;

    InitializeTable(&table);

    while(decls !=NULL){
        current = decls->first;
        add2table(&table, current.name, current.type);
        decls = decls->rest;
    }

    return table;
}


/********************************************************************
  Type checking
 *********************************************************************/

void convertType( Expression * old, DataType type )
{
    if(old->type == Float && type == Int){
        printf("error : can't convert float to integer\n");
        return;
    }
    if(old->type == Int && type == Float){
        Expression *tmp = (Expression *)malloc( sizeof(Expression) );
        if(old->v.type == Identifier)
            printf("convert to float %s \n",old->v.val.id);
        else
            printf("convert to float %d \n", old->v.val.ivalue);
        tmp->v = old->v;
        tmp->leftOperand = old->leftOperand;
        tmp->rightOperand = old->rightOperand;
        tmp->type = old->type;

        Value v;
        v.type = IntToFloatConvertNode;
        v.val.op = IntToFloatConvert;
        old->v = v;
        old->type = Int;
        old->leftOperand = tmp;
        old->rightOperand = NULL;
    }
}

DataType generalize( Expression *left, Expression *right )
{
    if(left->type == Float || right->type == Float){
        printf("generalize : float\n");
        return Float;
    }
    printf("generalize : int\n");
    return Int;
}

/*DataType lookup_table( SymbolTable *table, char *c )
{
    int id = *c-'a';
    if( table->table[id] != Int && table->table[id] != Float)
        printf("Error : identifier %s is not declared\n", c);//error
    return table->table[id];
}*/

void checkexpression( Expression * expr, SymbolTable * table )
{
    char *c;
    if(expr->leftOperand == NULL && expr->rightOperand == NULL){
        switch(expr->v.type){
            case Identifier:
                c = expr->v.val.id;
                printf("identifier : %s\n",c);
                expr->type = lookup_hashpair(table, c);
                break;
            case IntConst:
                printf("constant : int\n");
                expr->type = Int;
                break;
            case FloatConst:
                printf("constant : float\n");
                expr->type = Float;
                break;
                //case PlusNode: case MinusNode: case MulNode: case DivNode:
            default:
                break;
        }
    }
    else{
        Expression *left = expr->leftOperand;
        Expression *right = expr->rightOperand;

        checkexpression(left, table);
        checkexpression(right, table);

        DataType type = generalize(left, right);
        convertType(left, type);//left->type = type;//converto
        convertType(right, type);//right->type = type;//converto
        expr->type = type;
    }
}

void checkstmt( Statement *stmt, SymbolTable * table )
{
    if(stmt->type == Assignment){
        AssignmentStatement assign = stmt->stmt.assign;
        printf("assignment : %s \n",assign.id);
        checkexpression(assign.expr, table);
        stmt->stmt.assign.type = lookup_hashpair(table, assign.id);
        if (assign.expr->type == Float && stmt->stmt.assign.type == Int) {
            printf("error : can't convert float to integer\n");
        } else {
            convertType(assign.expr, stmt->stmt.assign.type);
        }
    }
    else if (stmt->type == Print){
        printf("print : %s \n",stmt->stmt.variable);
        lookup_hashpair(table, stmt->stmt.variable);
    }
    else printf("error : statement error\n");//error
}

void check( Program *program, SymbolTable * table )
{
    Statements *stmts = program->statements;
    while(stmts != NULL){
        checkstmt(&stmts->first,table);
        stmts = stmts->rest;
    }
}


/***********************************************************************
  Code generation
 ************************************************************************/
void fprint_op( FILE *target, ValueType op )
{
    switch(op){
        case MinusNode:
            fprintf(target,"-\n");
            break;
        case PlusNode:
            fprintf(target,"+\n");
            break;
        default:
            fprintf(target,"Error in fprintf_op ValueType = %d\n",op);
            break;
    }
}

void fprint_expr( FILE *target, Expression *expr)
{
    if(expr->leftOperand == NULL){
        switch( (expr->v).type ){
            case Identifier:
                fprintf(target,"l%s\n",(expr->v).val.id);
                break;
            case IntConst:
                fprintf(target,"%d\n",(expr->v).val.ivalue);
                printf("ivalue: %d\n", (expr->v).val.ivalue);
                break;
            case FloatConst:
                fprintf(target,"%f\n", (expr->v).val.fvalue);
                break;
            default:
                fprintf(target,"Error In fprint_left_expr. (expr->v).type=%d\n",(expr->v).type);
                break;
        }
    }
    else{
        fprint_expr(target, expr->leftOperand);
        if(expr->rightOperand == NULL){
            fprintf(target,"5k\n");
        }
        else{
            //	fprint_right_expr(expr->rightOperand);
            fprint_expr(target, expr->rightOperand);
            fprint_op(target, (expr->v).type);
        }
    }
}

void gencode(Program prog, FILE * target)
{
    Statements *stmts = prog.statements;
    Statement stmt;

    while(stmts != NULL){
        stmt = stmts->first;
        switch(stmt.type){
            case Print:
                fprintf(target,"l%s\n",stmt.stmt.variable);
                fprintf(target,"p\n");
                break;
            case Assignment:
                fprint_expr(target, stmt.stmt.assign.expr);
                /*
                   if(stmt.stmt.assign.type == Int){
                   fprintf(target,"0 k\n");
                   }
                   else if(stmt.stmt.assign.type == Float){
                   fprintf(target,"5 k\n");
                   }*/
                fprintf(target,"s%s\n",stmt.stmt.assign.id);
                fprintf(target,"0 k\n");
                break;
        }
        stmts=stmts->rest;
    }

}


/***************************************
  For our debug,
  you can omit them.
 ****************************************/
void print_expr(Expression *expr)
{
    if(expr == NULL)
        return;
    else{
        print_expr(expr->leftOperand);
        switch((expr->v).type){
            case Identifier:
                printf("%s ", (expr->v).val.id);
                break;
            case IntConst:
                printf("%d ", (expr->v).val.ivalue);
                break;
            case FloatConst:
                printf("%f ", (expr->v).val.fvalue);
                break;
            case PlusNode:
                printf("+ ");
                break;
            case MinusNode:
                printf("- ");
                break;
            case MulNode:
                printf("* ");
                break;
            case DivNode:
                printf("/ ");
                break;
            case IntToFloatConvertNode:
                printf("(float) ");
                break;
            default:
                printf("error ");
                break;
        }
        print_expr(expr->rightOperand);
    }
}

void test_parser( FILE *source )
{
    Declarations *decls;
    Statements *stmts;
    Declaration decl;
    Statement stmt;
    Program program = parser(source);

    decls = program.declarations;

    while(decls != NULL){
        decl = decls->first;
        if(decl.type == Int)
            printf("i ");
        if(decl.type == Float)
            printf("f ");
        printf("%s ",decl.name);
        decls = decls->rest;
    }

    stmts = program.statements;

    while(stmts != NULL){
        stmt = stmts->first;
        if(stmt.type == Print){
            printf("p %s ", stmt.stmt.variable);
        }

        if(stmt.type == Assignment){
            printf("%s = ", stmt.stmt.assign.id);
            print_expr(stmt.stmt.assign.expr);
        }
        stmts = stmts->rest;
    }

}
