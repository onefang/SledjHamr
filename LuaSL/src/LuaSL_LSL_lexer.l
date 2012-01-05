
N			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{N}+
FS			(f|F)
%e 10000
%n 4000
%p 5000

%{

#define excludeLexer
#include "LuaSL_LSL_tree.h"
#include "LuaSL_LSLS_yaccer.tab.h"


// Deal with the fact that lex/yacc generates unreachable code
#ifdef LL_WINDOWS
#pragma warning (disable : 4018) // warning C4018: signed/unsigned mismatch
#pragma warning (disable : 4702) // warning C4702: unreachable code
#endif	//	LL_WINDOWS

void count();
void line_comment();
void block_comment();
void parse_string();

#define YYLMAX 16384
#define YY_NEVER_INTERACTIVE 1 /* stops flex from calling isatty() */
#ifdef LL_WINDOWS
#define isatty(x) 0 /* hack for bug in cygwin flex 2.5.35 */
#endif

#ifdef ECHO
#undef ECHO
#endif

#define ECHO do { } while (0)

%}

%option reentrant noyywrap never-interactive nounistd
%option bison-bridge

%%
"//"					{ gInternalLine++; gInternalColumn = 0; line_comment(); }
"/*"					{ block_comment(); }

"integer"			{ count(); return(INTEGER); }
"float"				{ count(); return(FLOAT_TYPE); }
"string"			{ count(); return(STRING); }
"key"				{ count(); return(LLKEY); }
"vector"			{ count(); return(VECTOR); }
"quaternion"		{ count(); return(QUATERNION); }
"rotation"			{ count(); return(QUATERNION); }
"list"				{ count(); return(LIST); }

"default"			{ count(); yylval.sval = new char[strlen(yytext) + 1]; strcpy(yylval.sval, yytext); return(STATE_DEFAULT); }
"state"				{ count(); return(STATE); }
"event"				{ count(); return(EVENT); }
"jump"				{ count(); return(JUMP); }
"return"			{ count(); return(RETURN); }
"if"				{ count(); return(IF); }
"else"				{ count(); return(ELSE); }
"for"				{ count(); return(FOR); }
"do"				{ count(); return(DO); }
"while"				{ count(); return(WHILE); }

"state_entry"			{ count(); return(STATE_ENTRY); }
"state_exit"			{ count(); return(STATE_EXIT); }
"touch_start"			{ count(); return(TOUCH_START); }
"touch"					{ count(); return(TOUCH); }
"touch_end"				{ count(); return(TOUCH_END); }
"collision_start"		{ count(); return(COLLISION_START); }
"collision"				{ count(); return(COLLISION); }
"collision_end"			{ count(); return(COLLISION_END); }
"land_collision_start"	{ count(); return(LAND_COLLISION_START); }
"land_collision"		{ count(); return(LAND_COLLISION); }
"land_collision_end"	{ count(); return(LAND_COLLISION_END); }
"timer"					{ count(); return(TIMER); }
"listen"				{ count(); return(CHAT); }
"sensor"				{ count(); return(SENSOR); }
"no_sensor"				{ count(); return(NO_SENSOR); }
"control"				{ count(); return(CONTROL); }
"print"					{ count(); return(PRINT); }
"at_target"				{ count(); return(AT_TARGET); }
"not_at_target"			{ count(); return(NOT_AT_TARGET); }
"at_rot_target"			{ count(); return(AT_ROT_TARGET); }
"not_at_rot_target"		{ count(); return(NOT_AT_ROT_TARGET); }
"money"					{ count(); return(MONEY); }
"email"					{ count(); return(EMAIL); }
"run_time_permissions"	{ count(); return(RUN_TIME_PERMISSIONS); }
"changed"				{ count(); return(INVENTORY); }
"attach"				{ count(); return(ATTACH); }
"dataserver"			{ count(); return(DATASERVER); }
"moving_start"			{ count(); return(MOVING_START); }
"moving_end"			{ count(); return(MOVING_END); }
"link_message"			{ count(); return(LINK_MESSAGE); }
"on_rez"				{ count(); return(REZ); }
"object_rez"			{ count(); return(OBJECT_REZ); }
"remote_data"			{ count(); return(REMOTE_DATA); }
"http_response"			{ count(); return(HTTP_RESPONSE); }
"http_request"			{ count(); return(HTTP_REQUEST); }
"."						{ count(); return(PERIOD); }

0[xX]{H}+			{ count(); yylval.ival = strtoul(yytext, NULL, 0);  return(INTEGER_CONSTANT); }
{N}+				{ count(); yylval.ival = strtoul(yytext, NULL, 10); return(INTEGER_CONSTANT); }
"TRUE"				{ count(); yylval.ival = 1; return(INTEGER_TRUE); }
"FALSE"				{ count(); yylval.ival = 0; return(INTEGER_FALSE); }

"TOUCH_INVALID_FACE"	  { count(); yylval.ival = -1; return(INTEGER_CONSTANT); }
"TOUCH_INVALID_VECTOR"	  { count(); return(TOUCH_INVALID_VECTOR); }
"TOUCH_INVALID_TEXCOORD"  { count(); return(TOUCH_INVALID_TEXCOORD); }

