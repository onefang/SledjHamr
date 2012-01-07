
#include "LuaSL_LSL_tree.h"
#include <stdlib.h>
#include <stdio.h>

static void evaluateExpressionToken(LSL_Leaf  *content, LSL_Value *result);
static void outputIntegerToken(LSL_Leaf *content);
static void outputOperationToken(LSL_Leaf *content);
static void evaluateIntegerToken(LSL_Leaf  *content, LSL_Value *result);

LSL_Token LSL_Tokens[] =
{
    // Start with expression operators.
    // In order of precedence, high to low.
    // Left to right, unless oterwise stated.
    // According to http://wiki.secondlife.com/wiki/Category:LSL_Operators

//    {LSL_COMMA,				",",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},
//    {LSL_INCREMENT_PRE,			"++",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, NULL},
//    {LSL_INCREMENT_POST,		"++",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, NULL},
//    {LSL_DECREMENT_PRE,			"--",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, NULL},
//    {LSL_DECREMENT_POST,		"--",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, NULL},
//    {LSL_DOT,				".",	LSL_RIGHT2LEFT,				outputOperationToken, NULL, NULL},
//    {LSL_ASSIGNMENT_PLAIN,		"=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	outputOperationToken, NULL, NULL},
//    {LSL_ASSIGNMENT_DIVIDE,		"/=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	outputOperationToken, NULL, NULL},
//    {LSL_ASSIGNMENT_MODULO,		"%=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	outputOperationToken, NULL, NULL},
//    {LSL_ASSIGNMENT_MULTIPLY,		"*=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	outputOperationToken, NULL, NULL},
//    {LSL_ASSIGNMENT_SUBTRACT,		"-=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	outputOperationToken, NULL, NULL},
//    {LSL_ASSIGNMENT_ADD,		"+=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	outputOperationToken, NULL, NULL},
//    {LSL_ASSIGNMENT_CONCATENATE,	"+=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	outputOperationToken, NULL, NULL},
    {LSL_PARENTHESIS_OPEN,		"(",	LSL_INNER2OUTER,			outputOperationToken, NULL, NULL},
    {LSL_PARENTHESIS_CLOSE,		")",	LSL_INNER2OUTER,			outputOperationToken, NULL, NULL},
//    {LSL_BRACKET_OPEN,			"[",	LSL_INNER2OUTER | LSL_CREATION,		outputOperationToken, NULL, NULL},
//    {LSL_BRACKET_CLOSE,			"]",	LSL_INNER2OUTER | LSL_CREATION,		outputOperationToken, NULL, NULL},
//    {LSL_ANGLE_OPEN,			"<",	LSL_LEFT2RIGHT | LSL_CREATION,		outputOperationToken, NULL, NULL},
//    {LSL_ANGLE_CLOSE,			">",	LSL_LEFT2RIGHT | LSL_CREATION,		outputOperationToken, NULL, NULL},
//    {LSL_TYPECAST_OPEN,			"(",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, NULL},
//    {LSL_TYPECAST_CLOSE,		")",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, NULL},
    {LSL_BIT_NOT,			"~",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, NULL},
    {LSL_BOOL_NOT,			"!",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, NULL},
    {LSL_NEGATION,			"-",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, NULL},
    {LSL_DIVIDE,			"/",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},
    {LSL_MODULO,			"%",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},
    {LSL_MULTIPLY,			"*",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},
//    {LSL_DOT_PRODUCT,			"*",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},
//    {LSL_CROSS_PRODUCT,			"%",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},
    {LSL_SUBTRACT,			"-",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},
    {LSL_ADD,				"+",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},
//    {LSL_CONCATENATE,			"+",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},
    {LSL_LEFT_SHIFT,			"<<",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},
    {LSL_RIGHT_SHIFT,			">>",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},
// QUIRK - Conditionals are executed right to left.  Or left to right, depending on who you ask.  lol
    {LSL_LESS_THAN,			"<",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},
    {LSL_GREATER_THAN,			">",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},
    {LSL_LESS_EQUAL,			"<=",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},
    {LSL_GREATER_EQUAL,			">=",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},
    {LSL_EQUAL,				"==",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},
    {LSL_NOT_EQUAL,			"!=",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},
    {LSL_BIT_AND,			"&",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},
    {LSL_BIT_XOR,			"^",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},
    {LSL_BIT_OR,			"|",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},
// QUIRK - Seems to be some disagreement about BOOL_AND/BOOL_OR precedence.  Either they are equal, or OR is higher.
// QUIRK - No boolean short circuiting.
    {LSL_BOOL_OR,			"||",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},
    {LSL_BOOL_AND,			"&&",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, NULL},

    // Then the rest of the syntax tokens.

//    {LSL_COMMENT_LINE,			"//",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_COMMENT,			"/*",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_TYPE,				"type",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_NAME,				"name",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_IDENTIFIER,			"identifier",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_FLOAT,				"float",	LSL_NONE,			NULL, NULL, NULL},
    {LSL_INTEGER,			"integer",	LSL_NONE,			outputIntegerToken, NULL, evaluateIntegerToken},
//    {LSL_STRING,			"string",	LSL_NONE,			NULL, NULL, NULL},
//    {LSL_KEY,				"key",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_VECTOR,			"vector",	LSL_NONE,			NULL, NULL, NULL},
//    {LSL_ROTATION,			"rotation",	LSL_NONE,			NULL, NULL, NULL},
//    {LSL_LIST,				"list",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_LABEL,				"@",		LSL_NONE,			NULL, NULL, NULL},
    {LSL_EXPRESSION,			"expression",	LSL_NONE,			NULL, NULL, evaluateExpressionToken},
//    {LSL_DO,				"do",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_FOR,				"for",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_IF,				"if",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_ELSE,				"else",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_ELSE_IF,			"else if",	LSL_NONE,			NULL, NULL, NULL},
//    {LSL_JUMP,				"jump",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_STATE_CHANGE,			"state",	LSL_NONE,			NULL, NULL, NULL},
//    {LSL_WHILE,				"while",	LSL_NONE,			NULL, NULL, NULL},
//    {LSL_RETURN,			"return",	LSL_NONE,			NULL, NULL, NULL},
//    {LSL_STATEMENT,			";",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_BLOCK_OPEN,			"{",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_BLOCK_CLOSE,			"}",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_PARAMETER,			"parameter",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_FUNCTION,			"function",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_STATE,				"state",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_SCRIPT,			"script",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_UNKNOWN,			"unknown",	LSL_NONE,				NULL, NULL, NULL},
    {999999, NULL, LSL_NONE, NULL, NULL, NULL}
};

