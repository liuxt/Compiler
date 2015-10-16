%option noyywrap 
%{
#include <stdio.h>
#include "header.h" 
int linenumber;
symtab * lookup();
symtab * ptr;
void     insertID();
void 	 printSymTab();
/* You need to define for all tokens in C--, here are some examples */
#define INT		10
#define FLOAT 		11 
#define OP_ASSIGN 	12 
#define OP_OR 		13 
#define MK_LPAREN 	14 
#define MK_RPAREN 	15 
#define MK_LBRACE   16
#define MK_RBRACE   17
#define MK_LBRACKET 18
#define MK_RIGHTBRACKET     19
#define MK_SEMICOLON    20
#define MK_COMMA    21
#define MK_DOT  22
#define MK_NEWLINE  23

#define OP_AND  24
#define OP_NOT  25
#define OP_LT   26
#define OP_GT   27
#define OP_GE   28
#define OP_LE   29
#define OP_NE   30
#define OP_EQ   31
#define OP_PLUS  32
#define OP_MINUS    33
#define OP_MULTIPLY 34
#define OP_DIVIDE   35

#define RETURN  36
#define TYPEDEF 37
#define IF      38
#define ELSE    39
#define FOR     40
#define VOID    41
#define WHILE   42

#define INT_CONSTANT 43
#define FLOAT_CONSTANT 44
#define COMMENT 45
#define ERROR 		100 


%}

letter   [A-Za-z]
digit    [0-9]
ID	 {letter}({letter}|{digit}|"_")*
WS	 [ \t]+
Int_constant {digit}+
exp     [eE][+-]?{digit}+
float1  {digit}+{exp}
float2  ({digit}+"."{digit}*|"."{digit}+){exp}?

RWReturn    "return"
RWTypedef   "typedef"
RWIf    "if"
RWElse  "else"
RWInt   "int"
RWFloat     "float"
RWFor   "for"
RWVoid  "void"
RWWhile     "while"

Float_constant  [+-]?({float1}|{float2})
String_constant \"(\\.|[^\\"\n])*\"
comment    "/*"(([^*])|([*]+[^/*]))*("*")+"/"

OP_assign        "="
OP_or   "||"
OP_and  "&&"
OP_not  "!" 
OP_lt   "<"
OP_gt   ">"
OP_ge   ">="
OP_le   "<="
OP_ne   "!="
OP_eq   "=="
OP_plus "+"
OP_minus    "-"
OP_multiply     "*"
OP_divide   "/"
/* Other operators appear here */ 


newline  "\n"

DL_lparen "("
DL_rparen ")"
DL_lbrace "{"
DL_rbrace "}"
DL_lbracket "["
DL_rbracket "]"
DL_comma  ","
DL_semicol ";"
DL_dot  "."

/* Other separators appear here */


error    .

%%

{WS}            {/*printf("this is white space\n");*/}
{Float_constant} {printf("float: %s\n", yytext);}
{Int_constant}  {/*return INT_CONSTANT;*/}
{String_constant}   {printf("string: %s\n", yytext);}
{comment}       {printf("comment: %s\n", yytext);}

{RWInt}         { /*printf("%s\n", yytext);return INT;*/  }
{RWFloat}       { /*printf("%s\n", yytext);return FLOAT;*/ }
{RWReturn}      {/*return RETURN;*/}
{RWTypedef}     {/*return TYPEDEF;*/}
{RWIf}          {/*return IF;*/}
{RWElse}        {/*return ELSE;*/}
{RWFor}         {/*return FOR;*/}
{RWVoid}        {/*return VOID;*/}
{RWWhile}       {/*return WHILE;*/}

{ID}            { 
			ptr = lookup(yytext);
			if (ptr == NULL)
			     insertID(yytext);	
			else 
			     ptr->counter++;
		}

{OP_assign}     { /*return OP_ASSIGN;*/ }
{OP_or}         { /*return OP_OR;*/ }
{OP_and}        { /*return OP_AND;*/ }
{OP_not}        {/*return OP_NOT;*/}
{OP_lt}         {/*return OP_LT;*/}
{OP_gt}         {/*return OP_GT;*/}
{OP_ge}         {/*return OP_GE;*/}
{OP_le}         {/*return OP_LE;*/}
{OP_ne}         {/*return OP_NE;*/}
{OP_eq}         {/*return OP_EQ;*/}
{OP_plus}        {/*return OP_PLUS;*/}
{OP_minus}      {/*return OP_MINUS;*/}
{OP_multiply}   {/*return OP_MULTIPLY;*/}
{OP_divide}     {/*return OP_DIVIDE;*/}

{DL_lparen}     { /*return MK_LPAREN;*/ }
{DL_rparen}     { /*return MK_RPAREN;*/ }
{DL_lbrace}     { /*return MK_LBRACE;*/ }
{DL_rbrace}     { /*return MK_RBRACE;*/ }
{DL_rbracket}   {/*return MK_RIGHTBRACKET;*/}
{DL_lbracket}   {/*return MK_LBRACKET;*/}
{DL_comma}      {}
{DL_semicol}    {}
{newline}   { linenumber += 1; 
             printf("linenumber: %d\n",linenumber); }
{error}         {printf("ERR \n"); /*return ERROR;*/ }


%%

int main(int argc, char **argv)
{
 argc--; ++argv;
   if (argc > 0)
      yyin = fopen(argv[0], "r");
   else
      yyin = stdin;
   yylex();
   printSymTab();
   return 0;
}