{L}({L}|{N})*		{ count(); yylval.sval = new char[strlen(yytext) + 1]; strcpy(yylval.sval, yytext); return(IDENTIFIER); }

{N}+{E}					{ count(); yylval.fval = (F32)atof(yytext); return(FP_CONSTANT); }
{N}*"."{N}+({E})?{FS}?	{ count(); yylval.fval = (F32)atof(yytext); return(FP_CONSTANT); }
{N}+"."{N}*({E})?{FS}?	{ count(); yylval.fval = (F32)atof(yytext); return(FP_CONSTANT); }

L?\"(\\.|[^\\"])*\"	{ parse_string(); count(); return(STRING_CONSTANT); }

"++"				{ count(); return(INC_OP); }
"--"				{ count(); return(DEC_OP); }
"+="				{ count(); return(ADD_ASSIGN); }
"-="				{ count(); return(SUB_ASSIGN); }
"*="				{ count(); return(MUL_ASSIGN); }
"/="				{ count(); return(DIV_ASSIGN); }
"%="				{ count(); return(MOD_ASSIGN); }
";"					{ count(); return(';'); }
"{"					{ count(); return('{'); }
"}"					{ count(); return('}'); }
","					{ count(); return(','); }
"="					{ count(); return('='); }
"("					{ count(); return('('); }
")"					{ count(); return(')'); }
"-"					{ count(); return('-'); }
"+"					{ count(); return('+'); }
"*"					{ count(); return('*'); }
"/"					{ count(); return('/'); }
"%"					{ count(); return('%'); }
"@"					{ count(); return('@'); }
":"					{ count(); return(':'); }
">"					{ count(); return('>'); }
"<"					{ count(); return('<'); }
"]"					{ count(); return(']'); }
"["					{ count(); return('['); }
"=="				{ count(); return(EQ);  }
"!="				{ count(); return(NEQ);  }
">="				{ count(); return(GEQ);  }
"<="				{ count(); return(LEQ);  }
"&"					{ count(); return('&');  }
"|"					{ count(); return('|');  }
"^"					{ count(); return('^');  }
"~"					{ count(); return('~');  }
"!"					{ count(); return('!');  }
"&&"				{ count(); return(BOOLEAN_AND);	}
"||"				{ count(); return(BOOLEAN_OR);	}
"<<"				{ count(); return(SHIFT_LEFT);  }
">>"				{ count(); return(SHIFT_RIGHT); }

[ \t\v\n\f]			{ count(); }
.					{ /* ignore bad characters */ }

%%


// Prototype for the yacc parser entry point
int yyparse(void);


S32 yywrap()
{
#if defined(FLEX_SCANNER) && !defined(LL_WINDOWS)
	// get gcc to stop complaining about lack of use of yyunput
	(void) yyunput;
#endif
	return(1);
}

void line_comment()
{
	char c;

	while ((c = yyinput()) != '\n' && c != 0 && c != EOF)
		;
}

void block_comment()
{
	char c1 = 0;
	char c2 = yyinput();
	while (c2 != 0 && c2 != EOF && !(c1 == '*' && c2 == '/')) {
		if (c2 == '\n')
		{
			gInternalLine++;
			gInternalColumn = 0;
		}
		else if (c2 == '\t')
			gInternalColumn += 4 - (gInternalColumn % 8);
		else
			gInternalColumn++;
		c1 = c2;
		c2 = yyinput();
	}
}

void count()
{
	S32 i;

	gColumn = gInternalColumn;
	gLine = gInternalLine;

	for (i = 0; yytext[i] != '\0'; i++)
		if (yytext[i] == '\n')
		{
			gInternalLine++;
			gInternalColumn = 0;
		}
		else if (yytext[i] == '\t')
			gInternalColumn += 4 - (gInternalColumn % 8);
		else
			gInternalColumn++;
}

void parse_string()
{
	S32 length = (S32)strlen(yytext);
	length = length - 2; 
	char *temp = yytext + 1;

	S32 i;
	S32 escapes = 0;
	S32 tabs = 0;
	for (i = 0; i < length; i++)
	{
		if (temp[i] == '\\')
		{
			escapes++;
			i++;
			if (temp[i] == 't')
				tabs++;
		}
	}

	S32 newlength = length - escapes + tabs*3;
	yylval.sval = new char[newlength + 1];

	char *dest = yylval.sval;

	for (i = 0; i < length; i++)
	{
		if (temp[i] == '\\')
		{
			i++;
			// linefeed
			if (temp[i] == 'n')
			{
				*dest++ = 10;
			}
			else if (temp[i] == 't')
			{
				*dest++ = ' ';
				*dest++ = ' ';
				*dest++ = ' ';
				*dest++ = ' ';
			}
			else
			{
				*dest++ = temp[i];
			}
		}
		else
		{
			*dest++ = temp[i];
		}
	}
	yylval.sval[newlength] = 0;
}
