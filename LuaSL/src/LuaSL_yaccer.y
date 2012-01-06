%{

#include "LuaSL_LSL_tree.h"

//extern char *yytext;
//#define YYDEBUG_LEXER_TEXT yytext

%}

%define api.pure

%token <integerValue> LSL_INTEGER

%left LSL_BOOL_AND
%left LSL_BOOL_OR
%left LSL_BIT_AND LSL_BIT_XOR LSL_BIT_OR
%left LSL_EQUAL LSL_NOT_EQUAL
%left LSL_LESS_THAN LSL_GREATER_THAN LSL_LESS_EQUAL LSL_GREATER_EQUAL
%left LSL_LEFT_SHIFT LSL_RIGHT_SHIFT
%left LSL_SUBTRACT LSL_ADD
%left LSL_DIVIDE LSL_MODULO LSL_MULTIPLY
%right LSL_BIT_NOT LSL_BOOL_NOT LSL_NEGATION

%token LSL_PARENTHESIS_OPEN LSL_PARENTHESIS_CLOSE LSL_EXPRESSION

%type <expressionValue> expr

%%

input : 
    expr { ((LuaSL_yyparseParam*)data)->ast = addExpression($1); }
;

expr :
    LSL_INTEGER { $$ = addInteger($1); }
    | expr LSL_BOOL_AND expr { $$ = addOperation( LSL_BOOL_AND, $1, $3 ); }
    | expr LSL_BOOL_OR expr { $$ = addOperation( LSL_BOOL_OR, $1, $3 ); }
    | expr LSL_BIT_OR expr { $$ = addOperation( LSL_BIT_OR, $1, $3 ); }
    | expr LSL_BIT_XOR expr { $$ = addOperation( LSL_BIT_XOR, $1, $3 ); }
    | expr LSL_BIT_AND expr { $$ = addOperation( LSL_BIT_AND, $1, $3 ); }
    | expr LSL_NOT_EQUAL expr { $$ = addOperation( LSL_NOT_EQUAL, $1, $3 ); }
    | expr LSL_EQUAL expr { $$ = addOperation( LSL_EQUAL, $1, $3 ); }
    | expr LSL_GREATER_EQUAL expr { $$ = addOperation( LSL_GREATER_EQUAL, $1, $3 ); }
    | expr LSL_LESS_EQUAL expr { $$ = addOperation( LSL_LESS_EQUAL, $1, $3 ); }
    | expr LSL_GREATER_THAN expr { $$ = addOperation( LSL_GREATER_THAN, $1, $3 ); }
    | expr LSL_LESS_THAN expr { $$ = addOperation( LSL_LESS_THAN, $1, $3 ); }
    | expr LSL_RIGHT_SHIFT expr { $$ = addOperation( LSL_RIGHT_SHIFT, $1, $3 ); }
    | expr LSL_LEFT_SHIFT expr { $$ = addOperation( LSL_LEFT_SHIFT, $1, $3 ); }
    | expr LSL_ADD expr { $$ = addOperation( LSL_ADD, $1, $3 ); }
    | expr LSL_SUBTRACT expr { $$ = addOperation( LSL_SUBTRACT, $1, $3 ); }
    | expr LSL_MULTIPLY expr { $$ = addOperation( LSL_MULTIPLY, $1, $3 ); }
    | expr LSL_MODULO expr { $$ = addOperation( LSL_MODULO, $1, $3 ); }
    | expr LSL_DIVIDE expr { $$ = addOperation( LSL_DIVIDE, $1, $3 ); }
    | LSL_BIT_NOT expr { $$ = addOperation( LSL_BIT_NOT, NULL, $2 ); }
    | LSL_BOOL_NOT expr { $$ = addOperation( LSL_BOOL_NOT, NULL, $2 ); }
    | LSL_SUBTRACT expr { $$ = addOperation( LSL_NEGATION, NULL, $2 ); }  %prec LSL_NEGATION
    | LSL_PARENTHESIS_OPEN expr LSL_PARENTHESIS_CLOSE { $$ = $2; }
;

%%

