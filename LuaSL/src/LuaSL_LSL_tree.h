
#ifndef __LUASL_TREE_H__
#define __LUASL_TREE_H__

//#define LUASL_DEBUG


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

typedef enum
{
    OM_LSL,
    OM_LUA
} outputMode;

typedef void (*outputToken) (FILE *file, outputMode mode, LSL_Leaf *content);
typedef LSL_Leaf * (*evaluateToken) (LSL_Leaf  *content, LSL_Leaf *left, LSL_Leaf *right);

//#ifndef FALSE
//typedef enum
//{
//    FALSE	= 0, 
//    TRUE	= 1
//} boolean;
//#endif

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
    const char	*name;
    int         subTypes;
};

struct _LSL_Token
{
    LSL_Type		type;
    opSubType		subType;
    const char 		*token;
    LSL_Flags		flags;
    outputToken		output;
    evaluateToken	evaluate;
};

struct _LSL_Leaf
{
    LSL_Leaf		*left;
    LSL_Leaf		*right;
    LSL_Token		*token;
    Eina_Strbuf		*ignorableText;
    int 		line, column, len;
    opType		basicType;
    union
    {
	float		floatValue;
	float		vectorValue[3];
	float		rotationValue[4];
	int		integerValue;
	LSL_Leaf	*listValue;
	const char	*stringValue;
	opType		operationValue;
	LSL_Parenthesis *parenthesis;
	LSL_Identifier	*identifierValue;
	LSL_Statement	*statementValue;
	LSL_Block	*blockValue;
	LSL_Function	*functionValue;
	LSL_State	*stateValue;
	LSL_Script	*scriptValue;
    } value;
};

struct _LSL_Parenthesis
{
    LSL_Leaf		*left;
    LSL_Leaf		*contents;
    LSL_Leaf		*right;
    LSL_Type		type;
};

struct _LSL_Identifier	// For variables and function parameters.
{
    const char		*name;
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
//    Eina_Hash		*statements;	// Probably should be some sort of eina list.
    Eina_Hash		*variables;	// Those variables in this scope.
    int			scount;
};

struct _LSL_Function
{
    const char	*name;
    LSL_Leaf	*type;
    LSL_Leaf	*params;
    LSL_Leaf	*block;
};

struct _LSL_State
{
    const char		*name;
    LSL_Leaf		*block;
    Eina_Hash		*handlers;
};

struct _LSL_Script
{
    const char		*name;
    Eina_Hash		*functions;
    Eina_Hash		*states;
    Eina_Hash		*variables;
};

// Define the type for flex and lemon.
#define YYSTYPE LSL_Leaf

typedef struct
{
    gameGlobals *game;
    void	*scanner;	// This should be of type yyscan_t, which is typedef to void * anyway, but that does not get defined until LuaSL_lexer.h, which depends on this struct being defined first.
    int		argc;
    char	**argv;
    char	fileName[PATH_MAX];
    FILE	*file;
    LSL_Leaf	*ast;
    LSL_Script	script;
    Eina_Strbuf	*ignorableText;
    LSL_Leaf	*lval;
    int		column, line;
    LSL_Block	*currentBlock;
} LuaSL_compiler;


#ifndef excludeLexer
    #include "LuaSL_lexer.h"
#endif


void burnLeaf(void *data);
LSL_Leaf *addFunction(LuaSL_compiler *compiler, LSL_Leaf *type, LSL_Leaf *identifier, LSL_Leaf *open, LSL_Leaf *params, LSL_Leaf *close, LSL_Leaf *block);
LSL_Leaf *addOperation(LuaSL_compiler *compiler, LSL_Leaf *left, LSL_Leaf *lval, LSL_Leaf *right);
LSL_Leaf *addParameter(LSL_Leaf *type, LSL_Leaf *newParam);
LSL_Leaf *addParenthesis(LSL_Leaf *lval, LSL_Leaf *expr, LSL_Type type, LSL_Leaf *rval);
LSL_Leaf *addState(LuaSL_compiler *compiler, LSL_Leaf *identifier, LSL_Leaf *block);
LSL_Leaf *addStatement(LSL_Leaf *lval, LSL_Type type, LSL_Leaf *expr);
LSL_Leaf *addTypecast(LSL_Leaf *lval, LSL_Leaf *type, LSL_Leaf *rval, LSL_Leaf *expr);
LSL_Leaf *addVariable(LuaSL_compiler *compiler, LSL_Leaf *type, LSL_Leaf *identifier, LSL_Leaf *assignment, LSL_Leaf *expr);

void beginBlock(LuaSL_compiler *compiler, LSL_Leaf *block);
LSL_Leaf *collectParameters(LSL_Leaf *list, LSL_Leaf *comma, LSL_Leaf *newParam);
void endBlock(LuaSL_compiler *compiler, LSL_Leaf *block);

void *ParseAlloc(void *(*mallocProc)(size_t));
void ParseTrace(FILE *TraceFILE, char *zTracePrompt);
void Parse(void *yyp, int yymajor, LSL_Leaf *yyminor, LuaSL_compiler *compiler);
void ParseFree(void *p, void (*freeProc)(void*));


#endif // __LUASL_LSL_TREE_H__

