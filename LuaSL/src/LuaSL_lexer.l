%{

#define excludeLexer
#include "LuaSL_LSL_tree.h"
#include <stdio.h>

void count(char *text);

#ifdef LUASL_DEBUG
    #undef ECHO
    #define ECHO count(yytext); printf ("[%s]\n", yytext)
#else
    #undef ECHO
    #define ECHO count(yytext)
#endif

%}

%option reentrant never-interactive batch
%option bison-bridge yylineno 8bit 
%option noreject noyymore
%option backup debug perf-report perf-report verbose warn
%option align full

HEX         [[:xdigit:]]
INTEGER     [[:digit:]]+
EXPONANT    [eE][+-]?{INTEGER}
FLOAT       {INTEGER}("."{INTEGER})?{EXPONANT}?
CHAR        '(\\.|[^\\'\n])+'
STRING      \"(\\.|[^\\"\n])*\"
NAME        [[:alpha:]](_|[[:alpha:]]|[[:digit:]])*

%%

 /* The order here is important, in mysterious ways. The more specific the lower in case of ambiguities like "floats contain integers". I think, not tested that well yet. */

 /* White space. */
[[:space:]]+    %{ ECHO; /* yylval->spaceValue = strdup(yytext); return LSL_SPACE; */ %}
"/*"            %{ /* count(); comment(); */ %}
"//"[^\n]*      %{ /* consume //-comment */ %}

 /* Operations. */
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
 /* "<"              { ECHO; return LSL_ANGLE_OPEN; } */
 /* ">"              { ECHO; return LSL_ANGLE_CLOSE; } */
"["             { ECHO; return LSL_BRACKET_OPEN; }
"]"             { ECHO; return LSL_BRACKET_CLOSE; }
"("             { ECHO; return LSL_PARENTHESIS_OPEN; }
")"             { ECHO; return LSL_PARENTHESIS_CLOSE; }
"+="            { ECHO; return LSL_ASSIGNMENT_ADD; }
"-="            { ECHO; return LSL_ASSIGNMENT_SUBTRACT; }
"*="            { ECHO; return LSL_ASSIGNMENT_MULTIPLY; }
"%="            { ECHO; return LSL_ASSIGNMENT_MODULO; }
"/="            { ECHO; return LSL_ASSIGNMENT_DIVIDE; }
"="             { ECHO; return LSL_ASSIGNMENT_PLAIN; }
"."             { ECHO; return LSL_DOT; }
"--"            { ECHO; return LSL_DECREMENT_PRE; }
"++"            { ECHO; return LSL_INCREMENT_PRE; }
","             { ECHO; return LSL_COMMA; }

 /* Types. */
{INTEGER}       %{ ECHO; yylval->integerValue = atoi(yytext); return LSL_INTEGER; %}
{FLOAT}         %{ ECHO; yylval->floatValue = atof(yytext); return LSL_FLOAT; %}

 /* Type keywords. */
"float"         %{ ECHO; return LSL_TYPE_FLOAT; %}
"integer"       %{ ECHO; return LSL_TYPE_INTEGER; %}
"key"           %{ ECHO; return LSL_TYPE_KEY; %}
"list"          %{ ECHO; return LSL_TYPE_LIST; %}
"quaternion"    %{ ECHO; return LSL_TYPE_ROTATION; %}
"rotation"      %{ ECHO; return LSL_TYPE_ROTATION; %}
"string"        %{ ECHO; return LSL_TYPE_STRING; %}
"vector"        %{ ECHO; return LSL_TYPE_VECTOR; %}

 /* Statement keywords. */
"do"            %{ ECHO; return LSL_DO; %}
"for"           %{ ECHO; return LSL_FOR; %}
"else"          %{ ECHO; return LSL_ELSE; %}
"if"            %{ ECHO; return LSL_IF; %}
"jump"          %{ ECHO; return LSL_JUMP; %}
"return"        %{ ECHO; return LSL_RETURN; %}
"state"         %{ ECHO; return LSL_STATE_CHANGE; %}
"while"         %{ ECHO; return LSL_WHILE; %}

{NAME}          %{ ECHO; /* yylval->nameValue = strdup(yytext); return LSL_NAME; */ %}

 /* Other symbols. */
"@"             %{ ECHO; return LSL_LABEL; %}
"{"             %{ ECHO; return LSL_BLOCK_OPEN; %}
"}"             %{ ECHO; return LSL_BLOCK_CLOSE; %}
";"             { ECHO; return LSL_STATEMENT; }

<<EOF>>         { yyterminate(); }

 /* Everything else */
.               %{ ECHO; printf(" unexpected character.\n"); %}

%%

int column = 0;
int line = 0;

void count(char *text)
{
    int i;

    for (i = 0; text[i] != '\0'; i++)
	if (text[i] == '\n')
	{
	    column = 0;
	    line++;
	}
	else if (text[i] == '\t')
	    column += 8 - (column % 8);
	else
	    column++;
}

int yyerror(const char *msg)
{
    fprintf(stderr, "Parser error on line %d, column %d: %s\n", line, column, msg);
    return 0;
}

int yywrap(yyscan_t yyscanner)
{
#ifdef FLEX_SCANNER
    #ifndef LL_WINDOWS
	// Get gcc to stop complaining about lack of use of yyunput and input.
	(void) yyunput;
	(void) input;
    #endif
#endif
// TODO - If we are getting files from stdin, or multiple -f arguments, we should loop through them and return 0.  Return 1 when there are no more files.
	return(1);
}

