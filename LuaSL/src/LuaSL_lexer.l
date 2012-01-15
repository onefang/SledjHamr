%{

#define excludeLexer
#include "LuaSL_LSL_tree.h"

int common(YYSTYPE *lval, char *text, LuaSL_yyparseParam *param, boolean checkIgnorable, int type);

%}

%option reentrant never-interactive batch
%option bison-bridge yylineno 8bit 
%option noreject noyymore
%option backup debug perf-report perf-report verbose warn
%option align full
%option extra-type="LuaSL_yyparseParam *"

HEX         [[:xdigit:]]
DECIMAL     [[:digit:]]
 /* LSL has no octal integer type. */
INTEGER     ({DECIMAL}+)|(0[xX]{HEX}+)
EXPONANT    [eE][+-]?{DECIMAL}+
 /* Floats can be "0." or".0", but "." is not valid.  At least in OpenSim. A single dot should be caught by the LSL_Dot rule first anyway.*/
FLOAT       {DECIMAL}*"."{DECIMAL}*{EXPONANT}?[fF]?
CHAR        '(\\.|[^\\'\n])+'
STRING      \"(\\.|[^\\"\n])*\"
IDENTIFIER  [[:alpha:]](_|[[:alpha:]]|[[:digit:]])*

%%

 /* The order here is important, in mysterious ways. The more specific the lower in case of ambiguities like "floats contain integers". I think, not tested that well yet. */

 /* Ignorables. */
[[:space:]]+    %{ common(yylval, yytext, yyextra, FALSE, LSL_SPACE); %}
 /* Yes I know this will have problems with huge comments, just being simple to get it to work for now. */
"/*"([^"*/"]*)"*/"   %{ common(yylval, yytext, yyextra, FALSE, LSL_COMMENT); %}
"//"[^\n]*      %{ common(yylval, yytext, yyextra, FALSE, LSL_COMMENT_LINE); %}

 /* Operations. */
"&&"            { return common(yylval, yytext, yyextra, TRUE, LSL_BOOL_AND); }
"||"            { return common(yylval, yytext, yyextra, TRUE, LSL_BOOL_OR); }
"|"             { return common(yylval, yytext, yyextra, TRUE, LSL_BIT_OR); }
"^"             { return common(yylval, yytext, yyextra, TRUE, LSL_BIT_XOR); }
"&"             { return common(yylval, yytext, yyextra, TRUE, LSL_BIT_AND); }
"!="            { return common(yylval, yytext, yyextra, TRUE, LSL_NOT_EQUAL); }
"=="            { return common(yylval, yytext, yyextra, TRUE, LSL_EQUAL); }
">="            { return common(yylval, yytext, yyextra, TRUE, LSL_GREATER_EQUAL); }
"<="            { return common(yylval, yytext, yyextra, TRUE, LSL_LESS_EQUAL); }
">"             { return common(yylval, yytext, yyextra, TRUE, LSL_GREATER_THAN); }
"<"             { return common(yylval, yytext, yyextra, TRUE, LSL_LESS_THAN); }
">>"            { return common(yylval, yytext, yyextra, TRUE, LSL_RIGHT_SHIFT); }
"<<"            { return common(yylval, yytext, yyextra, TRUE, LSL_LEFT_SHIFT); }
"+"             { return common(yylval, yytext, yyextra, TRUE, LSL_ADD); }
"-"             { return common(yylval, yytext, yyextra, TRUE, LSL_SUBTRACT); }
"*"             { return common(yylval, yytext, yyextra, TRUE, LSL_MULTIPLY); }
"%"             { return common(yylval, yytext, yyextra, TRUE, LSL_MODULO); }
"/"             { return common(yylval, yytext, yyextra, TRUE, LSL_DIVIDE); }
"!"             { return common(yylval, yytext, yyextra, TRUE, LSL_BOOL_NOT); }
"~"             { return common(yylval, yytext, yyextra, TRUE, LSL_BIT_NOT); }
"["             { return common(yylval, yytext, yyextra, TRUE, LSL_BRACKET_OPEN); }
"]"             { return common(yylval, yytext, yyextra, TRUE, LSL_BRACKET_CLOSE); }
"("             { return common(yylval, yytext, yyextra, TRUE, LSL_PARENTHESIS_OPEN); }
")"             { return common(yylval, yytext, yyextra, TRUE, LSL_PARENTHESIS_CLOSE); }
"+="            { return common(yylval, yytext, yyextra, TRUE, LSL_ASSIGNMENT_ADD); }
"-="            { return common(yylval, yytext, yyextra, TRUE, LSL_ASSIGNMENT_SUBTRACT); }
"*="            { return common(yylval, yytext, yyextra, TRUE, LSL_ASSIGNMENT_MULTIPLY); }
"%="            { return common(yylval, yytext, yyextra, TRUE, LSL_ASSIGNMENT_MODULO); }
"/="            { return common(yylval, yytext, yyextra, TRUE, LSL_ASSIGNMENT_DIVIDE); }
"="             { return common(yylval, yytext, yyextra, TRUE, LSL_ASSIGNMENT_PLAIN); }
"."             { return common(yylval, yytext, yyextra, TRUE, LSL_DOT); }
"--"            { return common(yylval, yytext, yyextra, TRUE, LSL_DECREMENT_PRE); }
"++"            { return common(yylval, yytext, yyextra, TRUE, LSL_INCREMENT_PRE); }
","             { return common(yylval, yytext, yyextra, TRUE, LSL_COMMA); }

 /* Types. */
{INTEGER}       %{ yylval->value.integerValue = atoi(yytext); return common(yylval, yytext, yyextra, TRUE, LSL_INTEGER); %}
{FLOAT}         %{ yylval->value.floatValue = atof(yytext); return common(yylval, yytext, yyextra, TRUE, LSL_FLOAT); %}

 /* Type keywords. */
