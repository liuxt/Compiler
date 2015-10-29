/* Parser template 2015 
===== Definition Section ===== */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "header.h"
int linenumber = 1;
AST_NODE *prog;

extern int g_anyErrorOccur;

static inline AST_NODE* makeSibling(AST_NODE *a, AST_NODE *b)
{ 
    while (a->rightSibling) {
        a = a->rightSibling;
    }
    if (b == NULL) {
        return a;
    }
    b = b->leftmostSibling;
    a->rightSibling = b;
    
    b->leftmostSibling = a->leftmostSibling;
    b->parent = a->parent;
    while (b->rightSibling) {
        b = b->rightSibling;
        b->leftmostSibling = a->leftmostSibling;
        b->parent = a->parent;
    }
    return b;
}

static inline AST_NODE* makeChild(AST_NODE *parent, AST_NODE *child)
{
    if (child == NULL) {
        return parent;
    }
    if (parent->child) {
        makeSibling(parent->child, child);
    } else {
        child = child->leftmostSibling;
        parent->child = child;
        while (child) {
            child->parent = parent;
            child = child->rightSibling;
        }
    }
    return parent;
}

static AST_NODE* makeFamily(AST_NODE *parent, int childrenCount, ...)
{
    va_list childrenList;
    va_start(childrenList, childrenCount);
    AST_NODE* child = va_arg(childrenList, AST_NODE*);
    makeChild(parent, child);
    AST_NODE* tmp = child;
    int index = 1;
    for (index = 1; index < childrenCount; ++index) {
        child = va_arg(childrenList, AST_NODE*);
        tmp = makeSibling(tmp, child);
    }
    va_end(childrenList);
    return parent;
}

static inline AST_NODE* makeIDNode(char *lexeme, IDENTIFIER_KIND idKind)
{
    AST_NODE* identifier = Allocate(IDENTIFIER_NODE);
    identifier->semantic_value.identifierSemanticValue.identifierName = lexeme;
    identifier->semantic_value.identifierSemanticValue.kind = idKind;
    identifier->semantic_value.identifierSemanticValue.symbolTableEntry = NULL;
    return identifier;                        
}

static inline AST_NODE* makeStmtNode(STMT_KIND stmtKind)
{
    AST_NODE* stmtNode = Allocate(STMT_NODE);
    stmtNode->semantic_value.stmtSemanticValue.kind = stmtKind;
    return stmtNode;                        
}

static inline AST_NODE* makeDeclNode(DECL_KIND declKind)
{
    AST_NODE* declNode = Allocate(DECLARATION_NODE);
    declNode->semantic_value.declSemanticValue.kind = declKind;
    return declNode;                        
}

static inline AST_NODE* makeExprNode(EXPR_KIND exprKind, int operationEnumValue)
{
    AST_NODE* exprNode = Allocate(EXPR_NODE);
    exprNode->semantic_value.exprSemanticValue.isConstEval = 0;
    exprNode->semantic_value.exprSemanticValue.kind = exprKind;
    if (exprKind == BINARY_OPERATION) {
        exprNode->semantic_value.exprSemanticValue.op.binaryOp = operationEnumValue;
    } else if (exprKind == UNARY_OPERATION) {
        exprNode->semantic_value.exprSemanticValue.op.unaryOp = operationEnumValue;
    } else {
        printf("Error in static inline AST_NODE* makeExprNode(EXPR_KIND exprKind, int operationEnumValue)\n");
    }
    return exprNode;                        
}

%}



%union{
	char *lexeme;
	CON_Type  *const1;
	AST_NODE  *node;
};

%token <lexeme>ID
%token <const1>CONST
%token VOID    
%token INT     
%token FLOAT   
%token IF      
%token ELSE    
%token WHILE   
%token FOR 
%token TYPEDEF 
%token OP_ASSIGN  
%token OP_OR   
%token OP_AND  
%token OP_NOT  
%token OP_EQ   
%token OP_NE   
%token OP_GT   
%token OP_LT   
%token OP_GE   
%token OP_LE   
%token OP_PLUS 
%token OP_MINUS        
%token OP_TIMES        
%token OP_DIVIDE       
%token MK_LB 
%token MK_RB 
%token MK_LPAREN       
%token MK_RPAREN       
%token MK_LBRACE       
%token MK_RBRACE       
%token MK_COMMA        
%token MK_SEMICOLON    
%token MK_DOT  
%token ERROR
%token RETURN

