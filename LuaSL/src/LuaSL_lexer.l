%{

#define excludeLexer
#include "LuaSL_LSL_tree.h"

%}

%option reentrant noyywrap never-interactive nounistd
%option bison-bridge

SPACE       [ \r\n\t]*
NUMBER      [0-9]+
NAME        [a-zA-Z][a-zA-Z0-9]*

%%

{SPACE}         { /* Skip blanks. */ }
{NUMBER}        { yylval->integerValue = atoi(yytext); return LSL_INTEGER; }
{NAME}          { /* yylval->nameValue=strdup(yytext); return LSL_NAME; */ }

"*"             { return LSL_MULTIPLY; }
"+"             { return LSL_ADD; }
"("             { return LSL_PARENTHESIS_OPEN; }
")"             { return LSL_PARENTHESIS_CLOSE; }
.               { /* This should return an "unknown character" error */  }

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