LSL_Token **tokens = NULL;
int lowestToken = 999999;


static LSL_AST *newAST(LSL_Type type, LSL_AST *left, LSL_AST *right)
{
    LSL_AST *ast = malloc(sizeof(LSL_AST));

    if (ast == NULL) return NULL;

    ast->left = left;
    ast->right = right;
    ast->line = -1;
    ast->character = -1;
    ast->token = tokens[type - lowestToken];

    return ast;
}

static void burnAST(LSL_AST *ast)
{
    if (ast == NULL) return;

    burnAST(ast->left);
    burnAST(ast->right);
    free(ast);
}

LSL_AST *addInteger(int value)
{
    LSL_AST *ast = newAST(LSL_INTEGER, NULL, NULL);

    if (ast)
	ast->content.integerValue = value;

    return ast;
}

LSL_AST *addOperation(LSL_Operation type, LSL_AST *left, LSL_AST *right)
{
    LSL_AST *ast = newAST(type, left, right);

    if (ast)
    {
	if (LSL_EXPRESSION == type)
	{
	    ast->content.expressionValue = left;
	    ast->right = NULL;
	}
	else
	    ast->content.operationValue = type;
    }

    return ast;
}

static void evaluateIntegerToken(LSL_Leaf  *content, LSL_Value *result)
{
    if (content)
    {
#ifdef LUASL_DEBUG
	printf(" %d ", content->integerValue);
#endif
	result->content.integerValue = content->integerValue;
	result->type = LSL_INTEGER;
    }
}