%type <node> program global_decl_list global_decl function_decl block stmt_list decl_list decl var_decl type init_id_list init_id  stmt relop_expr relop_term relop_factor expr term factor var_ref
%type <node> param_list param dim_fn expr_null id_list dim_decl cexpr mcexpr cfactor assign_expr_list test assign_expr rel_op relop_expr_list nonempty_relop_expr_list
%type <node> add_op mul_op dim_list type_decl nonempty_assign_expr_list


%start program

%%

/* ==== Grammar Section ==== */

/* Productions */               /* Semantic actions */
program		: global_decl_list { $$=Allocate(PROGRAM_NODE);  makeChild($$,$1); prog=$$;}
		| { $$=Allocate(PROGRAM_NODE); prog=$$;}
		;

global_decl_list: global_decl_list global_decl 
                    {
                        $$ = makeSibling($1, $2);
                    }	
                | global_decl
                    {
                        $$ = $1;
                    }
                ; 

global_decl	: decl_list function_decl
                {
                    $$ = makeSibling(makeChild(Allocate(VARIABLE_DECL_LIST_NODE), $1), $2);
                }
            | function_decl 
                {
                    $$ = $1;
                }
            ;

function_decl	: type ID MK_LPAREN param_list MK_RPAREN MK_LBRACE block MK_RBRACE     
                    {
                        $$ = makeDeclNode(FUNCTION_DECL);
                        AST_NODE* parameterList = Allocate(PARAM_LIST_NODE);
                        makeChild(parameterList, $4);
                        makeFamily($$, 4, $1, makeIDNode($2, NORMAL_ID), parameterList, $7);
                    }
                | VOID ID MK_LPAREN param_list MK_RPAREN MK_LBRACE block MK_RBRACE      
                    {
                        /*TODO*/
                    }
                | type ID MK_LPAREN  MK_RPAREN MK_LBRACE block MK_RBRACE 
                    {
                        $$ = makeDeclNode(FUNCTION_DECL);
                        AST_NODE* emptyParameterList = Allocate(PARAM_LIST_NODE);
                        makeFamily($$, 4, $1, makeIDNode($2, NORMAL_ID), emptyParameterList, $6);
                    }
                | VOID ID MK_LPAREN  MK_RPAREN MK_LBRACE block MK_RBRACE 
                    {
                        /*TODO*/
                    } 
                ;

param_list	: param_list MK_COMMA  param 
                {
                    $$ = makeSibling($1, $3);
                }
            | param	
                {
                    /*TODO*/
                }
            ;

param		: type ID 
                {
                    $$ = makeDeclNode(FUNCTION_PARAMETER_DECL);
                    makeFamily($$, 2, $1, makeIDNode($2, NORMAL_ID));
                }
            | type ID dim_fn 
                {
                    /*TODO*/
                }
            ;
dim_fn		: MK_LB expr_null MK_RB 
                {
                    $$ = $2;
                }
            | dim_fn MK_LB expr MK_RB
                {
                    $$ = makeSibling($1, $3);
                }
		;

expr_null	:expr 
                {
                    /*TODO*/
                }
            |
                {
                    $$ = Allocate(NUL_NODE); 
                }
            ;

block           : decl_list stmt_list 
                    {
                        /*TODO*/
                    }
                | stmt_list  
                    {
                        $$ = Allocate(BLOCK_NODE);
                        makeChild($$, makeChild(Allocate(STMT_LIST_NODE), $1));
                    }
                | decl_list 
                    {
                        $$ = Allocate(BLOCK_NODE);
                        makeChild($$, makeChild(Allocate(VARIABLE_DECL_LIST_NODE), $1));
                    }
                |   {
                        /*TODO*/
                    }
                ;
 
decl_list	: decl_list decl 
                {
                        /*TODO*/
                }
            | decl 
                {
                        /*TODO*/
                }
            ;

