
#include "LuaSL_LSL_tree.h"
#include <stdlib.h>
#include <stdio.h>

static void evaluateIntegerToken(LSL_Leaf  *content, LSL_Value *left, LSL_Value *right);
static void evaluateOperationToken(LSL_Leaf  *content, LSL_Value *left, LSL_Value *right);
static void outputIntegerToken(LSL_Leaf *content);
static void outputOperationToken(LSL_Leaf *content);

LSL_Token LSL_Tokens[] =
{
    // Start with expression operators.
    // In order of precedence, high to low.
    // Left to right, unless oterwise stated.
    // According to http://wiki.secondlife.com/wiki/Category:LSL_Operators

//    {LSL_COMMA,				",",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_INCREMENT_PRE,			"++",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_INCREMENT_POST,		"++",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_DECREMENT_PRE,			"--",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_DECREMENT_POST,		"--",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_DOT,				".",	LSL_RIGHT2LEFT,				outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_ASSIGNMENT_PLAIN,		"=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_ASSIGNMENT_DIVIDE,		"/=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_ASSIGNMENT_MODULO,		"%=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_ASSIGNMENT_MULTIPLY,		"*=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_ASSIGNMENT_SUBTRACT,		"-=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_ASSIGNMENT_ADD,		"+=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_ASSIGNMENT_CONCATENATE,	"+=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	outputOperationToken, NULL, evaluateOperationToken},
    {LSL_PARENTHESIS_OPEN,		"(",	LSL_INNER2OUTER,			outputOperationToken, NULL, evaluateOperationToken},
    {LSL_PARENTHESIS_CLOSE,		")",	LSL_INNER2OUTER,			outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_BRACKET_OPEN,			"[",	LSL_INNER2OUTER | LSL_CREATION,		outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_BRACKET_CLOSE,			"]",	LSL_INNER2OUTER | LSL_CREATION,		outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_ANGLE_OPEN,			"<",	LSL_LEFT2RIGHT | LSL_CREATION,		outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_ANGLE_CLOSE,			">",	LSL_LEFT2RIGHT | LSL_CREATION,		outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_TYPECAST_OPEN,			"(",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_TYPECAST_CLOSE,		")",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, evaluateOperationToken},
    {LSL_BIT_NOT,			"~",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, evaluateOperationToken},
    {LSL_BOOL_NOT,			"!",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, evaluateOperationToken},
    {LSL_NEGATION,			"-",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, evaluateOperationToken},
    {LSL_DIVIDE,			"/",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_MODULO,			"%",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_MULTIPLY,			"*",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_DOT_PRODUCT,			"*",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_CROSS_PRODUCT,			"%",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_SUBTRACT,			"-",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_ADD,				"+",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_CONCATENATE,			"+",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_LEFT_SHIFT,			"<<",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_RIGHT_SHIFT,			">>",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
// QUIRK - Conditionals are executed right to left.  Or left to right, depending on who you ask.  lol
    {LSL_LESS_THAN,			"<",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_GREATER_THAN,			">",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_LESS_EQUAL,			"<=",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_GREATER_EQUAL,			">=",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_EQUAL,				"==",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_NOT_EQUAL,			"!=",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_BIT_AND,			"&",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_BIT_XOR,			"^",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_BIT_OR,			"|",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
// QUIRK - Seems to be some disagreement about BOOL_AND/BOOL_OR precedence.  Either they are equal, or OR is higher.
// QUIRK - No boolean short circuiting.
    {LSL_BOOL_OR,			"||",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_BOOL_AND,			"&&",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},

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
    {LSL_EXPRESSION,			"expression",	LSL_NONE,			NULL, NULL, NULL},
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
	    ast->content.expressionValue = right;
	    ast->left = NULL;
	}
	else
	    ast->content.operationValue = type;
    }

    return ast;
}

static void evaluateAST(LSL_AST *ast, LSL_Value *left, LSL_Value *right)
{
    if (ast)
    {
	LSL_Value lresult;
	LSL_Value rresult;

	memcpy(&lresult, left, sizeof(LSL_Value));
	memcpy(&rresult, right, sizeof(LSL_Value));

	if (LSL_RIGHT2LEFT & ast->token->flags)
	{
	    memcpy(&rresult, left, sizeof(LSL_Value));
	    evaluateAST(ast->right, &rresult, right);
	    if (!(LSL_UNARY & ast->token->flags))
	    {
		evaluateAST(ast->left, &lresult, right);
	    }
	}
	else // Assume left to right.
	{
	    evaluateAST(ast->left, &lresult, right);
	    if (!(LSL_UNARY & ast->token->flags))
	    {
		memcpy(&rresult, left, sizeof(LSL_Value));
		evaluateAST(ast->right, &rresult, right);
	    }
	}

	if (ast->token->evaluate)
	{
	    ast->token->evaluate(&(ast->content), &lresult, &rresult);
	    memcpy(left, &lresult, sizeof(LSL_Value));
	}
	else
	{
	    printf(" eval <%s %d %d %d %d> ", ast->token->token, left->content.integerValue, right->content.integerValue, lresult.content.integerValue, rresult.content.integerValue);
	    memcpy(left, &rresult, sizeof(LSL_Value));
	}
    }
}

static void evaluateIntegerToken(LSL_Leaf *content, LSL_Value *left, LSL_Value *right)
{
    if (content)
    {
#ifdef LUASL_DEBUG
	printf(" <%d> ", content->integerValue);
#endif
	left->content.integerValue = content->integerValue;
	left->type = LSL_INTEGER;
    }
}

