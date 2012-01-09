%{

#define excludeLexer
#include "LuaSL_LSL_tree.h"
#include <stdio.h>

void comment(yyscan_t yyscanner);
void common(YYSTYPE *lval, char *text, boolean checkIgnorable);
void ignorable(char *text);

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

 /* Ignorables. */
[[:space:]]+    %{ common(yylval, yytext, FALSE); ignorable(yytext); %}
 /* Yes I know this will have problems with huge comments, just being simple to get it to work for now. */
"/*"([^"*/"]*)"*/"   %{ common(yylval, yytext, FALSE); ignorable(yytext); %}
"//"[^\n]*      %{ common(yylval, yytext, FALSE); ignorable(yytext); %}

 /* Operations. */
"&&"            { common(yylval, yytext, TRUE); return LSL_BOOL_AND; }
"||"            { common(yylval, yytext, TRUE); return LSL_BOOL_OR; }
"|"             { common(yylval, yytext, TRUE); return LSL_BIT_OR; }
"^"             { common(yylval, yytext, TRUE); return LSL_BIT_XOR; }
"&"             { common(yylval, yytext, TRUE); return LSL_BIT_AND; }
"!="            { common(yylval, yytext, TRUE); return LSL_NOT_EQUAL; }
"=="            { common(yylval, yytext, TRUE); return LSL_EQUAL; }
">="            { common(yylval, yytext, TRUE); return LSL_GREATER_EQUAL; }
"<="            { common(yylval, yytext, TRUE); return LSL_LESS_EQUAL; }
">"             { common(yylval, yytext, TRUE); return LSL_GREATER_THAN; }
"<"             { common(yylval, yytext, TRUE); return LSL_LESS_THAN; }
">>"            { common(yylval, yytext, TRUE); return LSL_RIGHT_SHIFT; }
"<<"            { common(yylval, yytext, TRUE); return LSL_LEFT_SHIFT; }
"+"             { common(yylval, yytext, TRUE); return LSL_ADD; }
"-"             { common(yylval, yytext, TRUE); return LSL_SUBTRACT; }
"*"             { common(yylval, yytext, TRUE); return LSL_MULTIPLY; }
"%"             { common(yylval, yytext, TRUE); return LSL_MODULO; }
"/"             { common(yylval, yytext, TRUE); return LSL_DIVIDE; }
"!"             { common(yylval, yytext, TRUE); return LSL_BOOL_NOT; }
"~"             { common(yylval, yytext, TRUE); return LSL_BIT_NOT; }
"["             { common(yylval, yytext, TRUE); return LSL_BRACKET_OPEN; }
"]"             { common(yylval, yytext, TRUE); return LSL_BRACKET_CLOSE; }
"("             { common(yylval, yytext, TRUE); return LSL_PARENTHESIS_OPEN; }
")"             { common(yylval, yytext, TRUE); return LSL_PARENTHESIS_CLOSE; }
"+="            { common(yylval, yytext, TRUE); return LSL_ASSIGNMENT_ADD; }
"-="            { common(yylval, yytext, TRUE); return LSL_ASSIGNMENT_SUBTRACT; }
"*="            { common(yylval, yytext, TRUE); return LSL_ASSIGNMENT_MULTIPLY; }
"%="            { common(yylval, yytext, TRUE); return LSL_ASSIGNMENT_MODULO; }
"/="            { common(yylval, yytext, TRUE); return LSL_ASSIGNMENT_DIVIDE; }
"="             { common(yylval, yytext, TRUE); return LSL_ASSIGNMENT_PLAIN; }
"."             { common(yylval, yytext, TRUE); return LSL_DOT; }
"--"            { common(yylval, yytext, TRUE); return LSL_DECREMENT_PRE; }
"++"            { common(yylval, yytext, TRUE); return LSL_INCREMENT_PRE; }
","             { common(yylval, yytext, TRUE); return LSL_COMMA; }

 /* Types. */
{INTEGER}       %{ common(yylval, yytext, TRUE); yylval->value.integerValue = atoi(yytext); return LSL_INTEGER; %}
{FLOAT}         %{ common(yylval, yytext, TRUE); yylval->value.floatValue = atof(yytext); return LSL_FLOAT; %}

 /* Type keywords. */
"float"         %{ common(yylval, yytext, TRUE); return LSL_TYPE_FLOAT; %}
"integer"       %{ common(yylval, yytext, TRUE); return LSL_TYPE_INTEGER; %}
"key"           %{ common(yylval, yytext, TRUE); return LSL_TYPE_KEY; %}
"list"          %{ common(yylval, yytext, TRUE); return LSL_TYPE_LIST; %}
"quaternion"    %{ common(yylval, yytext, TRUE); return LSL_TYPE_ROTATION; %}
"rotation"      %{ common(yylval, yytext, TRUE); return LSL_TYPE_ROTATION; %}
"string"        %{ common(yylval, yytext, TRUE); return LSL_TYPE_STRING; %}
"vector"        %{ common(yylval, yytext, TRUE); return LSL_TYPE_VECTOR; %}

 /* Statement keywords. */
"do"            %{ common(yylval, yytext, TRUE); return LSL_DO; %}
"for"           %{ common(yylval, yytext, TRUE); return LSL_FOR; %}
"else"          %{ common(yylval, yytext, TRUE); return LSL_ELSE; %}
"if"            %{ common(yylval, yytext, TRUE); return LSL_IF; %}
"jump"          %{ common(yylval, yytext, TRUE); return LSL_JUMP; %}
"return"        %{ common(yylval, yytext, TRUE); return LSL_RETURN; %}
"state"         %{ common(yylval, yytext, TRUE); return LSL_STATE_CHANGE; %}
"while"         %{ common(yylval, yytext, TRUE); return LSL_WHILE; %}

{NAME}          %{ common(yylval, yytext, TRUE); /* yylval->value.nameValue = strdup(yytext); return LSL_NAME; */ %}

 /* Other symbols. */
"@"             %{ common(yylval, yytext, TRUE); return LSL_LABEL; %}
"{"             %{ common(yylval, yytext, TRUE); return LSL_BLOCK_OPEN; %}
"}"             %{ common(yylval, yytext, TRUE); return LSL_BLOCK_CLOSE; %}
";"             %{ common(yylval, yytext, TRUE); return LSL_STATEMENT; %}

<<EOF>>         { yyterminate(); }

 /* Everything else */
.               %{ common(yylval, yytext, TRUE); printf(" unexpected character.\n"); %}

%%

void comment(yyscan_t yyscanner)
{
    char c, prev = 0;

    while ((c = input(yyscanner)) != 0)      /* (EOF maps to 0) */
    {
	if (c == '/' && prev == '*')
	    return;
	prev = c;
    }
    yyerror("unterminated comment");
}

static char *ignorableText = NULL;
static int column = 0;
static int line = 0;

void common(YYSTYPE *lval, char *text, boolean checkIgnorable)
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

    lval->line = line;
    lval->column = column;

    if (checkIgnorable)
    {
	lval->ignorableText = ignorableText;
	ignorableText = NULL;
    }

#ifdef LUASL_DEBUG
	printf ("%04d, %04d [%s]\n", line, column, text);
#endif
}

void ignorable(char *text)
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
    #endif
#endif
// TODO - If we are getting files from stdin, or multiple -f arguments, we should loop through them and return 0.  Return 1 when there are no more files.
	return(1);
}

