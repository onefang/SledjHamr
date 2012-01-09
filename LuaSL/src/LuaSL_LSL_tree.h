
#ifndef __EXPRESSION_H__
#define __EXPRESSION_H__

#define LUASL_DEBUG


#include <stddef.h>	// So we can have NULL defined.
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "LuaSL_yaccer.tab.h"
                    
#define YYERRCODE 256
#define YYDEBUG 1
extern int yydebug;


// http://w-hat.com/stackdepth is a useful discussion about some aspects of the LL parser.


typedef struct _LSL_Token	LSL_Token;
typedef struct _LSL_Leaf	LSL_Leaf;
typedef struct _LSL_Identifier	LSL_Identifier;
typedef struct _LSL_Statement	LSL_Statement;
typedef struct _LSL_Block	LSL_Block;
typedef struct _LSL_Function	LSL_Function;
typedef struct _LSL_State	LSL_State;
typedef struct _LSL_Script	LSL_Script;
typedef struct _LSL_AST		LSL_AST;

extern LSL_Token **tokens;
extern int lowestToken;

typedef int LSL_Type;

typedef void (*convertToken2Lua) (LSL_Leaf *content);
typedef void (*outputToken) (LSL_Leaf *content);
typedef void (*evaluateToken) (LSL_Leaf  *content, LSL_Leaf *left, LSL_Leaf *right);

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
    LSL_CREATION	= 32
} LSL_Flags;

struct _LSL_Token
{
    LSL_Type		type;
    char 		*token;
    LSL_Flags		flags;
    outputToken		output;
    convertToken2Lua	convert;
    evaluateToken	evaluate;
};

struct _LSL_Leaf
{
    union
    {
	char		*commentValue;
	char		*spaceValue;

	LSL_Type	operationValue;
	LSL_AST		*expressionValue;

	float		floatValue;
	int		integerValue;
	char		*keyValue;
	LSL_Leaf	*listValue;
	char		*stringValue;
	float		vectorValue[3];
	float		rotationValue[4];

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
    char		*ignorableText;
    LSL_Token		*token;
    LSL_Type		type;
    int 		line, column;
};

struct _LSL_Identifier	// For variables and function parameters.
{
    char		*name;
    LSL_Leaf		value;
};

struct _LSL_Statement
{
    LSL_AST		*expressions;	/// For things like a for statement, might hold three expressions.
};

struct _LSL_Block
{
    LSL_Statement	*statements;
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

struct _LSL_AST
{
    LSL_AST		*left;
    LSL_AST		*right;
    LSL_Leaf		content;
};


// define the type for flex and bison
#define YYSTYPE LSL_Leaf


#ifndef excludeLexer
    #include "LuaSL_lexer.h"
#endif


typedef struct
{
        yyscan_t scanner;
        LSL_AST *ast;
} LuaSL_yyparseParam;

// the parameter name (of the reentrant 'yyparse' function)
// data is a pointer to a 'yyparseParam' structure
#define YYPARSE_PARAM data
 
// the argument for the 'yylex' function
#define YYLEX_PARAM   ((LuaSL_yyparseParam*)data)->scanner


LSL_AST *addExpression(LSL_AST *exp);
LSL_AST *addInteger(LSL_Leaf *lval, int value);
LSL_AST *addOperation(LSL_Leaf *lval, LSL_Type type, LSL_AST *left, LSL_AST *right);
LSL_AST *addParenthesis(LSL_Leaf *lval, LSL_AST *expr);
LSL_Statement *createStatement(LSL_Type type, LSL_AST *root);
LSL_AST *addStatement(LSL_Statement *statement, LSL_AST *root);

int yyerror(const char *msg);
int yyparse(void *param);


#endif // __EXPRESSION_H__