decl		: type_decl 
                {
                    $$ = $1;
                }
            | var_decl 
                {
                    $$ = $1;
                }
            ;

type_decl 	: TYPEDEF type id_list MK_SEMICOLON  
                {
                    /*TODO*/
                }
            | TYPEDEF VOID id_list MK_SEMICOLON 
                {
                    /*TODO*/
                }
            ;

var_decl	: type init_id_list MK_SEMICOLON 
                {
                    /*TODO*/
                }
            | ID id_list MK_SEMICOLON
                {
                    /*TODO*/
                }
            ;

type		: INT 
                {
                    $$ = makeIDNode("int", NORMAL_ID);  
                }
            | FLOAT 
                {
                    $$ = makeIDNode("float", NORMAL_ID);
                }
            ;

id_list		: ID 
                {
                    $$ = makeIDNode($1, NORMAL_ID);
                }
            | id_list MK_COMMA ID 
                {
                    /*TODO*/
                }
            | id_list MK_COMMA ID dim_decl
                {
                    /*TODO*/
                }
            | ID dim_decl
                {
                    /*TODO*/
                }
		;
dim_decl	: MK_LB cexpr MK_RB 
                {
                    /*TODO*/
                } 
            /*TODO: Try if you can define a recursive production rule
            | .......
            */
            ;
cexpr		: cexpr OP_PLUS mcexpr 
                {
                    $$ = makeExprNode(BINARY_OPERATION, BINARY_OP_ADD);
                    makeFamily($$, 2, $1, $3);
                } /* This is for array declarations */ 
            | cexpr OP_MINUS mcexpr
                {
                    /*TODO*/
                } 
            | mcexpr 
                {
                    /*TODO*/
                }
            ;  
mcexpr		: mcexpr OP_TIMES cfactor 
                {
                    /*TODO*/
                }
            | mcexpr OP_DIVIDE cfactor 
                {
                    /*TODO*/
                }
            | cfactor 
                {
                    /*TODO*/
                }
            ;
        
cfactor:	CONST 
                {
                    /*TODO*/
                }
            | MK_LPAREN cexpr MK_RPAREN 
                {
                    /*TODO*/
                }
            ;

init_id_list	: init_id 
                    {
                        /*TODO*/
                    }
                | init_id_list MK_COMMA init_id 
                    {
                        /*TODO*/
                    }
                ;

init_id		: ID 
                {
                    $$ = makeIDNode($1, NORMAL_ID);
                }
            | ID dim_decl 
                {
                    /*TODO*/
                }
            | ID OP_ASSIGN relop_expr 
                {
                    /*TODO*/
                }
            ;

stmt_list	: stmt_list stmt 
                {
                    /*TODO*/
                }
            | stmt
                {
                    /*TODO*/
                }
            ;



stmt		: MK_LBRACE block MK_RBRACE 
                {
                    /*TODO*/
                }
            /*TODO: | While Statement */
            | FOR MK_LPAREN assign_expr_list MK_SEMICOLON relop_expr_list MK_SEMICOLON assign_expr_list MK_RPAREN stmt
                {
                    /*TODO*/
                }
            | var_ref OP_ASSIGN relop_expr MK_SEMICOLON
                {
                    /*TODO*/
                }
            /*TODO: | If Statement */
            /*TODO: | If then else */
            /*TODO: | function call */
            | MK_SEMICOLON 
                {
                    /*TODO*/
                }
            | RETURN MK_SEMICOLON  
                {
                    /*TODO*/
                }
            | RETURN relop_expr MK_SEMICOLON
                {
                    /*TODO*/
                }
            ;

assign_expr_list : nonempty_assign_expr_list 
                     {
                        /*TODO*/
                     }
                 |  
                     {
                         $$ = Allocate(NUL_NODE); 
                     }
                 ;

nonempty_assign_expr_list        : nonempty_assign_expr_list MK_COMMA assign_expr 
                                    {
                                        /*TODO*/
                                    }
                                 | assign_expr
                                    {
                                        /*TODO*/
                                    }
                                 ;

test		: assign_expr
                {
                    $$ = $1;
                }
            ;