static void evaluateExpression(LSL_AST *ast, LSL_Value *result)
{
    LSL_Value left, right;

    if ((NULL == ast) || (NULL == result))
	return;

    if (LSL_INTEGER == ast->token->type)
    {
	evaluateIntegerToken(&(ast->content), result);
	return;
    }
    else if (LSL_LEFT2RIGHT & ast->token->flags)
    {
	evaluateExpression(ast->left, &left);
	if (!(LSL_UNARY & ast->token->flags))
	    evaluateExpression(ast->right, &right);
    }
    else if (LSL_RIGHT2LEFT & ast->token->flags)
    {
	evaluateExpression(ast->right, &right);
	if (!(LSL_UNARY & ast->token->flags))
	    evaluateExpression(ast->left, &left);
    }
    else
    {
    }

#ifdef LUASL_DEBUG
    printf(" %s ", ast->token->token);
#endif

    switch (ast->content.operationValue)
    {
#ifdef LUASL_USE_ENUM
	case LSL_COMMA			:
	case LSL_INCREMENT_PRE		:
	case LSL_INCREMENT_POST		:
	case LSL_DECREMENT_PRE		:
	case LSL_DECREMENT_POST		:
	case LSL_DOT			:
	case LSL_ASSIGNMENT_PLAIN	:
	case LSL_ASSIGNMENT_DIVIDE	:
	case LSL_ASSIGNMENT_MODULO	:
	case LSL_ASSIGNMENT_MULTIPLY	:
	case LSL_ASSIGNMENT_SUBTRACT	:
	case LSL_ASSIGNMENT_ADD		:
	case LSL_ASSIGNMENT_CONCATENATE	:
	case LSL_PARENTHESIS_OPEN	:
	case LSL_PARENTHESIS_CLOSE	:
	case LSL_BRACKET_OPEN		:
	case LSL_BRACKET_CLOSE		:
	case LSL_ANGLE_OPEN		:
	case LSL_ANGLE_CLOSE		:
	case LSL_TYPECAST		:
	    break;
#endif
	case LSL_BIT_NOT		:  result->content.integerValue = ~ right.content.integerValue;					break;
	case LSL_BOOL_NOT		:  result->content.integerValue = ! right.content.integerValue;					break;
	case LSL_NEGATION		:  result->content.integerValue = 0 - right.content.integerValue;				break;
	case LSL_DIVIDE			:  result->content.integerValue = left.content.integerValue / right.content.integerValue;	break;
	case LSL_MODULO			:  result->content.integerValue = left.content.integerValue % right.content.integerValue;	break;
	case LSL_MULTIPLY		:  result->content.integerValue = left.content.integerValue * right.content.integerValue;	break;
#ifdef LUASL_USE_ENUM
	case LSL_DOT_PRODUCT		: break;
	case LSL_CROSS_PRODUCT		: break;
#endif
	case LSL_SUBTRACT		:  result->content.integerValue = left.content.integerValue - right.content.integerValue;	break;
	case LSL_ADD			:  result->content.integerValue = left.content.integerValue + right.content.integerValue;	break;
#ifdef LUASL_USE_ENUM
	case LSL_CONCATENATE		: break;
#endif
	case LSL_LEFT_SHIFT		:  result->content.integerValue = left.content.integerValue << right.content.integerValue;	break;
	case LSL_RIGHT_SHIFT		:  result->content.integerValue = left.content.integerValue >> right.content.integerValue;	break;
	case LSL_LESS_THAN		:  result->content.integerValue = left.content.integerValue < right.content.integerValue;	break;
	case LSL_GREATER_THAN		:  result->content.integerValue = left.content.integerValue > right.content.integerValue;	break;
	case LSL_LESS_EQUAL		:  result->content.integerValue = left.content.integerValue <= right.content.integerValue;	break;
	case LSL_GREATER_EQUAL		:  result->content.integerValue = left.content.integerValue >= right.content.integerValue;	break;
	case LSL_EQUAL			:  result->content.integerValue = left.content.integerValue == right.content.integerValue;	break;
	case LSL_NOT_EQUAL		:  result->content.integerValue = left.content.integerValue != right.content.integerValue;	break;
	case LSL_BIT_AND		:  result->content.integerValue = left.content.integerValue & right.content.integerValue;	break;
	case LSL_BIT_XOR		:  result->content.integerValue = left.content.integerValue ^ right.content.integerValue;	break;
	case LSL_BIT_OR			:  result->content.integerValue = left.content.integerValue | right.content.integerValue;	break;
	case LSL_BOOL_OR		:  result->content.integerValue = left.content.integerValue || right.content.integerValue;	break;
	case LSL_BOOL_AND		:  result->content.integerValue = left.content.integerValue && right.content.integerValue;	break;
    }

    return;
}

