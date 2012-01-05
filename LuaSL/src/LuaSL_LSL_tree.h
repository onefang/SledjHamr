/*
 * Definition of the structure used to build the abstract syntax tree.
 */
#ifndef __EXPRESSION_H__
#define __EXPRESSION_H__

#ifndef YY_NO_UNISTD_H
#define YY_NO_UNISTD_H 1
#endif // YY_NO_UNISTD_H

// http://w-hat.com/stackdepth is a useful discussion about some aspects of the LL parser.

typedef enum
{
    LSL_LEFT2RIGHT	= 0,
    LSL_RIGHT2LEFT	= 1,
    LSL_INNER2OUTER	= 2,
    LSL_UNARY		= 4,
    LSL_ASSIGNMENT	= 8,
    LSL_CREATION	= 16
} LSL_Flags;

typedef enum			// In order of precedence, high to low.
				// Left to right, unless oterwise stated.
				// According to http://wiki.secondlife.com/wiki/Category:LSL_Operators
{
    LSL_COMMA,
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

typedef struct
{
//    LSL_Operation	operation,
    char 		*token;
    LSL_Flags		flags;
} LSL_Operator;

// QUIRK - Seems to be some disagreement about BOOL_AND/BOOL_OR precedence.  Either they are equal, or OR is higher.
// QUIRK - Conditionals are executed right to left.  Or left to right, depending on who you ask.  lol
// QUIRK - No boolean short circuiting.

#ifdef LSL_Tokens_define
LSL_Operator LSL_Tokens[] =
{
    {",", LSL_LEFT2RIGHT},
    {"++", LSL_RIGHT2LEFT | LSL_UNARY},
    {"++", LSL_RIGHT2LEFT | LSL_UNARY},
    {"--", LSL_RIGHT2LEFT | LSL_UNARY},
    {"--", LSL_RIGHT2LEFT | LSL_UNARY},
    {".", LSL_RIGHT2LEFT},
    {"=", LSL_RIGHT2LEFT | LSL_ASSIGNMENT},
    {"/=", LSL_RIGHT2LEFT | LSL_ASSIGNMENT},
    {"%=", LSL_RIGHT2LEFT | LSL_ASSIGNMENT},
    {"*=", LSL_RIGHT2LEFT | LSL_ASSIGNMENT},
    {"-=", LSL_RIGHT2LEFT | LSL_ASSIGNMENT},
    {"+=", LSL_RIGHT2LEFT | LSL_ASSIGNMENT},
    {"+=", LSL_RIGHT2LEFT | LSL_ASSIGNMENT},
    {"(", LSL_INNER2OUTER},
    {")", LSL_INNER2OUTER},
    {"[", LSL_INNER2OUTER | LSL_CREATION},
    {"]", LSL_INNER2OUTER | LSL_CREATION},
    {"<", LSL_LEFT2RIGHT | LSL_CREATION},
    {">", LSL_LEFT2RIGHT | LSL_CREATION},
    {"()", LSL_RIGHT2LEFT | LSL_UNARY},
    {"~", LSL_RIGHT2LEFT | LSL_UNARY},
    {"!", LSL_RIGHT2LEFT | LSL_UNARY},
    {"-", LSL_RIGHT2LEFT | LSL_UNARY},
    {"/", LSL_LEFT2RIGHT},
    {"%", LSL_LEFT2RIGHT},
    {"*", LSL_LEFT2RIGHT},
    {"*", LSL_LEFT2RIGHT},
    {"%", LSL_LEFT2RIGHT},
    {"-", LSL_LEFT2RIGHT},
    {"+", LSL_LEFT2RIGHT},
    {"+", LSL_LEFT2RIGHT},
    {"<<", LSL_LEFT2RIGHT},
    {">>", LSL_LEFT2RIGHT},
    {"<", LSL_LEFT2RIGHT},
    {">", LSL_LEFT2RIGHT},
    {"<=", LSL_LEFT2RIGHT},
    {">=", LSL_LEFT2RIGHT},
    {"==", LSL_LEFT2RIGHT},
    {"!=", LSL_LEFT2RIGHT},
    {"&", LSL_LEFT2RIGHT},
    {"^", LSL_LEFT2RIGHT},
    {"|", LSL_LEFT2RIGHT},
    {"||", LSL_LEFT2RIGHT},
    {"&&", LSL_LEFT2RIGHT}
};
#endif

typedef enum
{
    LSL_COMMENT,
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

#ifdef LSL_Keywords_define
char *LSL_Keywords[] =
{
    "//",	// Also "/*",
    "",
    "",
    "",
    "float",
    "integer",
    "string",
    "key",
    "vector",
    "rotation",
    "list",
    "@",
    "",
    "do",
    "for",
    "if",
    "else",
    "else if",
    "jump",
    "state",
    "while",
    "return",
    ";",
    "{}",
    "",
    "",
    "",
    ""
};
#endif

typedef union
{
    float		floatValue;
    int			integerValue;
    char		*stringValue;
    char		*keyValue;
    float		vectorValue[3];
    float		rotationValue[4];
    union LSL_Leaf	*listValue;
} LSL_Value;

typedef struct
{
    char		*name;
    LSL_Type		type;
    LSL_Value		value;
} LSL_Identifier;

typedef struct LSL_Expression
{
    struct LSL_Expression *left;
    struct LSL_Expression *right;
    LSL_Value		value;
    LSL_Operation	expression;
    LSL_Type		type;
} LSL_Expression;

typedef struct
{
    LSL_Type		type;
    LSL_Expression	*expressions;
} LSL_Statement;

typedef struct
{
    LSL_Statement	*statements;
} LSL_Block;

typedef struct
{
    char		*name;
    LSL_Identifier	*parameters;
    LSL_Block		block;
    LSL_Type		type;
} LSL_Function;

typedef struct
{
    char		*name;
    LSL_Function	*handlers;
} LSL_State;

typedef struct
{
    char		*name;
    LSL_Identifier	*variables;
    LSL_Function	*functions;
    LSL_State		*states;
} LSL_Script;

typedef union LSL_Leaf
{
    char		*commentValue;
    LSL_Type		typeValue;
    char		*nameValue;
    LSL_Identifier	*identifierValue;
    float		floatValue;
    int			integerValue;
    char		*stringValue;
    char		*keyValue;
    float		vectorValue[3];
    float		rotationValue[4];
    union LSL_Leaf	*listValue;
    char		*labelValue;
//    LSL_Operation	expressionValue;
    LSL_Expression	*expressionValue;
    LSL_Statement	*doValue;
    LSL_Statement	*forValue;
    LSL_Statement	*ifValue;
    LSL_Statement	*elseValue;
    LSL_Statement	*elseIfValue;
    char		*jumpValue;
    char		*stateChangeValue;
    LSL_Statement	*statementValue;
    LSL_Identifier	*parameterValue;
    LSL_Function	*functionValue;
    LSL_State		*stateValue;
    LSL_Script		*scriptValue;
} LSL_Leaf;

typedef struct LSL_AST
{
    struct LSL_AST	*left;
    struct LSL_AST	*right;
    int			line;
    int			character;
    LSL_Type		type;
    LSL_Leaf		content;
} LSL_AST;


/**
 * @brief The structure used by flex and bison
 */
//typedef union tagTypeParser
//{
//        SExpression			*expression;
//        int				value;
//	int				ival;
//	float				fval;
//	char				*sval;
//	class LLScriptType		*type;
//	class LLScriptConstant		*constant;
//	class LLScriptIdentifier	*identifier;
//	class LLScriptSimpleAssignable	*assignable;
//	class LLScriptGlobalVariable	*global;
//	class LLScriptEvent		*event;
//	class LLScriptEventHandler	*handler;
//	class LLScriptExpression	*expression;
//	class LLScriptStatement		*statement;
//	class LLScriptGlobalFunctions	*global_funcs;
//	class LLScriptFunctionDec	*global_decl;
//	class LLScriptState		*state;
//	class LLScritpGlobalStorage	*global_store;
//	class LLScriptScript		*script;
//}STypeParser;
 
// define the type for flex and bison
#define YYSTYPE LSL_Leaf


#ifndef excludeLexer
    #include "LuaSL_lexer.h"
#endif


typedef struct
{
        yyscan_t scanner;
        LSL_Expression *expression;
} LuaSL_yyparseParam;


void burnLeaf(LSL_AST *leaf);
void burnLSLExpression(LSL_Expression *exp);
LSL_Expression *addInteger(int value);
LSL_Expression *addOperation(LSL_Operation type, LSL_Expression *left, LSL_Expression *right);
LSL_Expression *newTree(const char *expr);
int evaluateExpression(LSL_Expression *exp, int old);
void outputExpression(LSL_Expression *exp);
void convertExpression2Lua(LSL_Expression *exp);

// the parameter name (of the reentrant 'yyparse' function)
// data is a pointer to a 'SParserParam' structure
#define YYPARSE_PARAM data
 
// the argument for the 'yylex' function
#define YYLEX_PARAM   ((LuaSL_yyparseParam*)data)->scanner

int yyerror(const char *msg);
int yyparse(void *param);

#include "LuaSL_yaccer.tab.h"


#endif // __EXPRESSION_H__

