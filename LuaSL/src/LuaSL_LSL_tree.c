
#include "LuaSL_LSL_tree.h"
#include <stdlib.h>
#include <stdio.h>

static void outputExpressionToken(LSL_Leaf *content);
static void evaluateExpressionToken(LSL_Leaf  *content, LSL_Value *result);
static void outputIntegerToken(LSL_Leaf *content);
static void evaluateIntegerToken(LSL_Leaf  *content, LSL_Value *result);

LSL_Token LSL_Tokens[] =
{
    // Start with expression operators.
    // In order of precedence, high to low.
    // Left to right, unless oterwise stated.
    // According to http://wiki.secondlife.com/wiki/Category:LSL_Operators

//    {LSL_COMMA,				",",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},
//    {LSL_INCREMENT_PRE,			"++",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, NULL, NULL},
//    {LSL_INCREMENT_POST,		"++",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, NULL, NULL},
//    {LSL_DECREMENT_PRE,			"--",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, NULL, NULL},
//    {LSL_DECREMENT_POST,		"--",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, NULL, NULL},
//    {LSL_DOT,				".",	LSL_RIGHT2LEFT,				NULL, NULL, NULL},
//    {LSL_ASSIGNMENT_PLAIN,		"=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL, NULL, NULL},
//    {LSL_ASSIGNMENT_DIVIDE,		"/=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL, NULL, NULL},
//    {LSL_ASSIGNMENT_MODULO,		"%=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL, NULL, NULL},
//    {LSL_ASSIGNMENT_MULTIPLY,		"*=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL, NULL, NULL},
//    {LSL_ASSIGNMENT_SUBTRACT,		"-=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL, NULL, NULL},
//    {LSL_ASSIGNMENT_ADD,		"+=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL, NULL, NULL},
//    {LSL_ASSIGNMENT_CONCATENATE,	"+=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL, NULL, NULL},
    {LSL_PARENTHESIS_OPEN,		"(",	LSL_INNER2OUTER,			NULL, NULL, NULL},
    {LSL_PARENTHESIS_CLOSE,		")",	LSL_INNER2OUTER,			NULL, NULL, NULL},
//    {LSL_BRACKET_OPEN,			"[",	LSL_INNER2OUTER | LSL_CREATION,		NULL, NULL, NULL},
//    {LSL_BRACKET_CLOSE,			"]",	LSL_INNER2OUTER | LSL_CREATION,		NULL, NULL, NULL},
//    {LSL_ANGLE_OPEN,			"<",	LSL_LEFT2RIGHT | LSL_CREATION,		NULL, NULL, NULL},
//    {LSL_ANGLE_CLOSE,			">",	LSL_LEFT2RIGHT | LSL_CREATION,		NULL, NULL, NULL},
//    {LSL_TYPECAST_OPEN,			"(",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, NULL, NULL},
//    {LSL_TYPECAST_CLOSE,		")",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, NULL, NULL},
    {LSL_BIT_NOT,			"~",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, NULL, NULL},
    {LSL_BOOL_NOT,			"!",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, NULL, NULL},
    {LSL_NEGATION,			"-",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, NULL, NULL},
    {LSL_DIVIDE,			"/",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},
    {LSL_MODULO,			"%",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},
    {LSL_MULTIPLY,			"*",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},
//    {LSL_DOT_PRODUCT,			"*",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},
//    {LSL_CROSS_PRODUCT,			"%",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},
    {LSL_SUBTRACT,			"-",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},
    {LSL_ADD,				"+",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},
//    {LSL_CONCATENATE,			"+",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},
    {LSL_LEFT_SHIFT,			"<<",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},
    {LSL_RIGHT_SHIFT,			">>",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},
// QUIRK - Conditionals are executed right to left.  Or left to right, depending on who you ask.  lol
    {LSL_LESS_THAN,			"<",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},
    {LSL_GREATER_THAN,			">",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},
    {LSL_LESS_EQUAL,			"<=",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},
    {LSL_GREATER_EQUAL,			">=",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},
    {LSL_EQUAL,				"==",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},
    {LSL_NOT_EQUAL,			"!=",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},
    {LSL_BIT_AND,			"&",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},
    {LSL_BIT_XOR,			"^",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},
    {LSL_BIT_OR,			"|",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},
// QUIRK - Seems to be some disagreement about BOOL_AND/BOOL_OR precedence.  Either they are equal, or OR is higher.
// QUIRK - No boolean short circuiting.
    {LSL_BOOL_OR,			"||",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},
    {LSL_BOOL_AND,			"&&",	LSL_LEFT2RIGHT,				NULL, NULL, NULL},

    // Then the rest of the syntax tokens.

//    {LSL_COMMENT_LINE,			"//",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_COMMENT,			"/*",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_TYPE,				"",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_NAME,				"",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_IDENTIFIER,			"",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_FLOAT,				"float",	LSL_NONE,			NULL, NULL, NULL},
    {LSL_INTEGER,			"integer",	LSL_NONE,			outputIntegerToken, NULL, evaluateIntegerToken},
//    {LSL_STRING,			"string",	LSL_NONE,			NULL, NULL, NULL},
//    {LSL_KEY,				"key",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_VECTOR,			"vector",	LSL_NONE,			NULL, NULL, NULL},
//    {LSL_ROTATION,			"rotation",	LSL_NONE,			NULL, NULL, NULL},
//    {LSL_LIST,				"list",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_LABEL,				"@",		LSL_NONE,			NULL, NULL, NULL},
    {LSL_EXPRESSION,			"",		LSL_NONE,			outputExpressionToken, NULL, evaluateExpressionToken},
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
//    {LSL_PARAMETER,			"",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_FUNCTION,			"",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_STATE,				"",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_SCRIPT,			"",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_UNKNOWN,			"",	LSL_NONE,				NULL, NULL, NULL},
    {999999, NULL, LSL_NONE, NULL, NULL, NULL}
};