static void evaluateOperationToken(LSL_Leaf *content, LSL_Value *left, LSL_Value *right)
{
    if ((content) && (content->operationValue))
    {
#ifdef LUASL_DEBUG
	printf(" [%s] ", tokens[content->operationValue - lowestToken]->token);
#endif

	switch (content->operationValue)
	{
#ifdef LUASL_USE_ENUM
	    case LSL_COMMA			:
	    case LSL_INCREMENT_PRE		:
	    case LSL_INCREMENT_POST		:
	    case LSL_DECREMENT_PRE		:
	    case LSL_DECREMENT_POST		:
	    case LSL_DOT			:
	    case LSL_ASSIGNMENT_PLAIN		:
	    case LSL_ASSIGNMENT_DIVIDE		:
	    case LSL_ASSIGNMENT_MODULO		:
	    case LSL_ASSIGNMENT_MULTIPLY	:
	    case LSL_ASSIGNMENT_SUBTRACT	:
	    case LSL_ASSIGNMENT_ADD		:
	    case LSL_ASSIGNMENT_CONCATENATE	:
	    case LSL_PARENTHESIS_OPEN		:
	    case LSL_PARENTHESIS_CLOSE		:
	    case LSL_BRACKET_OPEN		:
	    case LSL_BRACKET_CLOSE		:
	    case LSL_ANGLE_OPEN			:
	    case LSL_ANGLE_CLOSE		:
	    case LSL_TYPECAST			:
		break;
#endif
	    case LSL_BIT_NOT		:  left->content.integerValue = ~ right->content.integerValue;					break;
	    case LSL_BOOL_NOT		:  left->content.integerValue = ! right->content.integerValue;					break;
	    case LSL_NEGATION		:  left->content.integerValue = 0 - right->content.integerValue;				break;
	    case LSL_DIVIDE		:  left->content.integerValue = left->content.integerValue /  right->content.integerValue;	break;
	    case LSL_MODULO		:  left->content.integerValue = left->content.integerValue %  right->content.integerValue;	break;
	    case LSL_MULTIPLY		:  left->content.integerValue = left->content.integerValue *  right->content.integerValue;	break;
#ifdef LUASL_USE_ENUM
	    case LSL_DOT_PRODUCT	: break;
	    case LSL_CROSS_PRODUCT	: break;
#endif
	    case LSL_SUBTRACT		:  left->content.integerValue = left->content.integerValue -  right->content.integerValue;	break;
	    case LSL_ADD		:  left->content.integerValue = left->content.integerValue +  right->content.integerValue;	break;
#ifdef LUASL_USE_ENUM
	    case LSL_CONCATENATE	: break;
#endif
	    case LSL_LEFT_SHIFT		:  left->content.integerValue = left->content.integerValue << right->content.integerValue;	break;
	    case LSL_RIGHT_SHIFT	:  left->content.integerValue = left->content.integerValue >> right->content.integerValue;	break;
	    case LSL_LESS_THAN		:  left->content.integerValue = left->content.integerValue <  right->content.integerValue;	break;
	    case LSL_GREATER_THAN	:  left->content.integerValue = left->content.integerValue >  right->content.integerValue;	break;
	    case LSL_LESS_EQUAL		:  left->content.integerValue = left->content.integerValue <= right->content.integerValue;	break;
	    case LSL_GREATER_EQUAL	:  left->content.integerValue = left->content.integerValue >= right->content.integerValue;	break;
	    case LSL_EQUAL		:  left->content.integerValue = left->content.integerValue == right->content.integerValue;	break;
	    case LSL_NOT_EQUAL		:  left->content.integerValue = left->content.integerValue != right->content.integerValue;	break;
	    case LSL_BIT_AND		:  left->content.integerValue = left->content.integerValue &  right->content.integerValue;	break;
	    case LSL_BIT_XOR		:  left->content.integerValue = left->content.integerValue ^  right->content.integerValue;	break;
	    case LSL_BIT_OR		:  left->content.integerValue = left->content.integerValue |  right->content.integerValue;	break;
	    case LSL_BOOL_OR		:  left->content.integerValue = left->content.integerValue || right->content.integerValue;	break;
	    case LSL_BOOL_AND		:  left->content.integerValue = left->content.integerValue && right->content.integerValue;	break;
	}
#ifdef LUASL_DEBUG
	printf(" (=%d) ", left->content.integerValue);
#endif
    }
}
static void outputAST(LSL_AST *ast)
{
    if (ast)
    {
	outputAST(ast->left);
	if (ast->token->output)
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
	printf(" %s ", tokens[content->operationValue - lowestToken]->token);
}

static void convertAST2Lua(LSL_AST *ast)
{
    if (ast)
    {
	convertAST2Lua(ast->left);
	if (ast->token->convert)
	    ast->token->convert(&(ast->content));
	else if (ast->token->output)
	    ast->token->output(&(ast->content));
	convertAST2Lua(ast->right);
    }
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
	    LSL_Value left, right;

	    left.content.integerValue = 0;
	    left.type = LSL_INTEGER;
	    right.content.integerValue = 0;
	    right.type = LSL_INTEGER;
	    evaluateAST(ast, &left, &right);

#ifdef LUASL_DEBUG
	    printf("\n");
#endif
	    printf("Result of '%s' is %d %d\n", test, left.content.integerValue, right.content.integerValue);
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

