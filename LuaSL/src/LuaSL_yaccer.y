%{

#include "LuaSL_LSL_tree.h"

%}

%define api.pure

%left '+' LSL_ADD
%left '*' LSL_MULTIPLY

%token LSL_PARENTHESIS_OPEN
%token LSL_PARENTHESIS_CLOSE
%token LSL_ADD
%token LSL_MULTIPLY

%token <integerValue> LSL_INTEGER

%type <expressionValue> expr

%%

input: 
        expr { ((LuaSL_yyparseParam*)data)->expression = $1; }
        ;

expr:
      expr LSL_ADD expr { $$ = addOperation( LSL_ADD, $1, $3 ); }
    | expr LSL_MULTIPLY expr { $$ = addOperation( LSL_MULTIPLY, $1, $3 ); }
    | LSL_PARENTHESIS_OPEN expr LSL_PARENTHESIS_CLOSE { $$ = $2; }
    | LSL_INTEGER { $$ = addInteger($1); }
;

%%

