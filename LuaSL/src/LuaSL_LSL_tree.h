
#ifndef __LSL_TREE_H__
#define __LSL_TREE_H__

#define LUASL_DEBUG
//#define LUASL_FILES


#include <stddef.h>	// So we can have NULL defined.
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "assert.h"  
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>	// For PATH_MAX.

#include "LuaSL_lemon_yaccer.h"
                    
#define YYERRCODE 256
#define YYDEBUG 1
extern int yydebug;


// http://w-hat.com/stackdepth is a useful discussion about some aspects of the LL parser.


typedef struct _allowedTypes	allowedTypes;
typedef struct _LSL_Token	LSL_Token;
typedef struct _LSL_Leaf	LSL_Leaf;
typedef struct _LSL_Parenthesis LSL_Parenthesis;
typedef struct _LSL_Identifier	LSL_Identifier;
typedef struct _LSL_Statement	LSL_Statement;
typedef struct _LSL_Block	LSL_Block;
typedef struct _LSL_Function	LSL_Function;
typedef struct _LSL_State	LSL_State;
typedef struct _LSL_Script	LSL_Script;

extern LSL_Token **tokens;
extern int lowestToken;

typedef int LSL_Type;

typedef void (*convertToken2Lua) (FILE *file, LSL_Leaf *content);
typedef void (*outputToken) (FILE *file, LSL_Leaf *content);
typedef LSL_Leaf * (*evaluateToken) (LSL_Leaf  *content, LSL_Leaf *left, LSL_Leaf *right);

#ifndef FALSE
typedef enum
{
    FALSE	= 0, 
    TRUE	= 1
} boolean;
#endif

typedef enum
{
    LSL_NONE		= 0,
    LSL_LEFT2RIGHT	= 1,
    LSL_RIGHT2LEFT	= 2,
    LSL_INNER2OUTER	= 4,
    LSL_UNARY		= 8,
    LSL_ASSIGNMENT	= 16,
    LSL_CREATION	= 32,
    LSL_NOIGNORE	= 64
} LSL_Flags;

typedef enum
{
    OT_nothing,

    OT_bool,
    OT_integer,
    OT_float,
    OT_key,
    OT_list,
    OT_rotation,
    OT_string,
    OT_vector,
    OT_other,

    OT_boolBool,
    OT_intInt,
    OT_intFloat,
    OT_floatInt,
    OT_floatFloat,
    OT_keyString,
    OT_stringKey,
    OT_stringString,
    OT_listList,
    OT_listInt,
    OT_listFloat,
    OT_intList,
    OT_floatList,
    OT_listOther,
    OT_vectorVector,
    OT_vectorFloat,
    OT_vectorRotation,
    OT_rotationRotation,
    OT_otherOther,
    OT_invalid
} opType;

/*
Each op is of a specific type -

bool				!
int				- ~
float				-

bool		bool		&& ||     == !=           =
int		int		* / + - % == != < > <= >= = += -= *= /= %= & | ^ << >>
int		float		cast to float float
float		int		cast to float float
float		float		* / + -   == != < > <= >= = += -= *= /= 

key		string		cast to string string
string		key		cast to string string
string		string		+         == !=           = +=

list		list		+         == !=           = +=
list		integer/float	+               < > <= >= = +=
integer/float	list		+               < > <= >=
list		other		+                         = +=

vector		vector		* / + - % == !=           = += -= *= /= %=
vector		float		* /
vector		rotation	* /

rotation	rotation	* / + -   == !=           = += -= *= /= 
*/

typedef enum
{
    ST_NONE		= 0,
    ST_ASSIGNMENT	= 1,	// -= *= /=
    ST_BIT_NOT		= 2,	// ~
    ST_BOOL_NOT		= 4,	// !
    ST_BITWISE		= 8,	// & | ^ << >>
    ST_BOOLEAN		= 16,	// && !!
    ST_COMPARISON	= 32,	// < > <= >=
    ST_CONCATENATION	= 64,	// = +=
    ST_EQUALITY		= 128,	// == !=
    ST_ADD		= 512,	// +
    ST_SUBTRACT		= 1024,	// -
    ST_NEGATE		= 2048,	// -
    ST_MULTIPLY		= 4096,	// * /
    ST_MODULO		= 8192	// % %=
} opSubType;

struct _allowedTypes
{
    opType	result;
    int         subTypes;
};

