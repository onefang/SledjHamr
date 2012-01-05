%{

#define excludeLexer
#include "LuaSL_LSL_tree.h"

%}

%option reentrant noyywrap never-interactive nounistd
%option bison-bridge

LPAREN      "("
RPAREN      ")"
PLUS        "+"
MULTIPLY    "*"

NUMBER      [0-9]+
WS          [ \r\n\t]*

%%

{WS}            { /* Skip blanks. */ }
{NUMBER}        { sscanf(yytext,"%d",&yylval->integerValue); return TOKEN_NUMBER; }

{MULTIPLY}      { return TOKEN_MULTIPLY; }
{PLUS}          { return TOKEN_PLUS; }
{LPAREN}        { return TOKEN_LPAREN; }
{RPAREN}        { return TOKEN_RPAREN; }
.               {  }

%%

int XXyywrap()
{
#ifdef FLEX_SCANNER
    #ifndef LL_WINDOWS
	// get gcc to stop complaining about lack of use of yyunput
	(void) yyunput;
	(void) input;
    #endif
#endif
	return(1);
}

