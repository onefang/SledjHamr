
#include "LuaSL_LSL_tree.h"
#include <stdlib.h>
#include <stdio.h>

static void evaluateIntegerToken(LSL_Leaf  *content, LSL_Leaf *left, LSL_Leaf *right);
static void evaluateNoToken(LSL_Leaf *content, LSL_Leaf *left, LSL_Leaf *right);
static void evaluateOperationToken(LSL_Leaf  *content, LSL_Leaf *left, LSL_Leaf *right);
static void eveluateParenthesisToken(LSL_Leaf *content, LSL_Leaf *left, LSL_Leaf *right);
static void evaluateStatementToken(LSL_Leaf *content, LSL_Leaf *left, LSL_Leaf *right);
static void outputIntegerToken(FILE *file, LSL_Leaf *content);
static void outputParenthesisToken(FILE *file, LSL_Leaf *content);
static void outputStatementToken(FILE *file, LSL_Leaf *content);

LSL_Token LSL_Tokens[] =
{
    // Various forms of "space".
    {LSL_COMMENT,			"/*",		LSL_NONE,			NULL, NULL, NULL},
    {LSL_COMMENT_LINE,			"//",		LSL_NONE,			NULL, NULL, NULL},
    {LSL_SPACE,				" ",		LSL_NONE,			NULL, NULL, NULL},

    // Operators, in order of precedence, low to high
    // Left to right, unless oterwise stated.
    // According to http://wiki.secondlife.com/wiki/Category:LSL_Operators
    {LSL_BOOL_AND,			"&&",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},
// QUIRK - Seems to be some disagreement about BOOL_AND/BOOL_OR precedence.  Either they are equal, or OR is higher.
// QUIRK - No boolean short circuiting.
    {LSL_BOOL_OR,			"||",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},
    {LSL_BIT_OR,			"|",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},
    {LSL_BIT_XOR,			"^",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},
    {LSL_BIT_AND,			"&",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},
// QUIRK - Conditionals are executed right to left.  Or left to right, depending on who you ask.  lol
    {LSL_NOT_EQUAL,			"!=",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},
    {LSL_EQUAL,				"==",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},
    {LSL_GREATER_EQUAL,			">=",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},
    {LSL_LESS_EQUAL,			"<=",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},
    {LSL_GREATER_THAN,			">",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},
    {LSL_LESS_THAN,			"<",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},
    {LSL_RIGHT_SHIFT,			">>",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},
    {LSL_LEFT_SHIFT,			"<<",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},
//    {LSL_CONCATENATE,			"+",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},
    {LSL_ADD,				"+",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},
    {LSL_SUBTRACT,			"-",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},
//    {LSL_CROSS_PRODUCT,			"%",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},
//    {LSL_DOT_PRODUCT,			"*",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},
    {LSL_MULTIPLY,			"*",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},
    {LSL_MODULO,			"%",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},
    {LSL_DIVIDE,			"/",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},
    {LSL_NEGATION,			"-",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, NULL, evaluateOperationToken},
    {LSL_BOOL_NOT,			"!",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, NULL, evaluateOperationToken},
    {LSL_BIT_NOT,			"~",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, NULL, evaluateOperationToken},
//    {LSL_TYPECAST_CLOSE,		")",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, NULL, evaluateOperationToken},
//    {LSL_TYPECAST_OPEN,			"(",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, NULL, evaluateOperationToken},
    {LSL_ANGLE_CLOSE,			">",	LSL_LEFT2RIGHT | LSL_CREATION,		NULL, NULL, evaluateOperationToken},
    {LSL_ANGLE_OPEN,			"<",	LSL_LEFT2RIGHT | LSL_CREATION,		NULL, NULL, evaluateOperationToken},
    {LSL_BRACKET_CLOSE,			"]",	LSL_INNER2OUTER | LSL_CREATION,		NULL, NULL, evaluateOperationToken},
    {LSL_BRACKET_OPEN,			"[",	LSL_INNER2OUTER | LSL_CREATION,		NULL, NULL, evaluateOperationToken},
    {LSL_PARENTHESIS_CLOSE,		")",	LSL_INNER2OUTER,			NULL, NULL, evaluateNoToken},
    {LSL_PARENTHESIS_OPEN,		"(",	LSL_INNER2OUTER,			outputParenthesisToken, NULL, eveluateParenthesisToken},
//    {LSL_ASSIGNMENT_CONCATENATE,	"+=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL, NULL, evaluateOperationToken},
    {LSL_ASSIGNMENT_ADD,		"+=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL, NULL, evaluateOperationToken},
    {LSL_ASSIGNMENT_SUBTRACT,		"-=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL, NULL, evaluateOperationToken},
    {LSL_ASSIGNMENT_MULTIPLY,		"*=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL, NULL, evaluateOperationToken},
    {LSL_ASSIGNMENT_MODULO,		"%=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL, NULL, evaluateOperationToken},
    {LSL_ASSIGNMENT_DIVIDE,		"/=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL, NULL, evaluateOperationToken},
    {LSL_ASSIGNMENT_PLAIN,		"=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL, NULL, evaluateOperationToken},
    {LSL_DOT,				".",	LSL_RIGHT2LEFT,				NULL, NULL, evaluateOperationToken},
//    {LSL_DECREMENT_POST,		"--",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, NULL, evaluateOperationToken},
    {LSL_DECREMENT_PRE,			"--",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, NULL, evaluateOperationToken},
//    {LSL_INCREMENT_POST,		"++",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, NULL, evaluateOperationToken},
    {LSL_INCREMENT_PRE,			"++",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, NULL, evaluateOperationToken},
    {LSL_COMMA,				",",	LSL_LEFT2RIGHT,				NULL, NULL, evaluateOperationToken},

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
    {LSL_IDENTIFIER,			"identifier",	LSL_NONE,			NULL, NULL, NULL},

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
    {LSL_STATEMENT,			";",		LSL_NOIGNORE,			outputStatementToken, NULL, evaluateStatementToken},

    {LSL_BLOCK_CLOSE,			"}",		LSL_NONE,			NULL, NULL, NULL},
    {LSL_BLOCK_OPEN,			"{",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_PARAMETER,			"parameter",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_FUNCTION,			"function",		LSL_NONE,			NULL, NULL, NULL},
//    {LSL_STATE,				"state",		LSL_NONE,			NULL, NULL, NULL},
    {LSL_SCRIPT,			"",		LSL_NONE,			NULL, NULL, NULL},

    {LSL_UNKNOWN,			"unknown",	LSL_NONE,			NULL, NULL, NULL},

    // A sentinal.
    {999999, NULL, LSL_NONE, NULL, NULL, NULL}
};

LSL_Token **tokens = NULL;
int lowestToken = 999999;


/*  Not actually used, but it might be some day.
static LSL_Leaf *newLeaf(LSL_Type type, LSL_Leaf *left, LSL_Leaf *right)
{
    LSL_Leaf *leaf = calloc(1, sizeof(LSL_Leaf));

    if (leaf)
    {
	leaf->left = left;
	leaf->right = right;
	leaf->token = tokens[type - lowestToken];
    }

    return leaf;
}
*/

void burnLeaf(LSL_Leaf *leaf)
{
    if (leaf)
    {
	burnLeaf(leaf->left);
	burnLeaf(leaf->right);
	// TODO - Should free up the value to.
	free(leaf->ignorableText);
	free(leaf);
    }
}

LSL_Leaf *addOperation(LSL_Leaf *left, LSL_Leaf *lval, LSL_Leaf *right)
{
    if (lval)
    {
	lval->left = left;
	lval->right = right;
    }

    return lval;
}

LSL_Leaf *addParenthesis(LSL_Leaf *lval, LSL_Leaf *expr, LSL_Leaf *rval)
{
    LSL_Parenthesis *parens = malloc(sizeof(LSL_Parenthesis));

    if (parens)
    {
	parens->left = lval;
	parens->expression = expr;
	parens->right = rval;
	if (lval)
	    lval->value.parenthesis = parens;
    }
    return lval;
}

LSL_Leaf *addStatement(LSL_Leaf *lval, LSL_Type type, LSL_Leaf *expr)
{
    LSL_Statement *stat = malloc(sizeof(LSL_Statement));

    if (stat)
    {
	stat->type = type;
	stat->expressions = expr;
	if (lval)
	    lval->value.statementValue = stat;
    }

    return lval;
}

static void evaluateLeaf(LSL_Leaf *leaf, LSL_Leaf *left, LSL_Leaf *right)
{
    if (leaf)
    {
	LSL_Leaf lresult;
	LSL_Leaf rresult;

	memcpy(&lresult, left, sizeof(LSL_Leaf));
	memcpy(&rresult, right, sizeof(LSL_Leaf));

	if (LSL_RIGHT2LEFT & leaf->token->flags)
	{
	    memcpy(&rresult, left, sizeof(LSL_Leaf));
	    evaluateLeaf(leaf->right, &rresult, right);
	    if (!(LSL_UNARY & leaf->token->flags))
	    {
		evaluateLeaf(leaf->left, &lresult, right);
	    }
	}
	else // Assume left to right.
	{
	    evaluateLeaf(leaf->left, &lresult, right);
	    if (!(LSL_UNARY & leaf->token->flags))
	    {
		memcpy(&rresult, left, sizeof(LSL_Leaf));
		evaluateLeaf(leaf->right, &rresult, right);
	    }
	}

	if (leaf->token->evaluate)
	{
	    leaf->token->evaluate(leaf, &lresult, &rresult);
	    memcpy(left, &lresult, sizeof(LSL_Leaf));
	}
	else
	{
#ifdef LUASL_DEBUG
	    printf(" eval <%s %d %d %d %d> ", leaf->token->token, left->value.integerValue, right->value.integerValue, lresult.value.integerValue, rresult.value.integerValue);
#endif
	    memcpy(left, &rresult, sizeof(LSL_Leaf));
	}
    }
}

static void evaluateIntegerToken(LSL_Leaf *content, LSL_Leaf *left, LSL_Leaf *right)
{
    if (content)
    {
#ifdef LUASL_DEBUG
	printf(" <%d> ", content->value.integerValue);
#endif
	left->value.integerValue = content->value.integerValue;
	left->token = tokens[LSL_INTEGER - lowestToken];
    }
}

static void evaluateNoToken(LSL_Leaf *content, LSL_Leaf *left, LSL_Leaf *right)
{
    // Do nothing, that's the point.
}

static void evaluateOperationToken(LSL_Leaf *content, LSL_Leaf *left, LSL_Leaf *right)
{
    if (content)
    {
#ifdef LUASL_DEBUG
	printf(" [%s] ", content->token->token);
#endif

	switch (content->token->type)
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
	    case LSL_BIT_NOT		:  left->value.integerValue = ~ right->value.integerValue;				break;
	    case LSL_BOOL_NOT		:  left->value.integerValue = ! right->value.integerValue;				break;
	    case LSL_NEGATION		:  left->value.integerValue = 0 - right->value.integerValue;				break;
	    case LSL_DIVIDE		:  left->value.integerValue = left->value.integerValue /  right->value.integerValue;	break;
	    case LSL_MODULO		:  left->value.integerValue = left->value.integerValue %  right->value.integerValue;	break;
	    case LSL_MULTIPLY		:  left->value.integerValue = left->value.integerValue *  right->value.integerValue;	break;
//	    case LSL_DOT_PRODUCT	: break;
//	    case LSL_CROSS_PRODUCT	: break;
	    case LSL_SUBTRACT		:  left->value.integerValue = left->value.integerValue -  right->value.integerValue;	break;
	    case LSL_ADD		:  left->value.integerValue = left->value.integerValue +  right->value.integerValue;	break;
//	    case LSL_CONCATENATE	: break;
	    case LSL_LEFT_SHIFT		:  left->value.integerValue = left->value.integerValue << right->value.integerValue;	break;
	    case LSL_RIGHT_SHIFT	:  left->value.integerValue = left->value.integerValue >> right->value.integerValue;	break;
	    case LSL_LESS_THAN		:  left->value.integerValue = left->value.integerValue <  right->value.integerValue;	break;
	    case LSL_GREATER_THAN	:  left->value.integerValue = left->value.integerValue >  right->value.integerValue;	break;
	    case LSL_LESS_EQUAL		:  left->value.integerValue = left->value.integerValue <= right->value.integerValue;	break;
	    case LSL_GREATER_EQUAL	:  left->value.integerValue = left->value.integerValue >= right->value.integerValue;	break;
	    case LSL_EQUAL		:  left->value.integerValue = left->value.integerValue == right->value.integerValue;	break;
	    case LSL_NOT_EQUAL		:  left->value.integerValue = left->value.integerValue != right->value.integerValue;	break;
	    case LSL_BIT_AND		:  left->value.integerValue = left->value.integerValue &  right->value.integerValue;	break;
	    case LSL_BIT_XOR		:  left->value.integerValue = left->value.integerValue ^  right->value.integerValue;	break;
	    case LSL_BIT_OR		:  left->value.integerValue = left->value.integerValue |  right->value.integerValue;	break;
	    case LSL_BOOL_OR		:  left->value.integerValue = left->value.integerValue || right->value.integerValue;	break;
	    case LSL_BOOL_AND		:  left->value.integerValue = left->value.integerValue && right->value.integerValue;	break;
	}
#ifdef LUASL_DEBUG
	printf(" (=%d) ", left->value.integerValue);
#endif
    }
}

static void eveluateParenthesisToken(LSL_Leaf *content, LSL_Leaf *left, LSL_Leaf *right)
{
    if (content)
	evaluateLeaf(content->value.parenthesis->expression, left, right);
}


static void evaluateStatementToken(LSL_Leaf *content, LSL_Leaf *left, LSL_Leaf *right)
{
    if (content)
    {
	evaluateLeaf(content->value.statementValue->expressions, left, right);
	printf("\nResult is %d.\n", left->value.integerValue);
	left->value.integerValue = 0;
	right->value.integerValue = 0;
    }
}

static void outputLeaf(FILE *file, LSL_Leaf *leaf)
{
    if (leaf)
    {
	outputLeaf(file, leaf->left);
	if ((!(LSL_NOIGNORE & leaf->token->flags)) && (leaf->ignorableText))
	    fprintf(file, "%s", leaf->ignorableText);
	if (leaf->token->output)
	    leaf->token->output(file, leaf);
	else
	    fprintf(file, "%s", leaf->token->token);
	outputLeaf(file, leaf->right);
    }
}

static void outputIntegerToken(FILE *file, LSL_Leaf *content)
{
    if (content)
	fprintf(file, "%d", content->value.integerValue);
}

static void outputParenthesisToken(FILE *file, LSL_Leaf *content)
{
    if (content)
    {
	fprintf(file, "%s", content->token->token);
	outputLeaf(file, content->value.parenthesis->expression);
	outputLeaf(file, content->value.parenthesis->right);
    }
}

static void outputStatementToken(FILE *file, LSL_Leaf *content)
{
    if (content)
    {
	outputLeaf(file, content->value.statementValue->expressions);
	if (content->ignorableText)
	    fprintf(file, "%s", content->ignorableText);
	fprintf(file, "%s", content->token->token);
    }
}

static void convertLeaf2Lua(FILE *file, LSL_Leaf *leaf)
{
    if (leaf)
    {
	convertLeaf2Lua(file, leaf->left);
	if ((!(LSL_NOIGNORE & leaf->token->flags)) && (leaf->ignorableText))
	    fprintf(file, "%s", leaf->ignorableText);
	if (leaf->token->convert)
	    leaf->token->convert(file, leaf);
	else if (leaf->token->output)
	    leaf->token->output(file, leaf);
	else
	    fprintf(file, "%s", leaf->token->token);
	convertLeaf2Lua(file, leaf->right);
    }
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
	char buffer[PATH_MAX];
	char fileName[PATH_MAX];
	LuaSL_yyparseParam param;
	int file;
	int count;
	boolean badArgs = FALSE;

	// Sort the token table.
	for (i = 0; LSL_Tokens[i].token != NULL; i++)
	{
	    int j = LSL_Tokens[i].type - lowestToken;

	    tokens[j] = &(LSL_Tokens[i]);
	}

	fileName[0] = '\0';

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
			    strncpy(fileName, *argv, PATH_MAX - 1);
			    fileName[PATH_MAX - 1] = '\0';;
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

	if ('\0' == fileName[0])
	{
//strcpy(fileName, "test.lsl");

	    count = read(STDIN_FILENO, fileName, PATH_MAX - 1);
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
		fileName[count] = '\0';
		printf("Filename %s in stdin.\n", fileName);
	    }

	}
	else
	    printf("Filename %s in argument.\n", fileName);

	file = open(fileName, 0);
	if (-1 == file)
	{
	    printf("Error opening file %s.\n", fileName);
	    return 1;
	}
#ifdef LUASL_DEBUG
//	yydebug= 5;
#endif

	param.ast = NULL;
	param.lval = calloc(1, sizeof(LSL_Leaf));
	if (yylex_init(&(param.scanner)))
	    return 1;

#ifdef LUASL_DEBUG
	yyset_debug(1, param.scanner);
#endif
//	yyset_in(file, param.scanner);

	{
	    void *pParser = ParseAlloc(malloc);
	    int yv;

	    ParseTrace(stdout, "LSL_lemon ");

	    while ((i = read(file, buffer, PATH_MAX - 1)) >  0)
	    {
		buffer[i] = '\0';
		yy_scan_string(buffer, param.scanner);
		// on EOF yylex will return 0
		while((yv = yylex(param.lval, param.scanner)) != 0)
		{
		    Parse(pParser, yv, param.lval, &param);
		    if (LSL_SCRIPT == yv)
			break;
		    param.lval = calloc(1, sizeof(LSL_Leaf));
		}
	    }

	    yylex_destroy(param.scanner);
	    Parse (pParser, 0, param.lval, &param);
	    ParseFree(pParser, free);

	    if (param.ast)
	    {
		FILE *out;
		char outName[PATH_MAX];
		char luaName[PATH_MAX];

		LSL_Leaf left, right;

		left.value.integerValue = 0;
		left.token = tokens[LSL_INTEGER - lowestToken];
		right.value.integerValue = 0;
		right.token = tokens[LSL_INTEGER - lowestToken];

		outputLeaf(stdout, param.ast);
		printf("\n");
		evaluateLeaf(param.ast, &left, &right);
		printf("\n");

		strcpy(outName, fileName);
		strcat(outName, "2");
		strcpy(luaName, fileName);
		strcat(luaName, ".lua");
		out = fopen(outName, "w");
		if (out)
		{
		    outputLeaf(out, param.ast);
		    fclose(out);
		    sprintf(buffer, "diff %s %s", fileName, outName);
		    count = system(buffer);
		    printf("Return value of %s is %d\n", buffer, count);
		    if (0 != count}
			printf(stderr, "%s says they are different!\n", buffer);
		    
		}
		else
		    fprintf(stderr, "Unable to open file %s for writing!\n", outName);
		out = fopen(luaName, "w");
		if (out)
		{
		    convertLeaf2Lua(out, param.ast);
		    fclose(out);
		}
		else
		    fprintf(stderr, "Unable to open file %s for writing!\n", luaName);
		burnLeaf(param.ast);
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

