
#include "LuaSL_LSL_tree.h"
#include <stdlib.h>
#include <stdio.h>

static void outputExpressionToken(LSL_Leaf *content);
static LSL_Leaf *evaluateExpressionToken(LSL_Leaf  *content, LSL_Type oldType, LSL_Leaf *old);
static void outputIntegerToken(LSL_Leaf *content);
static LSL_Leaf *evaluateIntegerToken(LSL_Leaf  *content, LSL_Type oldType, LSL_Leaf *old);

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
    {999999, NULL, LSL_NONE, NULL, NULL, NULL}
};

LSL_Token **tokens = NULL;
int lowestToken = 999999;


static LSL_AST *newAST(LSL_Type type, LSL_AST *left, LSL_AST *right)
{
    LSL_AST *ast = malloc(sizeof(LSL_AST));

    if (ast == NULL) return NULL;

    ast->type = type;
    ast->left = left;
    ast->right = right;
    ast->line = -1;
    ast->character = -1;
    if (tokens)
	ast->token = tokens[type - lowestToken];
    else
	ast->token = NULL;

    return ast;
}

static void burnAST(LSL_AST *ast)
{
    if (ast == NULL) return;

    burnAST(ast->left);
    burnAST(ast->right);
    // TODO - burn the contents to.
    free(ast);
}

LSL_AST *addExpression(LSL_Expression *exp)
{
    LSL_AST *ast = newAST(LSL_EXPRESSION, NULL, NULL);

    if (ast)
	ast->content.expressionValue = exp;

    return ast;
}

static LSL_Expression *newLSLExpression(LSL_Type type, LSL_Expression *left, LSL_Expression *right)
{
    LSL_Expression *exp = malloc(sizeof(LSL_Expression));

    if (exp == NULL) return NULL;

    exp->type = type;
    exp->left = left;
    exp->right = right;
    if (tokens)
	exp->token = tokens[type - lowestToken];
    else
	exp->token = NULL;

    return exp;
}

static void burnLSLExpression(LSL_Expression *exp)
{
    if (exp == NULL) return;

    burnLSLExpression(exp->left);
    burnLSLExpression(exp->right);
    free(exp);
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
	if (tokens)
	    exp->token = tokens[type - lowestToken];
	else
	    exp->token = NULL;
    }

    return exp;
}

static int evaluateExpression(LSL_Expression *exp, int old)
{
    if (NULL == exp)
	return old;
#ifdef LUASL_DEBUG
	printf(" %s ", exp->token->token);
#endif

    if (LSL_INTEGER == exp->type)
    {
#ifdef LUASL_DEBUG
	printf("%d", exp->content.integerValue);
#endif
	return exp->content.integerValue;
    }

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
	case LSL_BIT_NOT		: return ~ evaluateExpression(exp->right, old);
	case LSL_BOOL_NOT		: return ! evaluateExpression(exp->right, old);
	case LSL_NEGATION		: return 0 - evaluateExpression(exp->right, old);
	case LSL_DIVIDE			: return evaluateExpression(exp->left, old) /  evaluateExpression(exp->right, old);
#ifdef LUASL_USE_ENUM
	case LSL_MODULO			: return evaluateExpression(exp->left, old) %  evaluateExpression(exp->right, old);
#endif
	case LSL_MULTIPLY		: return evaluateExpression(exp->left, old) *  evaluateExpression(exp->right, old);
#ifdef LUASL_USE_ENUM
	case LSL_DOT_PRODUCT		: break;
	case LSL_CROSS_PRODUCT		: break;
#endif
	case LSL_SUBTRACT		: return evaluateExpression(exp->left, old) -  evaluateExpression(exp->right, old);
	case LSL_ADD			: return evaluateExpression(exp->left, old) +  evaluateExpression(exp->right, old);
#ifdef LUASL_USE_ENUM
	case LSL_CONCATENATE		: break;
#endif
	case LSL_LEFT_SHIFT		: return evaluateExpression(exp->left, old) << evaluateExpression(exp->right, old);
	case LSL_RIGHT_SHIFT		: return evaluateExpression(exp->left, old) >> evaluateExpression(exp->right, old);
	case LSL_LESS_THAN		: return evaluateExpression(exp->left, old) <  evaluateExpression(exp->right, old);
	case LSL_GREATER_THAN		: return evaluateExpression(exp->left, old) >  evaluateExpression(exp->right, old);
	case LSL_LESS_EQUAL		: return evaluateExpression(exp->left, old) <= evaluateExpression(exp->right, old);
	case LSL_GREATER_EQUAL		: return evaluateExpression(exp->left, old) >= evaluateExpression(exp->right, old);
	case LSL_EQUAL			: return evaluateExpression(exp->left, old) == evaluateExpression(exp->right, old);
	case LSL_NOT_EQUAL		: return evaluateExpression(exp->left, old) != evaluateExpression(exp->right, old);
	case LSL_BIT_AND		: return evaluateExpression(exp->left, old) &  evaluateExpression(exp->right, old);
	case LSL_BIT_XOR		: return evaluateExpression(exp->left, old) ^  evaluateExpression(exp->right, old);
	case LSL_BIT_OR			: return evaluateExpression(exp->left, old) |  evaluateExpression(exp->right, old);
	case LSL_BOOL_OR		: return evaluateExpression(exp->left, old) || evaluateExpression(exp->right, old);
	case LSL_BOOL_AND		: return evaluateExpression(exp->left, old) && evaluateExpression(exp->right, old);
    }

    return old;
}

