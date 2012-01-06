%{

#define excludeLexer
#include "LuaSL_LSL_tree.h"

#ifdef LUASL_DEBUG
    #undef ECHO
    #define ECHO printf ("[%s]\n", yytext)
#else
    #undef ECHO
    #define ECHO {}
#endif


%}

%option reentrant noyywrap never-interactive nounistd
%option bison-bridge

SPACE       [ \r\n\t]*
NAME        [a-zA-Z_][a-zA-Z0-9_]*
NUMBER      [0-9]+

%%

{SPACE}         { ECHO; /* Skip blanks. */ }
{NAME}          { ECHO; /* yylval->nameValue=strdup(yytext); return LSL_NAME; */ }
{NUMBER}        { ECHO; yylval->integerValue = atoi(yytext); return LSL_INTEGER; }

"("             { ECHO; return LSL_PARENTHESIS_OPEN; }
")"             { ECHO; return LSL_PARENTHESIS_CLOSE; }
"*"             { ECHO; return LSL_MULTIPLY; }
"+"             { ECHO; return LSL_ADD; }
.               { ECHO; printf(" unexpected character.\n"); }

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

