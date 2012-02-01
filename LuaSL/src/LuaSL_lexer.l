%{

#define excludeLexer
#include "LuaSL.h"

int common(YYSTYPE *lval, char *text, int len, LuaSL_compiler *compiler, boolean checkIgnorable, int type);

%}

%option reentrant never-interactive batch
%option bison-bridge 8bit 
%option noreject noyymore
%option backup debug perf-report perf-report verbose warn
%option align full
%option extra-type="LuaSL_compiler *"

HEX         [[:xdigit:]]
DECIMAL     [[:digit:]]
 /* LSL has no octal integer type. */
INTEGER     ({DECIMAL}+)|(0[xX]{HEX}+)
EXPONANT    [eE][+-]?{DECIMAL}+
 /* Floats can be "0." or".0", but "." is not valid.  At least in OpenSim. A single dot should be caught by the LSL_Dot rule first anyway.*/
FLOAT       {DECIMAL}*"."{DECIMAL}*{EXPONANT}?[fF]?
 /* Variable identifiers can have these extra characters at the end or beginning, they will be ignored- #$`'\?  */
 /* Function identifiers can start with $ */
IDENTIFIER  (_|[[:alpha:]])(_|[[:alpha:]]|[[:digit:]])*
CHAR        '(\\.|[^\\'\n])+'
KEY         \"{HEX}{8}-{HEX}{4}-{HEX}{4}-{HEX}{4}-{HEX}{12}\"
STRING      \"(\\.|[^\\"\n])*\"

%%

 /* The order here is important, in mysterious ways. The more specific the lower in case of ambiguities like "floats contain integers". I think, not tested that well yet. */

 /* Ignorables. */
[[:space:]]+    %{ common(yylval, yytext, yyleng, yyextra, FALSE, LSL_SPACE); %}
 /* Yes I know this will have problems with huge comments, just being simple to get it to work for now. */
"/*"([^"*/"]*)"*/"   %{ common(yylval, yytext, yyleng, yyextra, FALSE, LSL_COMMENT); %}
"//"[^\n]*      %{ common(yylval, yytext, yyleng, yyextra, FALSE, LSL_COMMENT_LINE); %}

 /* Operations. */
"&&"            { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_BOOL_AND); }
"||"            { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_BOOL_OR); }
"|"             { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_BIT_OR); }
"^"             { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_BIT_XOR); }
"&"             { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_BIT_AND); }
"!="            { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_NOT_EQUAL); }
"=="            { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_EQUAL); }
">="            { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_GREATER_EQUAL); }
"<="            { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_LESS_EQUAL); }
">"             { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_GREATER_THAN); }
"<"             { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_LESS_THAN); }
">>"            { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_RIGHT_SHIFT); }
"<<"            { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_LEFT_SHIFT); }
"+"             { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_ADD); }
"-"             { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_SUBTRACT); }
"*"             { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_MULTIPLY); }
"%"             { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_MODULO); }
"/"             { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_DIVIDE); }
"!"             { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_BOOL_NOT); }
"~"             { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_BIT_NOT); }
"["             { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_BRACKET_OPEN); }
"]"             { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_BRACKET_CLOSE); }
"("             { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_PARENTHESIS_OPEN); }
")"             { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_PARENTHESIS_CLOSE); }
"+="            { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_ASSIGNMENT_ADD); }
"-="            { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_ASSIGNMENT_SUBTRACT); }
"*="            { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_ASSIGNMENT_MULTIPLY); }
"%="            { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_ASSIGNMENT_MODULO); }
"/="            { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_ASSIGNMENT_DIVIDE); }
"="             { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_ASSIGNMENT_PLAIN); }
"."             { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_DOT); }
"--"            { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_DECREMENT_PRE); }
"++"            { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_INCREMENT_PRE); }
","             { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_COMMA); }

 /* Other symbols. */
"@"             %{ return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_LABEL); %}
"{"             %{ return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_BLOCK_OPEN); %}
"}"             %{ return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_BLOCK_CLOSE); %}
";"             %{ return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_STATEMENT); %}

 /* Type keywords. */
"float"         %{ return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_TYPE_FLOAT); %}
"integer"       %{ return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_TYPE_INTEGER); %}
"key"           %{ return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_TYPE_KEY); %}
"list"          %{ return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_TYPE_LIST); %}
"quaternion"    %{ return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_TYPE_ROTATION); %}
"rotation"      %{ return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_TYPE_ROTATION); %}
"string"        %{ return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_TYPE_STRING); %}
"vector"        %{ return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_TYPE_VECTOR); %}

 /* Statement keywords. */
"default"       %{ yylval->value.stringValue = eina_stringshare_add_length(yytext, yyleng); return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_DEFAULT); %}
"do"            %{ return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_DO); %}
"for"           %{ return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_FOR); %}
"else"          %{ return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_ELSE); %}
"if"            %{ return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_IF); %}
"jump"          %{ return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_JUMP); %}
"return"        %{ return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_RETURN); %}
"state"         %{ return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_STATE_CHANGE); %}
"while"         %{ return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_WHILE); %}

{IDENTIFIER}    %{ yylval->value.stringValue = eina_stringshare_add_length(yytext, yyleng); return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_IDENTIFIER); %}

 /* Types. */
{INTEGER}       %{ yylval->value.stringValue = eina_stringshare_add_length(yytext, yyleng);  return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_INTEGER); %}
{FLOAT}         %{ yylval->value.stringValue = eina_stringshare_add_length(yytext, yyleng);  return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_FLOAT); %}
{KEY}           %{ yylval->value.stringValue = eina_stringshare_add_length(yytext, yyleng); return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_KEY); %}
{STRING}        %{ yylval->value.stringValue = eina_stringshare_add_length(yytext, yyleng); return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_STRING); %}

<<EOF>>         { return common(yylval, yytext, yyleng, yyextra, TRUE, LSL_SCRIPT); }

 /* Everything else */
.               %{ printf(" unexpected character.\n"); yylval->value.stringValue = eina_stringshare_add_length(yytext, yyleng); common(yylval, yytext, yyleng, yyextra, TRUE, LSL_UNKNOWN); %}

%%

int common(YYSTYPE *lval, char *text, int len, LuaSL_compiler *compiler, boolean checkIgnorable, int type)
{
    char *p;

    lval->toKen = tokens[type - lowestToken];
    lval->line = compiler->line;
    lval->column = compiler->column;
    lval->len = len;

    for (p = text; *p; p++)
    {
	if ('\n' == *p)
	{
	    compiler->line++;
	    compiler->column = 1;
	}
	else
	    compiler->column++;
    }

#if LUASL_DIFF_CHECK
    if (checkIgnorable)
    {
	lval->ignorable = compiler->ignorable;
	compiler->ignorable = eina_strbuf_new();
    }
    else
	eina_strbuf_append_length(compiler->ignorable, text, len);
#endif

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

