
#ifndef __EXPRESSION_H__
#define __EXPRESSION_H__

//#define LUASL_USE_ENUM
#define LUASL_DEBUG

#ifndef LUASL_USE_ENUM
#include "LuaSL_yaccer.tab.h"
#endif

#include <stddef.h>	// So we can have NULL defined.

#define YYERRCODE 256
#define YYDEBUG 1
extern int yydebug;


// http://w-hat.com/stackdepth is a useful discussion about some aspects of the LL parser.

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

#ifdef LUASL_USE_ENUM
typedef enum			// In order of precedence, high to low.
				// Left to right, unless oterwise stated.
				// According to http://wiki.secondlife.com/wiki/Category:LSL_Operators
{
    LSL_COMMA = 257,
    LSL_INCREMENT_PRE,		// Right to left.
    LSL_INCREMENT_POST,		// Right to left.
    LSL_DECREMENT_PRE,		// Right to left.
    LSL_DECREMENT_POST,		// Right to left.
    LSL_DOT,			// Right to left.
    LSL_ASSIGNMENT_PLAIN,	// Right to left.
    LSL_ASSIGNMENT_DIVIDE,	// Right to left.
    LSL_ASSIGNMENT_MODULO,	// Right to left.
    LSL_ASSIGNMENT_MULTIPLY,	// Right to left.
    LSL_ASSIGNMENT_SUBTRACT,	// Right to left.
    LSL_ASSIGNMENT_ADD,		// Right to left.
    LSL_ASSIGNMENT_CONCATENATE,	// Right to left.
    LSL_PARENTHESIS_OPEN,	// Inner to outer.
    LSL_PARENTHESIS_CLOSE,	// Inner to outer.
    LSL_BRACKET_OPEN,		// Inner to outer.
    LSL_BRACKET_CLOSE,		// Inner to outer.
    LSL_ANGLE_OPEN,
    LSL_ANGLE_CLOSE,
    LSL_TYPECAST,		// Right to left.
    LSL_BIT_NOT,		// Right to left.
    LSL_BOOL_NOT,		// Right to left.
    LSL_NEGATION,		// Right to left.
    LSL_DIVIDE,
    LSL_MODULO,
    LSL_MULTIPLY,
    LSL_DOT_PRODUCT,
    LSL_CROSS_PRODUCT,
    LSL_SUBTRACT,
    LSL_ADD,
    LSL_CONCATENATE,
    LSL_LEFT_SHIFT,
    LSL_RIGHT_SHIFT,
    LSL_LESS_THAN,
    LSL_GREATER_THAN,
    LSL_LESS_EQUAL,
    LSL_GREATER_EQUAL,
    LSL_EQUAL,
    LSL_NOT_EQUAL,
    LSL_BIT_AND,
    LSL_BIT_XOR,
    LSL_BIT_OR,
    LSL_BOOL_OR,
    LSL_BOOL_AND
} LSL_Operation;
#else
typedef int LSL_Operation;
#endif

#ifdef LUASL_USE_ENUM
typedef enum
{
    LSL_COMMENT = (LSL_BOOL_AND + 1),
    LSL_TYPE,
    LSL_NAME,
    LSL_IDENTIFIER,
    LSL_FLOAT,
    LSL_INTEGER,
    LSL_STRING,
    LSL_KEY,
    LSL_VECTOR,
    LSL_ROTATION,
    LSL_LIST,
    LSL_LABEL,
    LSL_EXPRESSION,
    LSL_DO,
    LSL_FOR,
    LSL_IF,
    LSL_ELSE,
    LSL_ELSEIF,
    LSL_JUMP,
    LSL_STATE_CHANGE,
    LSL_WHILE,
    LSL_RETURN,
    LSL_STATEMENT,
    LSL_BLOCK,
    LSL_PARAMETER,
    LSL_FUNCTION,
    LSL_STATE,
    LSL_SCRIPT
} LSL_Type;
#else
typedef int LSL_Type;
#endif

typedef struct
{
    LSL_Type			type;
    struct LSL_Expression	*expressions;
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
    LSL_Operation		operationValue;
    struct LSL_Expression	*expressionValue;
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
} LSL_Leaf;

typedef struct
{
    LSL_Type		type;
    LSL_Leaf		content;
} LSL_Value;

typedef void (*convertToken2Lua) (LSL_Leaf *content);
typedef void (*outputToken) (LSL_Leaf *content);
typedef void (*evaluateToken) (LSL_Leaf  *content, LSL_Value *result);

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

typedef struct LSL_Expression
{
    struct LSL_Expression	*left;
    struct LSL_Expression	*right;
    LSL_Token			*token;
    LSL_Leaf			content;
} LSL_Expression;

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


LSL_AST *addExpression(LSL_Expression *exp);
LSL_Expression *addInteger(int value);
LSL_Expression *addOperation(LSL_Operation type, LSL_Expression *left, LSL_Expression *right);

int yyerror(const char *msg);
int yyparse(void *param);


#endif // __EXPRESSION_H__