static void evaluateExpressionToken(LSL_Leaf  *content, LSL_Value *result)
{
    if (content)
	evaluateExpression(content->expressionValue, result);
}

static void evaluateAST(LSL_AST *ast, LSL_Value *result)
{
    if (ast)
    {
	evaluateAST(ast->left, result);
	if ((ast->token) && (ast->token->evaluate))
	    ast->token->evaluate(&(ast->content), result);
	evaluateAST(ast->right, result);
    }
}

static void outputAST(LSL_AST *ast)
{
    if (ast)
    {
	outputAST(ast->left);
	if ((ast->token) && (ast->token->output))
	    ast->token->output(&(ast->content));
	else
	    printf(" <%s> ", ast->token->token);
	outputAST(ast->right);
    }
}

static void outputIntegerToken(LSL_Leaf *content)
{
    if (content)
	printf("%d", content->integerValue);
}

static void outputOperationToken(LSL_Leaf *content)
{
    if (content)
	printf(" [%s] ", tokens[content->operationValue - lowestToken]->token);
}

static void convertAST2Lua(LSL_AST *ast)
{
    if ((ast) && (ast->token) && (ast->token->convert))
	ast->token->convert(&(ast->content));
}

int yyerror(const char *msg)
{
    fprintf(stderr, "Parser error: %s\n", msg);
    return 0;
}

static LSL_AST *newTree(const char *expr)
{
    LuaSL_yyparseParam param;
    YY_BUFFER_STATE state;

#ifdef LUASL_DEBUG
    yydebug= 5;
#endif

    param.ast = NULL;
    if (yylex_init(&(param.scanner)))
	return NULL;

#ifdef LUASL_DEBUG
    yyset_debug(1, param.scanner);
#endif

    state = yy_scan_string(expr, param.scanner);
    if (yyparse(&param))
	return NULL;

    yy_delete_buffer(state, param.scanner);
    yylex_destroy(param.scanner);

    return param.ast;
}

int main(void)
{
    int i;

    // Figure out what numbers yacc gave to our tokens.
    for (i = 0; LSL_Tokens[i].token != NULL; i++)
    {
	if (lowestToken > LSL_Tokens[i].type)
	    lowestToken = LSL_Tokens[i].type;
    }
    tokens = calloc(i + 1, sizeof(LSL_Token *));
    if (tokens)
    {
	const char test[] = " 4 + 2 * 10 + 3 * ( 5 + 1 )";
	LSL_AST *ast;

	// Sort the token table.
	for (i = 0; LSL_Tokens[i].token != NULL; i++)
	{
	    int j = LSL_Tokens[i].type - lowestToken;

	    tokens[j] = &(LSL_Tokens[i]);
	}

	// Run the parser on a test.
	if ((ast = newTree(test)))
	{
	    LSL_Value result;

	    result.content.integerValue = 0;
	    result.type = LSL_INTEGER;
	    evaluateAST(ast, &result);

#ifdef LUASL_DEBUG
	    printf("\n");
#endif
	    printf("Result of '%s' is %d\n", test, result.content.integerValue);
	    outputAST(ast);
	    printf("\n");
	    convertAST2Lua(ast);
	    printf("\n");
	    burnAST(ast);
	}
    }
    else
    {
	fprintf(stderr, "No memory for tokens!");
	return 1;
    }

    return 0;
}