static LSL_Leaf *evaluateExpressionToken(LSL_Leaf  *content, LSL_Type oldType, LSL_Leaf *old)
{
//    if (content)
//	return evaluateExpression(content->expressionValue, old->integerValue);
    return old;
}

static LSL_Leaf *evaluateIntegerToken(LSL_Leaf  *content, LSL_Type oldType, LSL_Leaf *old)
{
    if (content)
    {
#ifdef LUASL_DEBUG
	printf("%d", content->integerValue);
#endif
	return content;
    }
    return old;
}

static int evaluateAST(LSL_AST *ast, int old)
{
//    if ((ast) && (ast->token) && (ast->token->evaluate))
//	return ast->token->evaluate(&(ast->content), oldType, old);

    if (NULL == ast)
	return old;
    switch(ast->type)
    {
#ifdef LUASL_USE_ENUM
	case LSL_COMMENT	:
	case LSL_TYPE		:
	case LSL_NAME		:
	case LSL_IDENTIFIER	:
	    break;
	case LSL_FLOAT		: return (int) ast->content.floatValue;
#endif
	case LSL_INTEGER	:
#ifdef LUASL_DEBUG
	    printf("%d", ast->content.integerValue);
#endif
	    return ast->content.integerValue;
#ifdef LUASL_USE_ENUM
	case LSL_STRING		:
	case LSL_KEY		:
	case LSL_VECTOR		:
	case LSL_ROTATION	:
	case LSL_LIST		:
	case LSL_LABEL		:
	    break;
#endif
	case LSL_EXPRESSION	: return evaluateExpression(ast->content.expressionValue, old);
#ifdef LUASL_USE_ENUM
	case LSL_DO		:
	case LSL_FOR		:
	case LSL_IF		:
	case LSL_ELSE		:
	case LSL_ELSEIF		:
	case LSL_JUMP		:
	case LSL_STATE_CHANGE	:
	case LSL_WHILE		:
	case LSL_RETURN		:
	case LSL_STATEMENT	:
	case LSL_BLOCK		:
	case LSL_PARAMETER	:
	case LSL_FUNCTION	:
	case LSL_STATE		:
	case LSL_SCRIPT		:
	    break;
#endif
    }

    return old;
}

static void outputExpression(LSL_Expression *exp)
{
    if (NULL == exp)
	return;

    if (LSL_INTEGER == exp->type)
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
    const char test[] = " 4 + 2 * 10 + 3 * ( 5 + 1 )";
    LSL_AST *ast;
    int i;

    // Figure out what numbers yacc gave to our tokens.
    for (i = 0; LSL_Tokens[i].token != NULL; i++)
    {
	if (lowestToken > LSL_Tokens[i].type)
	    lowestToken = LSL_Tokens[i].type;
    }
    tokens = malloc(sizeof(LSL_Token *) * (i + 1));
    if (tokens)
    {
	// Sort the token table.
	for (i = 0; LSL_Tokens[i].token != NULL; i++)
	{
	    int j = LSL_Tokens[i].type - lowestToken;

	    tokens[j] = &(LSL_Tokens[i]);
	}
    }

    if ((ast = newTree(test)))
    {
	int result = evaluateAST(ast, 0);

#ifdef LUASL_DEBUG
	printf("\n");
#endif
	printf("Result of '%s' is %d\n", test, result);
	outputAST(ast);
	printf("\n");
	convertAST2Lua(ast);
	printf("\n");
	burnAST(ast);
    }

    return 0;
}