"float"         %{ return common(yylval, yytext, yyextra, TRUE, LSL_TYPE_FLOAT); %}
"integer"       %{ return common(yylval, yytext, yyextra, TRUE, LSL_TYPE_INTEGER); %}
"key"           %{ return common(yylval, yytext, yyextra, TRUE, LSL_TYPE_KEY); %}
"list"          %{ return common(yylval, yytext, yyextra, TRUE, LSL_TYPE_LIST); %}
"quaternion"    %{ return common(yylval, yytext, yyextra, TRUE, LSL_TYPE_ROTATION); %}
"rotation"      %{ return common(yylval, yytext, yyextra, TRUE, LSL_TYPE_ROTATION); %}
"string"        %{ return common(yylval, yytext, yyextra, TRUE, LSL_TYPE_STRING); %}
"vector"        %{ return common(yylval, yytext, yyextra, TRUE, LSL_TYPE_VECTOR); %}

 /* Statement keywords. */
"do"            %{ return common(yylval, yytext, yyextra, TRUE, LSL_DO); %}
"for"           %{ return common(yylval, yytext, yyextra, TRUE, LSL_FOR); %}
"else"          %{ return common(yylval, yytext, yyextra, TRUE, LSL_ELSE); %}
"if"            %{ return common(yylval, yytext, yyextra, TRUE, LSL_IF); %}
"jump"          %{ return common(yylval, yytext, yyextra, TRUE, LSL_JUMP); %}
"return"        %{ return common(yylval, yytext, yyextra, TRUE, LSL_RETURN); %}
"state"         %{ return common(yylval, yytext, yyextra, TRUE, LSL_STATE_CHANGE); %}
"while"         %{ return common(yylval, yytext, yyextra, TRUE, LSL_WHILE); %}

{IDENTIFIER}    %{ /* yylval->value.identifierValue = strdup(yytext); */ common(yylval, yytext, yyextra, TRUE, LSL_IDENTIFIER); %}

 /* Other symbols. */
"@"             %{ return common(yylval, yytext, yyextra, TRUE, LSL_LABEL); %}
"{"             %{ return common(yylval, yytext, yyextra, TRUE, LSL_BLOCK_OPEN); %}
"}"             %{ return common(yylval, yytext, yyextra, TRUE, LSL_BLOCK_CLOSE); %}
";"             %{ return common(yylval, yytext, yyextra, TRUE, LSL_STATEMENT); %}

<<EOF>>         { return common(yylval, yytext, yyextra, TRUE, LSL_SCRIPT); }

 /* Everything else */
.               %{ printf(" unexpected character.\n"); yylval->value.unknownValue = strdup(yytext); common(yylval, yytext, yyextra, TRUE, LSL_UNKNOWN); %}

%%

int common(YYSTYPE *lval, char *text, LuaSL_yyparseParam *param, boolean checkIgnorable, int type)
{
    int i;

    for (i = 0; text[i] != '\0'; i++)
	if (text[i] == '\n')
	{
	    param->column = 0;
	    param->line++;
	}
	else if (text[i] == '\t')
	    param->column += 8 - (param->column % 8);
	else
	    param->column++;

    lval->token = tokens[type - lowestToken];
    lval->line = param->line;
    lval->column = param->column;

    if (checkIgnorable)
    {
	lval->ignorableText = param->ignorableText;
	param->ignorableText = NULL;
    }
    else
    {
	if (param->ignorableText)
	{
	    int lenI = strlen(param->ignorableText);
	    int lenT = strlen(text);

	    param->ignorableText = realloc(param->ignorableText, lenI + lenT + 1);
	    sprintf(&(param->ignorableText[lenI]), "%s", text);
	}
	else
	    param->ignorableText = strdup(text);
    }

    return type;
}

int yywrap(yyscan_t yyscanner)  // This as actually useless for our needs, as it is called BEFORE the last token is dealt with.
{
#ifdef FLEX_SCANNER
    #ifndef LL_WINDOWS
	// Get gcc to stop complaining about lack of use of yyunput and input.
	(void) yyunput;
	(void) input;
    #endif
#endif

    return 1;
}

