
#include "LuaSL_LSL_tree.h"
#include <stdlib.h>
#include <stdio.h>

static void evaluateIntegerToken(LSL_Leaf  *content, LSL_Value *left, LSL_Value *right);
static void evaluateNoToken(LSL_Leaf *content, LSL_Value *left, LSL_Value *right);
static void evaluateOperationToken(LSL_Leaf  *content, LSL_Value *left, LSL_Value *right);
static void evaluateStatementToken(LSL_Leaf *content, LSL_Value *left, LSL_Value *right);
static void outputIntegerToken(LSL_Leaf *content);
static void outputOperationToken(LSL_Leaf *content);
static void outputStatementToken(LSL_Leaf *content);

LSL_Token LSL_Tokens[] =
{
//    {LSL_COMMENT,			"/*",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_COMMENT_LINE,			"//",		LSL_NONE,			NULL, NULL, NULL},
    {LSL_SPACE,			" ",		LSL_NONE,			NULL, NULL, NULL},

    // Operators, in order of precedence, low to high
    // Left to right, unless oterwise stated.
    // According to http://wiki.secondlife.com/wiki/Category:LSL_Operators

    {LSL_BOOL_AND,			"&&",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
// QUIRK - Seems to be some disagreement about BOOL_AND/BOOL_OR precedence.  Either they are equal, or OR is higher.
// QUIRK - No boolean short circuiting.
    {LSL_BOOL_OR,			"||",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_BIT_OR,			"|",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_BIT_XOR,			"^",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_BIT_AND,			"&",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
// QUIRK - Conditionals are executed right to left.  Or left to right, depending on who you ask.  lol
    {LSL_NOT_EQUAL,			"!=",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_EQUAL,				"==",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_GREATER_EQUAL,			">=",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_LESS_EQUAL,			"<=",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_GREATER_THAN,			">",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_LESS_THAN,			"<",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_RIGHT_SHIFT,			">>",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_LEFT_SHIFT,			"<<",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_CONCATENATE,			"+",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_ADD,				"+",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_SUBTRACT,			"-",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_CROSS_PRODUCT,			"%",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_DOT_PRODUCT,			"*",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_MULTIPLY,			"*",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_MODULO,			"%",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_DIVIDE,			"/",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},
    {LSL_NEGATION,			"-",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, evaluateOperationToken},
    {LSL_BOOL_NOT,			"!",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, evaluateOperationToken},
    {LSL_BIT_NOT,			"~",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_TYPECAST_CLOSE,		")",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_TYPECAST_OPEN,			"(",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, evaluateOperationToken},
    {LSL_ANGLE_CLOSE,			">",	LSL_LEFT2RIGHT | LSL_CREATION,		outputOperationToken, NULL, evaluateOperationToken},
    {LSL_ANGLE_OPEN,			"<",	LSL_LEFT2RIGHT | LSL_CREATION,		outputOperationToken, NULL, evaluateOperationToken},
    {LSL_BRACKET_CLOSE,			"]",	LSL_INNER2OUTER | LSL_CREATION,		outputOperationToken, NULL, evaluateOperationToken},
    {LSL_BRACKET_OPEN,			"[",	LSL_INNER2OUTER | LSL_CREATION,		outputOperationToken, NULL, evaluateOperationToken},
    {LSL_PARENTHESIS_CLOSE,		")",	LSL_INNER2OUTER,			NULL, NULL, evaluateNoToken},
    {LSL_PARENTHESIS_OPEN,		"(",	LSL_INNER2OUTER,			NULL, NULL, NULL},
//    {LSL_ASSIGNMENT_CONCATENATE,	"+=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	outputOperationToken, NULL, evaluateOperationToken},
    {LSL_ASSIGNMENT_ADD,		"+=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	outputOperationToken, NULL, evaluateOperationToken},
    {LSL_ASSIGNMENT_SUBTRACT,		"-=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	outputOperationToken, NULL, evaluateOperationToken},
    {LSL_ASSIGNMENT_MULTIPLY,		"*=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	outputOperationToken, NULL, evaluateOperationToken},
    {LSL_ASSIGNMENT_MODULO,		"%=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	outputOperationToken, NULL, evaluateOperationToken},
    {LSL_ASSIGNMENT_DIVIDE,		"/=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	outputOperationToken, NULL, evaluateOperationToken},
    {LSL_ASSIGNMENT_PLAIN,		"=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	outputOperationToken, NULL, evaluateOperationToken},
    {LSL_DOT,				".",	LSL_RIGHT2LEFT,				outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_DECREMENT_POST,		"--",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, evaluateOperationToken},
    {LSL_DECREMENT_PRE,			"--",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, evaluateOperationToken},
//    {LSL_INCREMENT_POST,		"++",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, evaluateOperationToken},
    {LSL_INCREMENT_PRE,			"++",	LSL_RIGHT2LEFT | LSL_UNARY,		outputOperationToken, NULL, evaluateOperationToken},
    {LSL_COMMA,				",",	LSL_LEFT2RIGHT,				outputOperationToken, NULL, evaluateOperationToken},

    {LSL_EXPRESSION,			"expression",	LSL_NONE,			NULL, NULL, NULL},

    // Types.

    {LSL_FLOAT,				"float",	LSL_NONE,			NULL, NULL, NULL},
    {LSL_INTEGER,			"integer",	LSL_NONE,			outputIntegerToken, NULL, evaluateIntegerToken},
//    {LSL_KEY,				"key",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_LIST,				"list",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_ROTATION,			"rotation",	LSL_NONE,			NULL, NULL, NULL},
//    {LSL_STRING,			"string",	LSL_NONE,			NULL, NULL, NULL},
//    {LSL_VECTOR,			"vector",	LSL_NONE,			NULL, NULL, NULL},

    // Types names.

    {LSL_TYPE_FLOAT,			"float",	LSL_NONE,			NULL, NULL, NULL},
    {LSL_TYPE_INTEGER,			"integer",	LSL_NONE,			NULL, NULL, NULL},
    {LSL_TYPE_KEY,			"key",		LSL_NONE,			NULL, NULL, NULL},
    {LSL_TYPE_LIST,			"list",		LSL_NONE,			NULL, NULL, NULL},
    {LSL_TYPE_ROTATION,			"rotation",	LSL_NONE,			NULL, NULL, NULL},
    {LSL_TYPE_STRING,			"string",	LSL_NONE,			NULL, NULL, NULL},
    {LSL_TYPE_VECTOR,			"vector",	LSL_NONE,			NULL, NULL, NULL},

    // Then the rest of the syntax tokens.

//    {LSL_IDENTIFIER,			"identifier",		LSL_NONE,			NULL, NULL, NULL},

    {LSL_LABEL,				"@",		LSL_NONE,			NULL, NULL, NULL},

    {LSL_DO,				"do",		LSL_NONE,			NULL, NULL, NULL},
    {LSL_FOR,				"for",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_ELSE_IF,			"else if",	LSL_NONE,			NULL, NULL, NULL},
    {LSL_ELSE,				"else",		LSL_NONE,			NULL, NULL, NULL},
    {LSL_IF,				"if",		LSL_NONE,			NULL, NULL, NULL},
    {LSL_JUMP,				"jump",		LSL_NONE,			NULL, NULL, NULL},
    {LSL_RETURN,			"return",	LSL_NONE,			NULL, NULL, NULL},
    {LSL_STATE_CHANGE,			"state",	LSL_NONE,			NULL, NULL, NULL},
    {LSL_WHILE,				"while",	LSL_NONE,			NULL, NULL, NULL},
    {LSL_STATEMENT,			";",		LSL_NONE,			outputStatementToken, NULL, evaluateStatementToken},

    {LSL_BLOCK_CLOSE,			"}",		LSL_NONE,			NULL, NULL, NULL},
    {LSL_BLOCK_OPEN,			"{",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_PARAMETER,			"parameter",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_FUNCTION,			"function",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_STATE,				"state",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_SCRIPT,			"script",		LSL_NONE,			NULL, NULL, NULL},

//    {LSL_UNKNOWN,			"unknown",	LSL_NONE,				NULL, NULL, NULL},

    // A sentinal.

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

LSL_AST *addOperation(LSL_Type type, LSL_AST *left, LSL_AST *right)
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

LSL_AST *addParenthesis(LSL_AST *expr)
{
    LSL_AST *ast = newAST(LSL_PARENTHESIS_OPEN, NULL, expr);

    if (ast)
	ast = newAST(LSL_PARENTHESIS_CLOSE, ast, NULL);

    return ast;
}

LSL_Statement *createStatement(LSL_Type type, LSL_AST *expr)
{
    LSL_Statement *stat = malloc(sizeof(LSL_Statement));

    if (stat == NULL) return NULL;

    stat->type = type;
    stat->expression = expr;

    return stat;
}

LSL_AST *addSpace(char *text, LSL_AST *root)
{
    LSL_AST *ast = newAST(LSL_SPACE, root, NULL);

    if (ast)
	ast->content.spaceValue = text;

    return ast;
}

LSL_AST *addStatement(LSL_Statement *statement, LSL_AST *root)
{
    LSL_AST *ast = newAST(LSL_STATEMENT, root, NULL);

    if (ast)
	ast->content.statementValue = statement;

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
#ifdef LUASL_DEBUG
	    printf(" eval <%s %d %d %d %d> ", ast->token->token, left->content.integerValue, right->content.integerValue, lresult.content.integerValue, rresult.content.integerValue);
#endif
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

static void evaluateNoToken(LSL_Leaf *content, LSL_Value *left, LSL_Value *right)
{
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
	    case LSL_COMMA			:
	    case LSL_INCREMENT_PRE		:
//	    case LSL_INCREMENT_POST		:
	    case LSL_DECREMENT_PRE		:
//	    case LSL_DECREMENT_POST		:
	    case LSL_DOT			:
	    case LSL_ASSIGNMENT_PLAIN		:
	    case LSL_ASSIGNMENT_DIVIDE		:
	    case LSL_ASSIGNMENT_MODULO		:
	    case LSL_ASSIGNMENT_MULTIPLY	:
	    case LSL_ASSIGNMENT_SUBTRACT	:
	    case LSL_ASSIGNMENT_ADD		:
//	    case LSL_ASSIGNMENT_CONCATENATE	:
	    case LSL_PARENTHESIS_OPEN		:
	    case LSL_PARENTHESIS_CLOSE		:
	    case LSL_BRACKET_OPEN		:
	    case LSL_BRACKET_CLOSE		:
	    case LSL_ANGLE_OPEN			:
	    case LSL_ANGLE_CLOSE		:
//	    case LSL_TYPECAST_OPEN		:
//	    case LSL_TYPECAST_CLOSE		:
		break;
	    case LSL_BIT_NOT		:  left->content.integerValue = ~ right->content.integerValue;					break;
	    case LSL_BOOL_NOT		:  left->content.integerValue = ! right->content.integerValue;					break;
	    case LSL_NEGATION		:  left->content.integerValue = 0 - right->content.integerValue;				break;
	    case LSL_DIVIDE		:  left->content.integerValue = left->content.integerValue /  right->content.integerValue;	break;
	    case LSL_MODULO		:  left->content.integerValue = left->content.integerValue %  right->content.integerValue;	break;
	    case LSL_MULTIPLY		:  left->content.integerValue = left->content.integerValue *  right->content.integerValue;	break;
//	    case LSL_DOT_PRODUCT	: break;
//	    case LSL_CROSS_PRODUCT	: break;
	    case LSL_SUBTRACT		:  left->content.integerValue = left->content.integerValue -  right->content.integerValue;	break;
	    case LSL_ADD		:  left->content.integerValue = left->content.integerValue +  right->content.integerValue;	break;
//	    case LSL_CONCATENATE	: break;
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

static void evaluateStatementToken(LSL_Leaf *content, LSL_Value *left, LSL_Value *right)
{
    if (content)
	evaluateAST(content->statementValue->expression, left, right);
}

static void outputAST(LSL_AST *ast)
{
    if (ast)
    {
	outputAST(ast->left);
	if (ast->token->output)
	    ast->token->output(&(ast->content));
	else
	    printf(" %s ", ast->token->token);
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

static void outputStatementToken(LSL_Leaf *content)
{
    if (content)
	outputAST(content->statementValue->expression);
    printf(";");
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
	else
	    printf(" %s ", ast->token->token);
	convertAST2Lua(ast->right);
    }
}

int yyerror(const char *msg)
{
    fprintf(stderr, "Parser error: %s\n", msg);
    return 0;
}

int main(int argc, char **argv)
{
    char *programName = argv[0];
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
	char buffer[4096];
	LSL_AST *ast;
	LuaSL_yyparseParam param;
	int count;
	FILE *file;
	boolean badArgs = FALSE;

	// Sort the token table.
	for (i = 0; LSL_Tokens[i].token != NULL; i++)
	{
	    int j = LSL_Tokens[i].type - lowestToken;

	    tokens[j] = &(LSL_Tokens[i]);
	}

	buffer[0] = '\0';

	// get the arguments passed in
	while (--argc > 0 && *++argv != '\0')
	{
	    if (*argv[0] == '-')
	    {
		// point to the characters after the '-' sign
		char *s = argv[0] + 1;

		switch (*s)
		{
		    case 'f': // file
		    {
			if (--argc > 0 && *++argv != '\0')
			{
			    strncpy(buffer, *argv, 4095);
			    buffer[4095] = '\0';;
			}
			else
			    badArgs = TRUE;
			break;
		    }
		    default:
			badArgs = TRUE;
		}
	    }
	    else
		badArgs = TRUE;
	}

	if (badArgs)
	{
	    printf("Usage: %s [-f filename]\n", programName);
	    printf("   -f: Script file to run.\n");
	    printf("Or pass filenames in stdin.\n");
	    return 1;
	}

	if ('\0' == buffer[0])
	{
	    count = read(STDIN_FILENO, buffer, sizeof(buffer));
	    if (0 > count)
	    {
		printf("Error in stdin!\n");
		return 1;
	    }
	    else if (0 == count)
	    {
		printf("No bytes in stdin!\n");
		return 1;
	    }
	    else
	    {
		buffer[count] = '\0';
		printf("Filename %s in stdin.\n", buffer);
	    }
	}
	else
	    printf("Filename %s in argument.\n", buffer);

	file = fopen(buffer, "r");
	if (NULL == file)
	{
	    printf("Error opening file %s.\n", buffer);
	    return 1;
	}

#ifdef LUASL_DEBUG
	yydebug= 5;
#endif

	param.ast = NULL;
	if (yylex_init(&(param.scanner)))
	    return 1;

#ifdef LUASL_DEBUG
	yyset_debug(1, param.scanner);
#endif
	yyset_in(file, param.scanner);

	if (!yyparse(&param))
	{
	    yylex_destroy(param.scanner);
	    ast = param.ast;

	    if (ast)
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
		printf("Result of -\n");
		outputAST(ast);
		printf("\n");
		printf("is %d %d.  And converted to Lua it is -\n", left.content.integerValue, right.content.integerValue);
		convertAST2Lua(ast);
		printf("\n");
		burnAST(ast);
	    }
	}
    }
    else
    {
	fprintf(stderr, "No memory for tokens!");
	return 1;
    }

    return 0;
}

