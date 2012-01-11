%{

#define excludeLexer
#include "LuaSL_LSL_tree.h"
#include <stdio.h>

int common(YYSTYPE *lval, char *text, boolean checkIgnorable, int type);

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
IDENTIFIER  [[:alpha:]](_|[[:alpha:]]|[[:digit:]])*

%%

 /* The order here is important, in mysterious ways. The more specific the lower in case of ambiguities like "floats contain integers". I think, not tested that well yet. */

 /* Ignorables. */
[[:space:]]+    %{ common(yylval, yytext, FALSE, LSL_SPACE); %}
 /* Yes I know this will have problems with huge comments, just being simple to get it to work for now. */
"/*"([^"*/"]*)"*/"   %{ common(yylval, yytext, FALSE, LSL_COMMENT); %}
"//"[^\n]*      %{ common(yylval, yytext, FALSE, LSL_COMMENT_LINE); %}

 /* Operations. */
"&&"            { return common(yylval, yytext, TRUE, LSL_BOOL_AND); }
"||"            { return common(yylval, yytext, TRUE, LSL_BOOL_OR); }
"|"             { return common(yylval, yytext, TRUE, LSL_BIT_OR); }
"^"             { return common(yylval, yytext, TRUE, LSL_BIT_XOR); }
"&"             { return common(yylval, yytext, TRUE, LSL_BIT_AND); }
"!="            { return common(yylval, yytext, TRUE, LSL_NOT_EQUAL); }
"=="            { return common(yylval, yytext, TRUE, LSL_EQUAL); }
">="            { return common(yylval, yytext, TRUE, LSL_GREATER_EQUAL); }
"<="            { return common(yylval, yytext, TRUE, LSL_LESS_EQUAL); }
">"             { return common(yylval, yytext, TRUE, LSL_GREATER_THAN); }
"<"             { return common(yylval, yytext, TRUE, LSL_LESS_THAN); }
">>"            { return common(yylval, yytext, TRUE, LSL_RIGHT_SHIFT); }
"<<"            { return common(yylval, yytext, TRUE, LSL_LEFT_SHIFT); }
"+"             { return common(yylval, yytext, TRUE, LSL_ADD); }
"-"             { return common(yylval, yytext, TRUE, LSL_SUBTRACT); }
"*"             { return common(yylval, yytext, TRUE, LSL_MULTIPLY); }
"%"             { return common(yylval, yytext, TRUE, LSL_MODULO); }
"/"             { return common(yylval, yytext, TRUE, LSL_DIVIDE); }
"!"             { return common(yylval, yytext, TRUE, LSL_BOOL_NOT); }
"~"             { return common(yylval, yytext, TRUE, LSL_BIT_NOT); }
"["             { return common(yylval, yytext, TRUE, LSL_BRACKET_OPEN); }
"]"             { return common(yylval, yytext, TRUE, LSL_BRACKET_CLOSE); }
"("             { return common(yylval, yytext, TRUE, LSL_PARENTHESIS_OPEN); }
")"             { return common(yylval, yytext, TRUE, LSL_PARENTHESIS_CLOSE); }
"+="            { return common(yylval, yytext, TRUE, LSL_ASSIGNMENT_ADD); }
"-="            { return common(yylval, yytext, TRUE, LSL_ASSIGNMENT_SUBTRACT); }
"*="            { return common(yylval, yytext, TRUE, LSL_ASSIGNMENT_MULTIPLY); }
"%="            { return common(yylval, yytext, TRUE, LSL_ASSIGNMENT_MODULO); }
"/="            { return common(yylval, yytext, TRUE, LSL_ASSIGNMENT_DIVIDE); }
"="             { return common(yylval, yytext, TRUE, LSL_ASSIGNMENT_PLAIN); }
"."             { return common(yylval, yytext, TRUE, LSL_DOT); }
"--"            { return common(yylval, yytext, TRUE, LSL_DECREMENT_PRE); }
"++"            { return common(yylval, yytext, TRUE, LSL_INCREMENT_PRE); }
","             { return common(yylval, yytext, TRUE, LSL_COMMA); }

 /* Types. */
{INTEGER}       %{ yylval->value.integerValue = atoi(yytext); return common(yylval, yytext, TRUE, LSL_INTEGER); %}
{FLOAT}         %{ yylval->value.floatValue = atof(yytext); return common(yylval, yytext, TRUE, LSL_FLOAT); %}

 /* Type keywords. */
"float"         %{ return common(yylval, yytext, TRUE, LSL_TYPE_FLOAT); %}
"integer"       %{ return common(yylval, yytext, TRUE, LSL_TYPE_INTEGER); %}
"key"           %{ return common(yylval, yytext, TRUE, LSL_TYPE_KEY); %}
"list"          %{ return common(yylval, yytext, TRUE, LSL_TYPE_LIST); %}
"quaternion"    %{ return common(yylval, yytext, TRUE, LSL_TYPE_ROTATION); %}
"rotation"      %{ return common(yylval, yytext, TRUE, LSL_TYPE_ROTATION); %}
"string"        %{ return common(yylval, yytext, TRUE, LSL_TYPE_STRING); %}
"vector"        %{ return common(yylval, yytext, TRUE, LSL_TYPE_VECTOR); %}

 /* Statement keywords. */
"do"            %{ return common(yylval, yytext, TRUE, LSL_DO); %}
"for"           %{ return common(yylval, yytext, TRUE, LSL_FOR); %}
"else"          %{ return common(yylval, yytext, TRUE, LSL_ELSE); %}
"if"            %{ return common(yylval, yytext, TRUE, LSL_IF); %}
"jump"          %{ return common(yylval, yytext, TRUE, LSL_JUMP); %}
"return"        %{ return common(yylval, yytext, TRUE, LSL_RETURN); %}
"state"         %{ return common(yylval, yytext, TRUE, LSL_STATE_CHANGE); %}
"while"         %{ return common(yylval, yytext, TRUE, LSL_WHILE); %}

{IDENTIFIER}    %{ /* yylval->value.identifierValue = strdup(yytext); */ common(yylval, yytext, TRUE, LSL_IDENTIFIER); %}

 /* Other symbols. */
"@"             %{ return common(yylval, yytext, TRUE, LSL_LABEL); %}
"{"             %{ return common(yylval, yytext, TRUE, LSL_BLOCK_OPEN); %}
"}"             %{ return common(yylval, yytext, TRUE, LSL_BLOCK_CLOSE); %}
";"             %{ return common(yylval, yytext, TRUE, LSL_STATEMENT); %}

<<EOF>>         { return common(yylval, yytext, TRUE, LSL_SCRIPT); }

 /* Everything else */
.               %{ printf(" unexpected character.\n"); yylval->value.unknownValue = strdup(yytext); common(yylval, yytext, TRUE, LSL_UNKNOWN); %}

%%

// TODO - this is not reentrant, should make it so.
static char *ignorableText = NULL;
static int column = 0;
static int line = 0;

int common(YYSTYPE *lval, char *text, boolean checkIgnorable, int type)
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

    lval->token = tokens[type - lowestToken];
    lval->line = line;
    lval->column = column;

    if (checkIgnorable)
    {
	lval->ignorableText = ignorableText;
	ignorableText = NULL;
    }
    else
    {
	if (ignorableText)
	{
	    int lenI = strlen(ignorableText);
	    int lenT = strlen(text);

	    ignorableText = realloc(ignorableText, lenI + lenT + 1);
	    sprintf(&ignorableText[lenI], "%s", text);
	}
	else
	    ignorableText = strdup(text);
    }

    return type;
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