struct _LSL_Token
{
    LSL_Type		type;
    opSubType		subType;
    char 		*token;
    LSL_Flags		flags;
    outputToken		output;
    convertToken2Lua	convert;
    evaluateToken	evaluate;
};

struct _LSL_Leaf
{
    LSL_Leaf		*left;
    LSL_Leaf		*right;
    LSL_Token		*token;
    char		*ignorableText;
    int 		line, column;
    opType		basicType;
    union
    {
	opType		operationValue;
	LSL_Parenthesis *parenthesis;

	float		floatValue;
	int		integerValue;
	char		*keyValue;
	LSL_Leaf	*listValue;
	char		*stringValue;
	float		vectorValue[3];
	float		rotationValue[4];

	LSL_Identifier	*identifierValue;
	LSL_Identifier	*variableValue;

	char		*labelValue;
	LSL_Statement	*doValue;
	LSL_Statement	*forValue;
	LSL_Statement	*elseIfValue;
	LSL_Statement	*elseValue;
	LSL_Statement	*ifValue;
	char		*jumpValue;
	LSL_Statement	*returnValue;
	char		*stateChangeValue;
	LSL_Statement	*whileValue;
	LSL_Statement	*statementValue;

	LSL_Block	*blockValue;
	LSL_Identifier	*parameterValue;
	LSL_Function	*functionValue;
	LSL_State	*stateValue;
	LSL_Script	*scriptValue;

	char		*unknownValue;
    } value;
};

struct _LSL_Parenthesis
{
    LSL_Leaf		*left;
    LSL_Leaf		*expression;
    LSL_Leaf		*right;
};

struct _LSL_Identifier	// For variables and function parameters.
{
    char		*name;
    LSL_Leaf		value;
};

struct _LSL_Statement
{
    LSL_Leaf		*expressions;	// For things like a for statement, might hold three expressions.
    LSL_Type		type;		// Expression type.
};

struct _LSL_Block
{
    LSL_Block		*outerBlock;
    LSL_Statement	*statements;
    LSL_Identifier	*scopeVariables;
};

struct _LSL_Function
{
    char		*name;
    LSL_Block		block;
    LSL_Identifier	*parameters;
    LSL_Type		type;	// Return type.
};

struct _LSL_State
{
    char		*name;
    LSL_Function	*handlers;
};

struct _LSL_Script
{
    char		*name;
    LSL_Function	*functions;
    LSL_State		*states;
    LSL_Identifier	*variables;
};

typedef struct
{
    char *ignorableText;
    int column;
    int line;
    int argc;
    char **argv;
    char *fileName;
    FILE *file;
} LuaSL_yyparseExtra;


// define the type for flex and lemon3
#define YYSTYPE LSL_Leaf


#ifndef excludeLexer
    #include "LuaSL_lexer.h"
#endif

typedef struct
{
        yyscan_t scanner;
        LSL_Leaf *ast;
        LSL_Leaf *lval;
} LuaSL_yyparseParam;

// the parameter name (of the reentrant 'yyparse' function)
// data is a pointer to a 'yyparseParam' structure
//#define YYPARSE_PARAM data
 
// the argument for the 'yylex' function
#define YYLEX_PARAM   ((LuaSL_yyparseParam*)data)->scanner
//#define ParseTOKENTYPE YYSTYPE *
//#define ParseARG_PDECL , LuaSL_yyparseParam *param

void burnLeaf(LSL_Leaf *leaf);
int nextFile(LuaSL_yyparseExtra *extra);
LSL_Leaf *addExpression(LSL_Leaf *exp);
LSL_Leaf *addOperation(LSL_Leaf *left, LSL_Leaf *lval, LSL_Leaf *right);
LSL_Leaf *addParenthesis(LSL_Leaf *lval, LSL_Leaf *expr, LSL_Leaf *rval);
LSL_Leaf *addStatement(LSL_Leaf *lval, LSL_Type type, LSL_Leaf *expr);

void *ParseAlloc(void *(*mallocProc)(size_t));
void ParseTrace(FILE *TraceFILE, char *zTracePrompt);
void Parse(void *yyp, int yymajor, LSL_Leaf *yyminor, LuaSL_yyparseParam *param);
void ParseFree(void *p, void (*freeProc)(void*));


#endif // __LSL_TREE_H__

