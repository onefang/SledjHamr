%{

#include "LuaSL_LSL_tree.h"

//extern char *yytext;
//#define YYDEBUG_LEXER_TEXT yytext

%}

%define api.pure

%left '+' LSL_ADD
%left '*' LSL_MULTIPLY

%token LSL_ADD
%token LSL_MULTIPLY
%token LSL_PARENTHESIS_OPEN LSL_PARENTHESIS_CLOSE

%token <integerValue> LSL_INTEGER

%type <expressionValue> expr

%%

input: 
        expr { ((LuaSL_yyparseParam*)data)->expression = $1; }
        ;

expr:
     LSL_PARENTHESIS_OPEN expr LSL_PARENTHESIS_CLOSE { $$ = $2; }
    | expr LSL_MULTIPLY expr { $$ = addOperation( LSL_MULTIPLY, $1, $3 ); }
    | expr LSL_ADD expr { $$ = addOperation( LSL_ADD, $1, $3 ); }
    | LSL_INTEGER { $$ = addInteger($1); }
;

%%

