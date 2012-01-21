
#ifndef __LUASL_TREE_H__
#define __LUASL_TREE_H__

//#define LUASL_DEBUG
//#define LUASL_DIFF_CHECK


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
#ifdef LUASL_DIFF_CHECK
    Eina_Strbuf		*ignorableText;
#endif
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
    LSL_Leaf		*contents;
#ifdef LUASL_DIFF_CHECK
    Eina_Strbuf		*rightIgnorableText;
#endif
    LSL_Type		type;
};

struct _LSL_Identifier	// For variables and function parameters.
{
    const char		*name;
    LSL_Leaf		value;
};

struct _LSL_Statement
{
    Eina_Clist		statement;
    LSL_Leaf		*expressions;	// For things like a for statement, might hold three expressions.
    LSL_Type		type;		// Expression type.
};

struct _LSL_Block
{
    LSL_Block		*outerBlock;
    Eina_Clist		statements;
    Eina_Hash		*variables;	// Those variables in this scope.
    LSL_Function	*function;	// A pointer to the function if this block is a function.
};

struct _LSL_Function
{
    const char	*name;
    LSL_Leaf	*type;
#ifdef LUASL_DIFF_CHECK
    LSL_Leaf	*params;	// So we store the parenthesis, and their ignorables.
#endif
    Eina_Inarray vars;		// Eina Inarray has not been released yet (Eina 1.2).
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

/* Tracking variables.

There are global variables, block local variables, and function parameters.

For outputting Lua, which is the ultimate goal -
    track order, name, and type.

For looking them up during the compile -
    quick access from name.

For validating them during compile -
    track type.

For outputting LSL to double check -
    track order, name, type, and white space.

For executing directly from the AST -
    track order, name, type, and value.
    In this case, order is only important for functions.

We can assume that names are stringshared.  This means we only have to
compare pointers.  It also means the same name stored at diffferent
scopes, must be stored in separate structures, coz the pointers are the
same.

Order is taken care of by the AST anyway, but for somethings we want to
condense the AST down to something more efficient.

On the other hand, no need to micro optimise it just yet, we should be
able to try out other data structures at a later date, then benchmark
them with typical scripts.

Right now I see nothing wrong with the current use of hash for script
and block variables.  The same for script states and functions, as well
as state functions. Though in the near future, they will have similar
problems to functions I think - the need to track order and white
space.

Function params got unwieldy.  Cleaned that up now.

*/

/* General design.

NOTE We can remove the white space tracking at compile time, as it's
only a debugging aid.  Will be a performance and memory gain for
productidon use. Tracking values on the other hand will still be useful
for constants.

The compile process starts with turning tokens into AST nodes connected
in a tree.  During that process the parser wants to condense nodes down
to more efficient data structures.  This is a good idea, as we will
spend a fair amount of time looking up names, no matter which part of
the process is important at the time.

Once the parser has condensed things down, it only deals with the
condensed nodes.  So we can get rid of some of the AST parts at this
time, so long as we keep the relevant information.  This is what the
other data structures above are for.  Lemon tries to free the no longer
needed AST nodes itself, even if we are still using them internally. 
Need to do something about that.

*/

// Define the type for flex and lemon.
#define YYSTYPE LSL_Leaf

typedef struct
{
    gameGlobals		*game;
    void		*scanner;	// This should be of type yyscan_t, which is typedef to void * anyway, but that does not get defined until LuaSL_lexer.h, which depends on this struct being defined first.
    int			argc;
    char		**argv;
    char		fileName[PATH_MAX];
    FILE		*file;
    LSL_Leaf		*ast;
    LSL_Script		script;
#ifdef LUASL_DIFF_CHECK
    Eina_Strbuf		*ignorableText;
#endif
    LSL_Leaf		*lval;
    int			column, line;
    LSL_Block		*currentBlock;
} LuaSL_compiler;


#ifndef excludeLexer
    #include "LuaSL_lexer.h"
#endif


void burnLeaf(void *data);
LSL_Leaf *addFunction(LuaSL_compiler *compiler, LSL_Leaf *type, LSL_Leaf *identifier, LSL_Leaf *open, LSL_Leaf *params, LSL_Leaf *close);
LSL_Leaf *addFunctionBody(LuaSL_compiler *compiler, LSL_Leaf *function, LSL_Leaf *block);
LSL_Leaf *addOperation(LuaSL_compiler *compiler, LSL_Leaf *left, LSL_Leaf *lval, LSL_Leaf *right);
LSL_Leaf *addParameter(LuaSL_compiler *compiler, LSL_Leaf *type, LSL_Leaf *newParam);
LSL_Leaf *addParenthesis(LSL_Leaf *lval, LSL_Leaf *expr, LSL_Type type, LSL_Leaf *rval);
LSL_Leaf *addState(LuaSL_compiler *compiler, LSL_Leaf *identifier, LSL_Leaf *block);
LSL_Leaf *addStatement(LSL_Leaf *lval, LSL_Type type, LSL_Leaf *expr);
LSL_Leaf *addTypecast(LSL_Leaf *lval, LSL_Leaf *type, LSL_Leaf *rval, LSL_Leaf *expr);
LSL_Leaf *addVariable(LuaSL_compiler *compiler, LSL_Leaf *type, LSL_Leaf *identifier, LSL_Leaf *assignment, LSL_Leaf *expr);

void beginBlock(LuaSL_compiler *compiler, LSL_Leaf *block);
LSL_Leaf *checkVariable(LuaSL_compiler *compiler, LSL_Leaf *identifier);
LSL_Leaf *collectParameters(LuaSL_compiler *compiler, LSL_Leaf *list, LSL_Leaf *comma, LSL_Leaf *newParam);
LSL_Leaf *collectStatements(LuaSL_compiler *compiler, LSL_Leaf *list, LSL_Leaf *newStatement);
void endBlock(LuaSL_compiler *compiler, LSL_Leaf *block);

void *ParseAlloc(void *(*mallocProc)(size_t));
void ParseTrace(FILE *TraceFILE, char *zTracePrompt);
void Parse(void *yyp, int yymajor, LSL_Leaf *yyminor, LuaSL_compiler *compiler);
void ParseFree(void *p, void (*freeProc)(void*));


#endif // __LUASL_LSL_TREE_H__