assign_expr     : ID OP_ASSIGN relop_expr 
                    {
                        /*TODO*/
                    }
                | relop_expr
                    {
                        /*TODO*/
                    }
		;

relop_expr	: relop_term 
                {
                    $$ = $1;
                }
            | relop_expr OP_OR relop_term
                {
                    $$ = makeExprNode(BINARY_OPERATION, BINARY_OP_OR);
                    makeFamily($$, 2, $1, $3);
                }
            ;

relop_term	: relop_factor 
                {
                    /*TODO*/
                }
            | relop_term OP_AND relop_factor
                {
                    /*TODO*/
                }
            ;

relop_factor	: expr
                    {
                        /*TODO*/
                    }
                | expr rel_op expr 
                    {
                        /*TODO*/
                    }
                ;

rel_op		: OP_EQ
                {
                    /*TODO*/
                }
            | OP_GE 
                {
                    /*TODO*/
                }
            | OP_LE 
                {
                    /*TODO*/
                }
            | OP_NE 
                {
                    /*TODO*/
                }
            | OP_GT 
                {
                    /*TODO*/
                }
            | OP_LT 
                {
                    /*TODO*/
                }
            ;


relop_expr_list	: nonempty_relop_expr_list 
                    {
                        /*TODO*/
                    }
                | 
                    {
                        $$ = Allocate(NUL_NODE);
                    }
                ;

nonempty_relop_expr_list	: nonempty_relop_expr_list MK_COMMA relop_expr
                                {
                                    /*TODO*/
                                }
                            | relop_expr 
                                {
                                    /*TODO*/
                                }
                            ;

expr		: expr add_op term 
                {
                    /*TODO*/
                }
            | term 
                {
                    /*TODO*/
                }
            ;

add_op		: OP_PLUS
                {
                    $$ = makeExprNode(BINARY_OPERATION, BINARY_OP_ADD);
                }
            | OP_MINUS 
                {
                    $$ = makeExprNode(BINARY_OPERATION, BINARY_OP_SUB);
                }
            ;

term		: term mul_op factor
                {
                    /*TODO*/
                }
            | factor
                {
                    /*TODO*/
                }
            ;

mul_op		: OP_TIMES
                {
                    /*TODO*/
                }
            | OP_DIVIDE 
                {
                    /*TODO*/
                }
            ;

factor		: MK_LPAREN relop_expr MK_RPAREN
                {
                    /*TODO*/
                }
            /*TODO: | -(<relop_expr>) e.g. -(4) */
            | OP_NOT MK_LPAREN relop_expr MK_RPAREN
                {   
                    /*TODO*/
                }
            | CONST 
                {
                    $$ = Allocate(CONST_VALUE_NODE);
                    $$->semantic_value.const1=$1;
                }
            /*TODO: | -<constant> e.g. -4 */
            | OP_NOT CONST
                {
                    /*TODO*/
                }
            | ID MK_LPAREN relop_expr_list MK_RPAREN 
                {
                    /*TODO*/
                }
            /*TODO: | -<function call> e.g. -f(4) */
            | OP_NOT ID MK_LPAREN relop_expr_list MK_RPAREN
                {
                    /*TODO*/
                }
            | var_ref 
                {
                    /*TODO*/
                }
            /*TODO: | -<var_ref> e.g. -var */
            | OP_NOT var_ref 
                {
                    /*TODO*/
                }
            ;

var_ref		: ID 
                {
                    /*TODO*/
                }
            | ID dim_list 
                {
                    /*TODO*/
                }
            ;


dim_list	: dim_list MK_LB expr MK_RB 
                {
                    /*TODO*/
                }
            | MK_LB expr MK_RB
                {
                    /*TODO*/
                }
		;


%%

#include "lex.yy.c"
main (argc, argv)
int argc;
char *argv[];
  {
     yyin = fopen(argv[1],"r");
     yyparse();
	 printf("%s\n", "Parsing completed. No errors found.");
	 printGV(prog, NULL);
  } /* main */


int yyerror (mesg)
char *mesg;
  {
  printf("%s\t%d\t%s\t%s\n", "Error found in Line ", linenumber, "next token: ", yytext );
  exit(1);
  }