LSL_Token **tokens = NULL;
int lowestToken = 999999;


static LSL_Expression *newLSLExpression(LSL_Type type, LSL_Expression *left, LSL_Expression *right)
{
    LSL_Expression *exp = malloc(sizeof(LSL_Expression));

    if (exp == NULL) return NULL;

    exp->left = left;
    exp->right = right;
    exp->token = tokens[type - lowestToken];

    return exp;
}

static void burnLSLExpression(LSL_Expression *exp)
{
    if (exp == NULL) return;

    burnLSLExpression(exp->left);
    burnLSLExpression(exp->right);
    free(exp);
}

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
    // Burn the contents to.
    if ((ast->token) && (ast->token->type == LSL_EXPRESSION))
	burnLSLExpression(ast->content.expressionValue);
    free(ast);
}

LSL_AST *addExpression(LSL_Expression *exp)
{
    LSL_AST *ast = newAST(LSL_EXPRESSION, NULL, NULL);

    if (ast)
	ast->content.expressionValue = exp;

    return ast;
}

LSL_Expression *addInteger(int value)
{
    LSL_Expression *exp = newLSLExpression(LSL_INTEGER, NULL, NULL);

    if (exp)
	exp->content.integerValue = value;

    return exp;
}

LSL_Expression *addOperation(LSL_Operation type, LSL_Expression *left, LSL_Expression *right)
{
    LSL_Expression *exp = newLSLExpression(LSL_EXPRESSION, left, right);

    if (exp)
    {
	exp->content.operationValue = type;
	exp->token = tokens[type - lowestToken];
    }

    return exp;
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

static void evaluateExpression(LSL_Expression *exp, LSL_Value *result)
{
    LSL_Value left, right;

    if ((NULL == exp) || (NULL == result))
	return;

    if (LSL_INTEGER == exp->token->type)
    {
	evaluateIntegerToken(&(exp->content), result);
	return;
    }
    else if (LSL_LEFT2RIGHT & exp->token->flags)
    {
	evaluateExpression(exp->left, &left);
	if (!(LSL_UNARY & exp->token->flags))
	    evaluateExpression(exp->right, &right);
    }
    else if (LSL_RIGHT2LEFT & exp->token->flags)
    {
	evaluateExpression(exp->right, &right);
	if (!(LSL_UNARY & exp->token->flags))
	    evaluateExpression(exp->left, &left);
    }
    else
    {
    }

#ifdef LUASL_DEBUG
    printf(" %s ", exp->token->token);
#endif

    switch (exp->content.operationValue)
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
    if ((ast) && (ast->token) && (ast->token->evaluate))
	ast->token->evaluate(&(ast->content), result);
}

static void outputExpression(LSL_Expression *exp)
{
    if (NULL == exp)
	return;

    if (LSL_INTEGER == exp->token->type)
    {
	printf("%d", exp->content.integerValue);
    }
    else
    {
	outputExpression(exp->left);
	printf(" %s ", exp->token->token);
	outputExpression(exp->right);
    }
}

static void outputExpressionToken(LSL_Leaf *content)
{
    if (content)
	outputExpression(content->expressionValue);
}

static void outputIntegerToken(LSL_Leaf *content)
{
    if (content)
	printf("%d", content->integerValue);
}

static void outputAST(LSL_AST *ast)
{
    if ((ast) && (ast->token) && (ast->token->output))
	ast->token->output(&(ast->content));
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

