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

%option reentrant never-interactive batch
%option bison-bridge yylineno 8bit 
%option noreject noyymore noyywrap
%option backup debug perf-report perf-report verbose warn
%option align full

SPACE       [ \r\n\t]*
ENDSPACE    {SPACE}$

LETTER      [A-Za-z]
DECIMAL     [0-9]
HEX         [0-9A-Fa-f]

NAME        {LETTER}(_|{LETTER}|{DECIMAL})*
INTEGER     {DECIMAL}+
EXPONANT    [eE][+-]?{INTEGER}
FLOAT       {INTEGER}("."{INTEGER})?{EXPONANT}?

%%

 /* Basic tokens */
{ENDSPACE}      %{ /* Skip blanks at end of lines. */ %}
{SPACE}         %{ ECHO; /* Skip blanks. */ %}
{NAME}          %{ ECHO; /* yylval->nameValue=strdup(yytext); return LSL_NAME; */ %}
{INTEGER}       %{ ECHO; yylval->integerValue = atoi(yytext); return LSL_INTEGER; %}
{FLOAT}         %{ ECHO; /* yylval->floatValue = atof(yytext); return LSL_FLOAT; */ %}

 /* Expression tokens */
"&&"            { ECHO; return LSL_BOOL_AND; }
"||"            { ECHO; return LSL_BOOL_OR; }
"|"             { ECHO; return LSL_BIT_OR; }
"^"             { ECHO; return LSL_BIT_XOR; }
"&"             { ECHO; return LSL_BIT_AND; }
"!="            { ECHO; return LSL_NOT_EQUAL; }
"=="            { ECHO; return LSL_EQUAL; }
">="            { ECHO; return LSL_GREATER_EQUAL; }
"<="            { ECHO; return LSL_LESS_EQUAL; }
">"             { ECHO; return LSL_GREATER_THAN; }
"<"             { ECHO; return LSL_LESS_THAN; }
">>"            { ECHO; return LSL_RIGHT_SHIFT; }
"<<"            { ECHO; return LSL_LEFT_SHIFT; }
"+"             { ECHO; return LSL_ADD; }
"-"             { ECHO; return LSL_SUBTRACT; }
"*"             { ECHO; return LSL_MULTIPLY; }
"%"             { ECHO; return LSL_MODULO; }
"/"             { ECHO; return LSL_DIVIDE; }
"!"             { ECHO; return LSL_BOOL_NOT; }
"~"             { ECHO; return LSL_BIT_NOT; }
"("             { ECHO; return LSL_PARENTHESIS_OPEN; }
")"             { ECHO; return LSL_PARENTHESIS_CLOSE; }

 /* Everything else */
.               %{ ECHO; printf(" unexpected character.\n"); %}

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

