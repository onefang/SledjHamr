%{

#include "LuaSL_LSL_tree.h"

%}

%define api.pure

%left '+' TOKEN_PLUS
%left '*' TOKEN_MULTIPLY

%token TOKEN_LPAREN
%token TOKEN_RPAREN
%token TOKEN_PLUS
%token TOKEN_MULTIPLY

%token <integerValue> TOKEN_NUMBER

%type <expressionValue> expr

%%

input: 
        expr { ((LuaSL_yyparseParam*)data)->expression = $1; }
        ;

expr:
      expr TOKEN_PLUS expr { $$ = addOperation( LSL_ADD, $1, $3 ); }
    | expr TOKEN_MULTIPLY expr { $$ = addOperation( LSL_MULTIPLY, $1, $3 ); }
    | TOKEN_LPAREN expr TOKEN_RPAREN { $$ = $2; }
    | TOKEN_NUMBER { $$ = addInteger($1); }
;

%%

