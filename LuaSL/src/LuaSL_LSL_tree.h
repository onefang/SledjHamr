
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

#ifndef FALSE
// NEVER change this
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

typedef int LSL_Type;

typedef struct
{
    LSL_Type			type;
    struct LSL_AST		*expression;
} LSL_Statement;

typedef struct
{
    LSL_Statement		*statements;
} LSL_Block;

typedef struct
{
    char			*name;
    struct LSL_Identifier	*parameters;
    LSL_Block			block;
    LSL_Type			type;
} LSL_Function;

typedef struct
{
    char			*name;
    LSL_Function		*handlers;
} LSL_State;

typedef struct
{
    char			*name;
    struct LSL_Identifier	*variables;
    LSL_Function		*functions;
    LSL_State			*states;
} LSL_Script;

typedef union LSL_Leaf
{
    char			*spaceValue;
    char			*commentValue;
    LSL_Type			typeValue;
    char			*nameValue;
    struct LSL_Identifier	*identifierValue;
    float			floatValue;
    int				integerValue;
    char			*stringValue;
    char			*keyValue;
    float			vectorValue[3];
    float			rotationValue[4];
    union LSL_Leaf		*listValue;
    char			*labelValue;
    LSL_Type			operationValue;
    struct LSL_AST		*expressionValue;
    LSL_Statement		*doValue;
    LSL_Statement		*forValue;
    LSL_Statement		*ifValue;
    LSL_Statement		*elseValue;
    LSL_Statement		*elseIfValue;
    char			*jumpValue;
    char			*stateChangeValue;
    LSL_Statement		*statementValue;
    struct LSL_Identifier	*parameterValue;
    LSL_Function		*functionValue;
    LSL_State			*stateValue;
    LSL_Script			*scriptValue;
    char			*unknownValue;
} LSL_Leaf;

typedef struct
{
    LSL_Type		type;
    LSL_Leaf		content;
} LSL_Value;

typedef void (*convertToken2Lua) (LSL_Leaf *content);
typedef void (*outputToken) (LSL_Leaf *content);
typedef void (*evaluateToken) (LSL_Leaf  *content, LSL_Value *left, LSL_Value *right);

typedef struct
{
    LSL_Type		type;
    char 		*token;
    LSL_Flags		flags;
    outputToken		output;
    convertToken2Lua	convert;
    evaluateToken	evaluate;
} LSL_Token;

typedef struct
{
    char		*name;
    LSL_Type		type;
    LSL_Leaf		content;
} LSL_Identifier;

typedef struct LSL_AST
{
    struct LSL_AST	*left;
    struct LSL_AST	*right;
    int			line;
    int			character;
    LSL_Token		*token;
    LSL_Leaf		content;
} LSL_AST;

 
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
// data is a pointer to a 'SParserParam' structure
#define YYPARSE_PARAM data
 
// the argument for the 'yylex' function
#define YYLEX_PARAM   ((LuaSL_yyparseParam*)data)->scanner


LSL_AST *addExpression(LSL_AST *exp);
LSL_AST *addInteger(int value);
LSL_AST *addOperation(LSL_Type type, LSL_AST *left, LSL_AST *right);
LSL_AST *addParenthesis(LSL_AST *expr);
LSL_Statement *createStatement(LSL_Type type, LSL_AST *root);
LSL_AST *addStatement(LSL_Statement *statement, LSL_AST *root);
LSL_AST *addSpace(char *text, LSL_AST *root);

int yyerror(const char *msg);
int yyparse(void *param);


#endif // __EXPRESSION_H__

