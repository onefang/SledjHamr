#include "LuaSL.h"


static LSL_Leaf *evaluateFloatToken(LSL_Leaf  *content, LSL_Leaf *left, LSL_Leaf *right);
static LSL_Leaf *evaluateIntegerToken(LSL_Leaf  *content, LSL_Leaf *left, LSL_Leaf *right);
static LSL_Leaf *evaluateNoToken(LSL_Leaf *content, LSL_Leaf *left, LSL_Leaf *right);
static LSL_Leaf *evaluateOperationToken(LSL_Leaf  *content, LSL_Leaf *left, LSL_Leaf *right);
static LSL_Leaf *eveluateParenthesisToken(LSL_Leaf *content, LSL_Leaf *left, LSL_Leaf *right);
static LSL_Leaf *evaluateStatementToken(LSL_Leaf *content, LSL_Leaf *left, LSL_Leaf *right);
static void outputFloatToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputFunctionToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputIntegerToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputIdentifierToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputParameterListToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputParenthesisToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputStateToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputStatementToken(FILE *file, outputMode mode, LSL_Leaf *content);

LSL_Token LSL_Tokens[] =
{
    // Various forms of "space".
    {LSL_COMMENT,		ST_NONE,		"/*",	LSL_NONE,				NULL, NULL},
    {LSL_COMMENT_LINE,		ST_NONE,		"//",	LSL_NONE,				NULL, NULL},
    {LSL_SPACE,			ST_NONE,		" ",	LSL_NONE,				NULL, NULL},

    // Operators, in order of precedence, low to high
    // Left to right, unless otherwise stated.
    // According to http://wiki.secondlife.com/wiki/Category:LSL_Operators
    {LSL_BOOL_AND,		ST_BOOLEAN,		"&&",	LSL_RIGHT2LEFT,				NULL, evaluateOperationToken},
// QUIRK - Seems to be some disagreement about BOOL_AND/BOOL_OR precedence.  Either they are equal, or OR is higher.
// QUIRK - No boolean short circuiting.
// QUIRK - Booleans and conditionals are executed right to left.  Or maybe not, depending on who you believe.
    {LSL_BOOL_OR,		ST_BOOLEAN,		"||",	LSL_RIGHT2LEFT,				NULL, evaluateOperationToken},
    {LSL_BIT_OR,		ST_BITWISE,		"|",	LSL_LEFT2RIGHT,				NULL, evaluateOperationToken},
    {LSL_BIT_XOR,		ST_BITWISE,		"^",	LSL_LEFT2RIGHT,				NULL, evaluateOperationToken},
    {LSL_BIT_AND,		ST_BITWISE,		"&",	LSL_LEFT2RIGHT,				NULL, evaluateOperationToken},
// QUIRK - Booleans and conditionals are executed right to left.  Or maybe not, depending on who you believe.
    {LSL_NOT_EQUAL,		ST_EQUALITY,		"!=",	LSL_RIGHT2LEFT,				NULL, evaluateOperationToken},
    {LSL_EQUAL,			ST_EQUALITY,		"==",	LSL_RIGHT2LEFT,				NULL, evaluateOperationToken},
    {LSL_GREATER_EQUAL,		ST_COMPARISON,		">=",	LSL_RIGHT2LEFT,				NULL, evaluateOperationToken},
    {LSL_LESS_EQUAL,		ST_COMPARISON,		"<=",	LSL_RIGHT2LEFT,				NULL, evaluateOperationToken},
    {LSL_GREATER_THAN,		ST_COMPARISON,		">",	LSL_RIGHT2LEFT,				NULL, evaluateOperationToken},
    {LSL_LESS_THAN,		ST_COMPARISON,		"<",	LSL_RIGHT2LEFT,				NULL, evaluateOperationToken},
    {LSL_RIGHT_SHIFT,		ST_BITWISE,		">>",	LSL_LEFT2RIGHT,				NULL, evaluateOperationToken},
    {LSL_LEFT_SHIFT,		ST_BITWISE,		"<<",	LSL_LEFT2RIGHT,				NULL, evaluateOperationToken},
    {LSL_CONCATENATE,		ST_ADD,			"+",	LSL_LEFT2RIGHT,				NULL, evaluateOperationToken},
    {LSL_ADD,			ST_ADD,			"+",	LSL_LEFT2RIGHT,				NULL, evaluateOperationToken},
    {LSL_SUBTRACT,		ST_SUBTRACT,		"-",	LSL_LEFT2RIGHT,				NULL, evaluateOperationToken},
    {LSL_CROSS_PRODUCT,		ST_NONE,		"%",	LSL_LEFT2RIGHT,				NULL, evaluateOperationToken},
    {LSL_DOT_PRODUCT,		ST_NONE,		"*",	LSL_LEFT2RIGHT,				NULL, evaluateOperationToken},
    {LSL_MULTIPLY,		ST_MULTIPLY,		"*",	LSL_LEFT2RIGHT,				NULL, evaluateOperationToken},
    {LSL_MODULO,		ST_MODULO,		"%",	LSL_LEFT2RIGHT,				NULL, evaluateOperationToken},
    {LSL_DIVIDE,		ST_MULTIPLY,		"/",	LSL_LEFT2RIGHT,				NULL, evaluateOperationToken},
    {LSL_NEGATION,		ST_NEGATE,		"-",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, evaluateOperationToken},
    {LSL_BOOL_NOT,		ST_BOOL_NOT,		"!",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, evaluateOperationToken},
    {LSL_BIT_NOT,		ST_BIT_NOT,		"~",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, evaluateOperationToken},
    {LSL_TYPECAST_CLOSE,	ST_NONE,		")",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, evaluateNoToken},
    {LSL_TYPECAST_OPEN,		ST_NONE,		"(",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, evaluateOperationToken},
    {LSL_ANGLE_CLOSE,		ST_NONE,		">",	LSL_LEFT2RIGHT | LSL_CREATION,		NULL, evaluateNoToken},
    {LSL_ANGLE_OPEN,		ST_NONE,		"<",	LSL_LEFT2RIGHT | LSL_CREATION,		NULL, evaluateOperationToken},
    {LSL_BRACKET_CLOSE,		ST_NONE,		"]",	LSL_INNER2OUTER | LSL_CREATION,		NULL, evaluateNoToken},
    {LSL_BRACKET_OPEN,		ST_NONE,		"[",	LSL_INNER2OUTER | LSL_CREATION,		NULL, evaluateOperationToken},
    {LSL_PARENTHESIS_CLOSE,	ST_NONE,		")",	LSL_INNER2OUTER,			NULL, evaluateNoToken},
    {LSL_PARENTHESIS_OPEN,	ST_NONE,		"(",	LSL_INNER2OUTER,			outputParenthesisToken, eveluateParenthesisToken},
    {LSL_ASSIGNMENT_CONCATENATE,ST_CONCATENATION,	"+=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL, evaluateOperationToken},
    {LSL_ASSIGNMENT_ADD,	ST_CONCATENATION,	"+=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL, evaluateOperationToken},
    {LSL_ASSIGNMENT_SUBTRACT,	ST_ASSIGNMENT,		"-=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL, evaluateOperationToken},
    {LSL_ASSIGNMENT_MULTIPLY,	ST_ASSIGNMENT,		"*=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL, evaluateOperationToken},
    {LSL_ASSIGNMENT_MODULO,	ST_MODULO,		"%=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL, evaluateOperationToken},
    {LSL_ASSIGNMENT_DIVIDE,	ST_ASSIGNMENT,		"/=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL, evaluateOperationToken},
    {LSL_ASSIGNMENT_PLAIN,	ST_CONCATENATION,	"=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL, evaluateOperationToken},
    {LSL_DOT,			ST_NONE,		".",	LSL_RIGHT2LEFT,				NULL, evaluateOperationToken},
    {LSL_DECREMENT_POST,	ST_NONE,		"--",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, evaluateOperationToken},
    {LSL_DECREMENT_PRE,		ST_NONE,		"--",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, evaluateOperationToken},
    {LSL_INCREMENT_POST,	ST_NONE,		"++",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, evaluateOperationToken},
    {LSL_INCREMENT_PRE,		ST_NONE,		"++",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, evaluateOperationToken},
    {LSL_COMMA,			ST_NONE,		",",	LSL_LEFT2RIGHT,				NULL, evaluateOperationToken},

    {LSL_EXPRESSION,		ST_NONE,	"expression",	LSL_NONE	,			NULL, NULL},

    // Types.
    {LSL_FLOAT,			ST_NONE,	"float",	LSL_NONE,				outputFloatToken, evaluateFloatToken},
    {LSL_INTEGER,		ST_NONE,	"integer",	LSL_NONE,				outputIntegerToken, evaluateIntegerToken},
    {LSL_KEY,			ST_NONE,	"key",		LSL_NONE,				NULL, NULL},
    {LSL_LIST,			ST_NONE,	"list",		LSL_NONE,				NULL, NULL},
    {LSL_ROTATION,		ST_NONE,	"rotation",	LSL_NONE,				NULL, NULL},
    {LSL_STRING,		ST_NONE,	"string",	LSL_NONE,				NULL, NULL},
    {LSL_VECTOR,		ST_NONE,	"vector",	LSL_NONE,				NULL, NULL},

    // Types names.
    {LSL_TYPE_FLOAT,		ST_NONE,	"float",	LSL_NONE,				NULL, NULL},
    {LSL_TYPE_INTEGER,		ST_NONE,	"integer",	LSL_NONE,				NULL, NULL},
    {LSL_TYPE_KEY,		ST_NONE,	"key",		LSL_NONE,				NULL, NULL},
    {LSL_TYPE_LIST,		ST_NONE,	"list",		LSL_NONE,				NULL, NULL},
    {LSL_TYPE_ROTATION,		ST_NONE,	"rotation",	LSL_NONE,				NULL, NULL},
    {LSL_TYPE_STRING,		ST_NONE,	"string",	LSL_NONE,				NULL, NULL},
    {LSL_TYPE_VECTOR,		ST_NONE,	"vector",	LSL_NONE,				NULL, NULL},

    // Then the rest of the syntax tokens.
    {LSL_IDENTIFIER,		ST_NONE,	"identifier",	LSL_NONE,				outputIdentifierToken, NULL},

    {LSL_LABEL,			ST_NONE,	"@",		LSL_NONE,				NULL, NULL},

    {LSL_DO,			ST_NONE,	"do",		LSL_NONE,				NULL, NULL},
    {LSL_FOR,			ST_NONE,	"for",		LSL_NONE,				NULL, NULL},
    {LSL_ELSE_IF,		ST_NONE,	"else if",	LSL_NONE,				NULL, NULL},
    {LSL_ELSE,			ST_NONE,	"else",		LSL_NONE,				NULL, NULL},
    {LSL_IF,			ST_NONE,	"if",		LSL_NONE,				NULL, NULL},
    {LSL_JUMP,			ST_NONE,	"jump",		LSL_NONE,				NULL, NULL},
    {LSL_RETURN,		ST_NONE,	"return",	LSL_NONE,				NULL, NULL},
    {LSL_STATE_CHANGE,		ST_NONE,	"state",	LSL_NONE,				NULL, NULL},
    {LSL_WHILE,			ST_NONE,	"while",	LSL_NONE,				NULL, NULL},
    {LSL_STATEMENT,		ST_NONE,	";",		LSL_NOIGNORE,				outputStatementToken, evaluateStatementToken},

    {LSL_BLOCK_CLOSE,		ST_NONE,	"}",		LSL_NONE,				NULL, NULL},
    {LSL_BLOCK_OPEN,		ST_NONE,	"{",		LSL_NONE,				NULL, NULL},
    {LSL_PARAMETER,		ST_NONE,	"parameter",	LSL_NONE,				outputIdentifierToken, NULL},
    {LSL_PARAMETER_LIST,	ST_NONE,	"plist",	LSL_NONE,				outputParameterListToken, NULL},
    {LSL_FUNCTION,		ST_NONE,	"function",	LSL_NONE,				outputFunctionToken, NULL},
    {LSL_STATE,			ST_NONE,	"state",	LSL_NONE,				outputStateToken, NULL},
    {LSL_SCRIPT,		ST_NONE,	"",		LSL_NONE,				NULL,  NULL},

    {LSL_UNKNOWN,		ST_NONE,	"unknown",	LSL_NONE,				NULL, NULL},

    // A sentinal.
    {999999, ST_NONE, NULL, LSL_NONE, NULL, NULL}
};

allowedTypes allowed[] = 
{
    {OT_nothing,	"nothing",	(ST_NONE)},

    {OT_bool,		"boolean",	(ST_BOOL_NOT)},
    {OT_integer,	"integer",	(ST_BIT_NOT | ST_NEGATE)},
    {OT_float,		"float",	(ST_NONE)},
    {OT_key,		"key",		(ST_NONE)},
    {OT_list,		"list",		(ST_NONE)},
    {OT_rotation,	"rotation",	(ST_NONE)},
    {OT_string,		"string",	(ST_NONE)},
    {OT_vector,		"vector",	(ST_NONE)},
    {OT_other,		"other",	(ST_NONE)},

    {OT_bool,		"boolean",	(ST_BOOLEAN | ST_EQUALITY)},
    {OT_integer,	"integer",	(ST_MULTIPLY | ST_ADD | ST_SUBTRACT | ST_EQUALITY | ST_COMPARISON | ST_CONCATENATION | ST_ASSIGNMENT | ST_MODULO | ST_BITWISE)},
    {OT_float,		"float",	(ST_MULTIPLY | ST_ADD | ST_SUBTRACT | ST_EQUALITY | ST_COMPARISON | ST_CONCATENATION | ST_ASSIGNMENT)},
    {OT_float,		"float",	(ST_MULTIPLY | ST_ADD | ST_SUBTRACT | ST_EQUALITY | ST_COMPARISON | ST_CONCATENATION | ST_ASSIGNMENT)},
    {OT_float,		"float",	(ST_MULTIPLY | ST_ADD | ST_SUBTRACT | ST_EQUALITY | ST_COMPARISON | ST_CONCATENATION | ST_ASSIGNMENT)},
    {OT_string,		"string",	(ST_ADD | ST_EQUALITY | ST_CONCATENATION)},
    {OT_string,		"string",	(ST_ADD | ST_EQUALITY | ST_CONCATENATION)},
    {OT_string,		"string",	(ST_ADD | ST_EQUALITY | ST_CONCATENATION)},
    {OT_list,		"list",		(ST_ADD | ST_EQUALITY | ST_CONCATENATION)},
    {OT_list,		"list",		(ST_ADD | ST_COMPARISON | ST_CONCATENATION)},
    {OT_list,		"list",		(ST_ADD | ST_COMPARISON | ST_CONCATENATION)},
    {OT_integer,	"integer",	(ST_ADD | ST_COMPARISON)},
    {OT_float,		"float",	(ST_ADD | ST_COMPARISON)},
    {OT_list,		"list",		(ST_ADD | ST_CONCATENATION)},
    {OT_vector,		"vector",	(ST_MULTIPLY | ST_ADD | ST_SUBTRACT | ST_EQUALITY | ST_CONCATENATION | ST_ASSIGNMENT | ST_MODULO)},
    {OT_vector,		"vector",	(ST_MULTIPLY)},
    {OT_vector,		"vector",	(ST_MULTIPLY)},
    {OT_rotation,	"rotation",	(ST_MULTIPLY | ST_ADD | ST_SUBTRACT | ST_EQUALITY | ST_CONCATENATION | ST_ASSIGNMENT)},
    {OT_other,		"other",	(ST_NONE)},
    {OT_invalid,	"invalid",	(ST_NONE)}
};

opType opExpr[][10] =
{
    {OT_nothing,  OT_bool,     OT_integer,  OT_float,       OT_key,       OT_list,      OT_rotation,         OT_string,       OT_vector,       OT_other},
    {OT_bool,     OT_boolBool, OT_invalid,  OT_invalid,     OT_invalid,   OT_invalid,   OT_invalid,          OT_invalid,      OT_invalid,      OT_invalid},
    {OT_integer,  OT_invalid,  OT_intInt,   OT_intFloat,    OT_invalid,   OT_intList,   OT_invalid,          OT_invalid,      OT_invalid,      OT_invalid},
    {OT_float,    OT_invalid,  OT_floatInt, OT_floatFloat,  OT_invalid,   OT_floatList, OT_invalid,          OT_invalid,      OT_invalid,      OT_invalid},
    {OT_key,      OT_invalid,  OT_invalid,  OT_invalid,     OT_invalid,   OT_invalid,   OT_invalid,          OT_keyString,    OT_invalid,      OT_invalid},
    {OT_list,     OT_invalid,  OT_listInt,  OT_listFloat,   OT_invalid,   OT_listList,  OT_invalid,          OT_invalid,      OT_invalid,      OT_listOther},
    {OT_rotation, OT_invalid,  OT_invalid,  OT_invalid,     OT_invalid,   OT_invalid,   OT_rotationRotation, OT_invalid,      OT_invalid,      OT_invalid},
    {OT_string,   OT_invalid,  OT_invalid,  OT_invalid,     OT_stringKey, OT_invalid,   OT_invalid,          OT_stringString, OT_invalid,      OT_invalid},
    {OT_vector,   OT_invalid,  OT_invalid,  OT_vectorFloat, OT_invalid,   OT_invalid,   OT_vectorRotation,   OT_invalid,      OT_vectorVector, OT_invalid},
    {OT_other,    OT_invalid,  OT_invalid,  OT_invalid,     OT_invalid,   OT_invalid,   OT_invalid,          OT_invalid,      OT_invalid,      OT_otherOther}

};


LSL_Token **tokens = NULL;
int lowestToken = 999999;


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

LSL_Leaf *addOperation(LuaSL_compiler *compiler, LSL_Leaf *left, LSL_Leaf *lval, LSL_Leaf *right)
{
    gameGlobals *game = compiler->game;

    if (lval)
    {
	opType lType, rType;

	lval->left = left;
	lval->right = right;

	// Try to figure out what type of operation this is.
	if (NULL == left)
	    lType = OT_nothing;
	else
	{
	    lType = left->basicType;
	    if (OT_vector < lType)
		lType = allowed[lType].result;
	}
	if (NULL == right)
	    rType = OT_nothing;
	else
	{
	    rType = right->basicType;
	    if (OT_vector < rType)
		rType = allowed[rType].result;
	}

	// The basic lookup.
	lval->basicType = opExpr[lType][rType];
	if (OT_invalid != lval->basicType)
	{
	    // Check if it's an allowed operation.
	    if (0 == (lval->token->subType & allowed[lval->basicType].subTypes))
	    {
		lval->basicType = OT_invalid;
	    }
	    else
	    {
		// Double check the corner cases.
		switch (lval->token->subType)
		{
		    case ST_BOOLEAN :
			lval->basicType = OT_bool;
			break;
		    case ST_COMPARISON :
			lval->basicType = OT_bool;
			break;
		    case ST_MULTIPLY :
			if (OT_vectorVector == lval->basicType)
			{
			    if (LSL_MULTIPLY == lval->token->type)
			    {
				lval->basicType = OT_float;
			    	lval->token = tokens[LSL_DOT_PRODUCT - lowestToken];
			    }
			    else
				lval->basicType = OT_vector;
			}
			break;
		    default :
			break;
		}
	    }
	}
	if (OT_invalid == lval->basicType)
	{
	    const char *leftType = "", *rightType = "";

	    if (left)
		leftType = allowed[left->basicType].name;
	    if (right)
		rightType = allowed[right->basicType].name;

	    PE("Invalid operation [%s %s %s] @ line %d column %d", leftType, lval->token->token, rightType, lval->line, lval->column);
	}
    }

    return lval;
}

LSL_Leaf *addParameter(LSL_Leaf *type, LSL_Leaf *identifier)
{
    LSL_Identifier *result = calloc(1, sizeof(LSL_Identifier));

    if ( (identifier) && (result))
    {
	result->name = identifier->value.stringValue;
	identifier->value.identifierValue = result;
	identifier->token = tokens[LSL_PARAMETER - lowestToken];
	identifier->left = type;
	if (type)
	{
	    identifier->basicType = type->basicType;
	    result->value.basicType = type->basicType;
	}
    }
    return identifier;
}

LSL_Leaf *collectParameters(LSL_Leaf *list, LSL_Leaf *comma, LSL_Leaf *newParam)
{
    LSL_Leaf *newList = newLeaf(LSL_PARAMETER_LIST, NULL, NULL);

    if (newList)
    {
	newList->left = list;
	newList->value.listValue = newParam;
	if ((list) && (list->value.listValue))
	{
	    list->value.listValue->right = comma;
	}
    }

    return newList;
}

LSL_Leaf *addFunction(LSL_Leaf *type, LSL_Leaf *identifier, LSL_Leaf *open, LSL_Leaf *params, LSL_Leaf *close, LSL_Leaf *block)
{
    LSL_Function *func = calloc(1, sizeof(LSL_Function));

    if (func)
    {
	if (identifier)
	{
	    const char *temp = identifier->value.stringValue;

	    identifier->token = tokens[LSL_FUNCTION - lowestToken];
	    identifier->value.functionValue = func;
	    identifier->value.functionValue->name = temp;
	    identifier->value.functionValue->block = block;
	    func->type = type;
	    if (type)
		identifier->basicType = type->basicType;
	    else
		identifier->basicType = OT_nothing;
	    func->params = addParenthesis(open, params, LSL_PARAMETER_LIST, close);
	}
    }
    return identifier;
}

LSL_Leaf *addParenthesis(LSL_Leaf *lval, LSL_Leaf *expr, LSL_Type type, LSL_Leaf *rval)
{
    LSL_Parenthesis *parens = malloc(sizeof(LSL_Parenthesis));

    if (parens)
    {
	parens->left = lval;
	parens->contents = expr;
	parens->type = type;
	parens->right = rval;
	if (lval)
	{
	    lval->value.parenthesis = parens;
	    if (expr)
		lval->basicType = expr->basicType;
	}
    }
    return lval;
}

LSL_Leaf *addState(LuaSL_compiler *compiler, LSL_Leaf *identifier, LSL_Leaf *block)
{
    LSL_State *result = calloc(1, sizeof(LSL_State));

    if ((identifier) && (result))
    {
	result->name = identifier->value.stringValue;
	result->block = block;
	identifier->value.stateValue = result;
	compiler->script.scount++;
	compiler->script.states = realloc(compiler->script.states, compiler->script.scount * sizeof(LSL_State *));
	compiler->script.states[compiler->script.scount - 1] = result;
    }

    return identifier;
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

LSL_Leaf *addTypecast(LSL_Leaf *lval, LSL_Leaf *type, LSL_Leaf *rval, LSL_Leaf *expr)
{
    LSL_Parenthesis *parens = malloc(sizeof(LSL_Parenthesis));

    if (parens)
    {
	parens->left = lval;
	parens->contents = expr;
	parens->type = LSL_TYPECAST_OPEN;
	parens->right = rval;
	if (lval)
	{
	    lval->value.parenthesis = parens;
	    if (type)
		lval->basicType = type->basicType;
	    lval->token = tokens[LSL_TYPECAST_OPEN - lowestToken];
	}
	if (rval)
	{
	    rval->token = tokens[LSL_TYPECAST_CLOSE - lowestToken];
	}
    }
    return lval;
}

LSL_Leaf *addVariable(LuaSL_compiler *compiler, LSL_Leaf *type, LSL_Leaf *identifier, LSL_Leaf *assignment, LSL_Leaf *expr)
{
    LSL_Identifier *result = calloc(1, sizeof(LSL_Identifier));

    if ( (identifier) && (result))
    {
	result->name = identifier->value.stringValue;
	identifier->value.identifierValue = result;
	identifier->left = type;
	identifier->right = assignment;
	if (assignment)
	    assignment->right = expr;
	if (type)
	{
	    identifier->basicType = type->basicType;
	    result->value.basicType = type->basicType;
	}
	if (compiler->currentBlock)
	{
	    compiler->currentBlock->vcount++;
	    compiler->currentBlock->variables = realloc(compiler->currentBlock->variables, compiler->currentBlock->vcount * sizeof(LSL_Identifier *));
	    compiler->currentBlock->variables[compiler->currentBlock->vcount - 1] = result;
	}
	else
	{
	    compiler->script.vcount++;
	    compiler->script.variables = realloc(compiler->script.variables, compiler->script.vcount * sizeof(LSL_Identifier *));
	    compiler->script.variables[compiler->script.vcount - 1] = result;
	}
    }

    return identifier;
}

void beginBlock(LuaSL_compiler *compiler, LSL_Leaf *block)
{
    LSL_Block *blok = malloc(sizeof(LSL_Block));

    if (blok)
    {
	block->value.blockValue = blok;
	blok->outerBlock = compiler->currentBlock;
	compiler->currentBlock = blok;
    }
}

void endBlock(LuaSL_compiler *compiler, LSL_Leaf *block)
{
    compiler->currentBlock = compiler->currentBlock->outerBlock;
}

static LSL_Leaf *evaluateLeaf(LSL_Leaf *leaf, LSL_Leaf *left, LSL_Leaf *right)
{
    LSL_Leaf *result = NULL;

    if (leaf)
    {
	LSL_Leaf *lresult = NULL;
	LSL_Leaf *rresult = NULL;

	if (LSL_RIGHT2LEFT & leaf->token->flags)
	{
	    rresult = evaluateLeaf(leaf->right, left, right);
	    if (!(LSL_UNARY & leaf->token->flags))
		lresult = evaluateLeaf(leaf->left, left, right);
	}
	else // Assume left to right.
	{
	    lresult = evaluateLeaf(leaf->left, left, right);
	    if (!(LSL_UNARY & leaf->token->flags))
		rresult = evaluateLeaf(leaf->right, left, right);
	}

	if (leaf->token->evaluate)
	    result = leaf->token->evaluate(leaf, lresult, rresult);
	else
	{
	    result = calloc(1, sizeof(LSL_Leaf));
	    if (rresult && result)
		memcpy(result, rresult, sizeof(LSL_Leaf));
	}

	if (lresult)
	    free(lresult);
	if (rresult)
	    free(rresult);
    }

    return result;
}

static LSL_Leaf *evaluateFloatToken(LSL_Leaf *content, LSL_Leaf *left, LSL_Leaf *right)
{
    LSL_Leaf *result = malloc(sizeof(LSL_Leaf));

    if (content && result)
    {
#ifdef LUASL_DEBUG
	printf(" <%g> ", content->value.floatValue);
#endif
	memcpy(result, content, sizeof(LSL_Leaf));
	result->basicType = OT_float;
    }
    return result;
}

static LSL_Leaf *evaluateIntegerToken(LSL_Leaf *content, LSL_Leaf *left, LSL_Leaf *right)
{
    LSL_Leaf *result = malloc(sizeof(LSL_Leaf));

    if (content && result)
    {
#ifdef LUASL_DEBUG
	printf(" <%d> ", content->value.integerValue);
#endif
	memcpy(result, content, sizeof(LSL_Leaf));
	result->basicType = OT_integer;
    }
    return result;
}

static LSL_Leaf *evaluateNoToken(LSL_Leaf *content, LSL_Leaf *left, LSL_Leaf *right)
{
    // Do nothing, that's the point.

    return content;
}

/*  Typecasting

LSL is statically typed, so stored values are not converted, only the values used in expressions are.
Lua is dynamically typed, so stored values are changed (sometimes I think).

LSL implicitly typecasts - There is a shitload of QUIRKs about this.  Apparently some don't work anyway.
			integer -> float  (Says in lslwiki that precision is never lost, which is bullshit, since they are both 32 bit.  Would be true if the float is 64 bit.  Lua suggest to use 64 bit floats to emulate 32 bit integers.)
			string  -> key
			    Some functions need help with this or the other way around.
			string  -> vector (Maybe, should test that.)
			vector  -> string (Maybe, should test that.)
	Also happens when getting stuff from lists.

Explicit type casting -
			string -> integer
			    Leading spaces are ignored, as are any characters after the run of digits.
			    All other strings convert to 0.
			    Which means "" and " " convert to 0.
			    Strings in hexadecimal format will work.
			keys <-> string
			    No other typecasting can be done with keys.
			float -> string
			    You get a bunch of trailing 0s.

QUIRK - I have seen cases where a double explicit typecast was needed in SL, but was considered to be invalid syntax in OS.

Any binary operation involving a float and an integer implicitly casts the integer to float.

A boolean operation deals with TRUE (1) and FALSE (0).  Any non zero value is a TRUE (generally sigh).
Bitwise operations only apply to integers.  The shifts are arithmatic, not logical.  Right shifted bits are dropped, left shifts the sign bit.

integer  = integer0   % integer1;  // Apparently only applies to integers, but works fine on floats in OS.
string   = string0    + string1;   // Concatenation.
list     = list0      + list1;     // Concatenation.   Also works if either is not a list, it's promoted to a list first.
list     = (list=[])  + list + ["new_item"];  // Voodoo needed for old LSL, works in Mono but not needed, does not work in OS.  Works for strings to.
bool     = list == != int          // Only compares the lengths, probably applies to the other conditionals to.
vector   = vector0    + vector1;   // Add elements together.
vector   = vector0    - vector1;   // Subtract elements of vector1 from elements of vector0.
float    = vector0    * vector1;   // A dot product of the vectors.
vector   = vector0    % vector1;   // A cross product of the vectors.
vector   = vector     * float;     // Scale the vector, works the other way around I think.  Works for integer to, but it will end up being cast to float.
vector   = vector     / float;     // Scale the vector, works the other way around I think.  Works for integer to, but it will end up being cast to float.
vector   = vector     * rotation;  // Rotate the vector by the rotation.  Other way around wont compile.
vector   = vector     / rotation;  // Rotate the vector by the rotation, in the opposite direction.  Other way around wont compile.
rotation = llGetRot() * rotation;  // Rotate an object around the global axis.
rotation = rotation   * llGetLocalRot();  // Rotate an object around the local axis.
rotation = rotation0  * rotation1; // Add two rotations, so the result is as if you applied each rotation one after the other.
				   // Division rotates in the opposite direction.
rotation = rotation0  + rotation1; // Similar to vector, but it's a meaningless thing as far as rotations go.
rotation = rotation0  - rotation1; // Similar to vector, but it's a meaningless thing as far as rotations go.

A boolean     operator results in a boolean value.	(any types)
A comparison  operator results in a boolean value.	(any types)
A bitwise     operator results in an integer value.	(intInt or int)
A dot product operator results in a float value.  	(vector * vector)
A vectorFloat          results in a vector value.

*/

static LSL_Leaf *evaluateOperationToken(LSL_Leaf *content, LSL_Leaf *left, LSL_Leaf *right)
{
    LSL_Leaf *result = calloc(1, sizeof(LSL_Leaf));

    if (content && result)
    {
#ifdef LUASL_DEBUG
	printf(" [%s] ", content->token->token);
#endif

	memcpy(result, content, sizeof(LSL_Leaf));

	// Figure out the type of the operation.
	if (OT_vector < result->basicType)
	    result->basicType = allowed[result->basicType].result;

	switch (result->basicType)
	{
	    case OT_float   :
	    {
		float fleft  = left->value.floatValue;
		float fright = right->value.floatValue;

		// Do the casting.
		if (OT_floatInt == content->basicType)
		    fright = right->value.integerValue;
		if (OT_intFloat == content->basicType)
		    fleft = left->value.integerValue;
		switch (result->token->type)
		{
		    case LSL_COMMA			:
		    case LSL_INCREMENT_PRE		:
		    case LSL_INCREMENT_POST		:
		    case LSL_DECREMENT_PRE		:
		    case LSL_DECREMENT_POST		:
		    case LSL_ASSIGNMENT_PLAIN		:
		    case LSL_ASSIGNMENT_DIVIDE		:
		    case LSL_ASSIGNMENT_MULTIPLY	:
		    case LSL_ASSIGNMENT_SUBTRACT	:
		    case LSL_ASSIGNMENT_ADD		:
		    case LSL_BRACKET_OPEN		:
		    case LSL_BRACKET_CLOSE		:
		    case LSL_ANGLE_OPEN			:
		    case LSL_ANGLE_CLOSE		:
		    case LSL_TYPECAST_OPEN		:
		    case LSL_TYPECAST_CLOSE		:
		    case LSL_DOT_PRODUCT		:
			break;
		    case LSL_NEGATION		:  result->value.floatValue = 0 - fright;	break;
		    case LSL_DIVIDE		:  result->value.floatValue = fleft /  fright;	break;
		    case LSL_MULTIPLY		:  result->value.floatValue = fleft *  fright;	break;
		    case LSL_SUBTRACT		:  result->value.floatValue = fleft -  fright;	break;
		    case LSL_ADD		:  result->value.floatValue = fleft +  fright;	break;
		    case LSL_LESS_THAN		:  result->value.floatValue = fleft <  fright;	break;
		    case LSL_GREATER_THAN	:  result->value.floatValue = fleft >  fright;	break;
		    case LSL_LESS_EQUAL		:  result->value.floatValue = fleft <= fright;	break;
		    case LSL_GREATER_EQUAL	:  result->value.floatValue = fleft >= fright;	break;
		    case LSL_EQUAL		:  result->value.floatValue = fleft == fright;	break;
		    case LSL_NOT_EQUAL		:  result->value.floatValue = fleft != fright;	break;
		}
#ifdef LUASL_DEBUG
		printf(" (=%g) ", result->value.floatValue);
#endif
		break;
	    }

	    case OT_integer :
	    {
		switch (result->token->type)
		{
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
		    case LSL_BRACKET_OPEN		:
		    case LSL_BRACKET_CLOSE		:
		    case LSL_ANGLE_OPEN			:
		    case LSL_ANGLE_CLOSE		:
		    case LSL_TYPECAST_OPEN		:
		    case LSL_TYPECAST_CLOSE		:
			break;
		    case LSL_BIT_NOT		:  result->value.integerValue = ~ right->value.integerValue;				break;
		    case LSL_BOOL_NOT		:  result->value.integerValue = ! right->value.integerValue;				break;
		    case LSL_NEGATION		:  result->value.integerValue = 0 - right->value.integerValue;				break;
		    case LSL_DIVIDE		:  result->value.integerValue = left->value.integerValue /  right->value.integerValue;	break;
		    case LSL_MODULO		:  result->value.integerValue = left->value.integerValue %  right->value.integerValue;	break;
		    case LSL_MULTIPLY		:  result->value.integerValue = left->value.integerValue *  right->value.integerValue;	break;
		    case LSL_SUBTRACT		:  result->value.integerValue = left->value.integerValue -  right->value.integerValue;	break;
		    case LSL_ADD		:  result->value.integerValue = left->value.integerValue +  right->value.integerValue;	break;
		    case LSL_LEFT_SHIFT		:  result->value.integerValue = left->value.integerValue << right->value.integerValue;	break;
		    case LSL_RIGHT_SHIFT	:  result->value.integerValue = left->value.integerValue >> right->value.integerValue;	break;
		    case LSL_LESS_THAN		:  result->value.integerValue = left->value.integerValue <  right->value.integerValue;	break;
		    case LSL_GREATER_THAN	:  result->value.integerValue = left->value.integerValue >  right->value.integerValue;	break;
		    case LSL_LESS_EQUAL		:  result->value.integerValue = left->value.integerValue <= right->value.integerValue;	break;
		    case LSL_GREATER_EQUAL	:  result->value.integerValue = left->value.integerValue >= right->value.integerValue;	break;
		    case LSL_EQUAL		:  result->value.integerValue = left->value.integerValue == right->value.integerValue;	break;
		    case LSL_NOT_EQUAL		:  result->value.integerValue = left->value.integerValue != right->value.integerValue;	break;
		    case LSL_BIT_AND		:  result->value.integerValue = left->value.integerValue &  right->value.integerValue;	break;
		    case LSL_BIT_XOR		:  result->value.integerValue = left->value.integerValue ^  right->value.integerValue;	break;
		    case LSL_BIT_OR		:  result->value.integerValue = left->value.integerValue |  right->value.integerValue;	break;
		    case LSL_BOOL_OR		:  result->value.integerValue = left->value.integerValue || right->value.integerValue;	break;
		    case LSL_BOOL_AND		:  result->value.integerValue = left->value.integerValue && right->value.integerValue;	break;
		}
#ifdef LUASL_DEBUG
		printf(" (=%d) ", result->value.integerValue);
#endif
		break;
	    }

	    default :
		break;
	}
    }
    return result;
}

static LSL_Leaf *eveluateParenthesisToken(LSL_Leaf *content, LSL_Leaf *left, LSL_Leaf *right)
{
    LSL_Leaf *result = NULL;

    if (content)
    {
	if (LSL_PARAMETER_LIST != content->value.parenthesis->type)
	    result = evaluateLeaf(content->value.parenthesis->contents, left, right);
    }
    return result;
}


static LSL_Leaf *evaluateStatementToken(LSL_Leaf *content, LSL_Leaf *left, LSL_Leaf *right)
{
    LSL_Leaf *result = NULL;

    if (content)
    {
	result = evaluateLeaf(content->value.statementValue->expressions, left, right);
	if (result)
	{
	    switch (result->basicType)
	    {
		case OT_float   :  printf("\nResult is the float %g.\n", result->value.floatValue);  break;
		case OT_integer :  printf("\nResult is the integer %d.\n", result->value.integerValue);  break;
		default         :  printf("\nResult of an unknown type [%d] %d!\n", result->basicType, result->value.integerValue);  break;
	    }
	    free(result);
	    result = NULL;
	}
	if (left)
	    left->value.integerValue = 0;
	if (right)
	    right->value.integerValue = 0;
    }
    return result;
}

static void outputLeaf(FILE *file, outputMode mode, LSL_Leaf *leaf)
{
    if (leaf)
    {
	outputLeaf(file, mode, leaf->left);
	if ((!(LSL_NOIGNORE & leaf->token->flags)) && (leaf->ignorableText))
	    fprintf(file, "%s", leaf->ignorableText);
	if (leaf->token->output)
	    leaf->token->output(file, mode, leaf);
	else
	    fprintf(file, "%s", leaf->token->token);
	outputLeaf(file, mode, leaf->right);
    }
}

static void outputFloatToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
	fprintf(file, "%g", content->value.floatValue);
}

static void outputFunctionToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
    {
	LSL_Function *func = content->value.functionValue;

	outputLeaf(file, mode, func->type);
	fprintf(file, "%s", func->name);
	outputLeaf(file, mode, func->params);
	outputLeaf(file, mode, func->block);
    }
}

static void outputIntegerToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
	fprintf(file, "%d", content->value.integerValue);
}

static void outputIdentifierToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
	fprintf(file, "%s", content->value.identifierValue->name);
}

static void outputParameterListToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
	outputLeaf(file, mode, content->value.listValue);
}

static void outputParenthesisToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
    {
	fprintf(file, "%s", content->token->token);
	outputLeaf(file, mode, content->value.parenthesis->contents);
	outputLeaf(file, mode, content->value.parenthesis->right);
    }
}

static void outputStateToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
    {
	LSL_State *state = content->value.stateValue;

	fprintf(file, "%s", state->name);
	outputLeaf(file, mode, state->block);
    }
}

static void outputStatementToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
    {
	outputLeaf(file, mode, content->value.statementValue->expressions);
	if (content->ignorableText)
	    fprintf(file, "%s", content->ignorableText);
	fprintf(file, "%s", content->token->token);
    }
}

static void doneParsing(LuaSL_compiler *compiler)
{
    gameGlobals *game = compiler->game;

    if (compiler->ast)
    {
	FILE *out;
	char buffer[PATH_MAX];
	char outName[PATH_MAX];
	char luaName[PATH_MAX];

	outputLeaf(stdout, OM_LSL, compiler->ast);
	printf("\n");
	evaluateLeaf(compiler->ast, NULL, NULL);
	printf("\n");

	strcpy(outName, compiler->fileName);
	strcat(outName, "2");
	strcpy(luaName, compiler->fileName);
	strcat(luaName, ".lua");
	out = fopen(outName, "w");
	if (out)
	{
//	    int count;
	    outputLeaf(out, OM_LSL, compiler->ast);
	    fclose(out);
	    sprintf(buffer, "diff %s %s", compiler->fileName, outName);
//	    count = system(buffer);
//	    PI("Return value of %s is %d", buffer, count);
//	    if (0 != count)
//	        PE("%s says they are different!", buffer);
	}
	else
	    PC("Unable to open file %s for writing!", outName);
	out = fopen(luaName, "w");
	if (out)
	{
	    outputLeaf(out, OM_LUA, compiler->ast);
	    fclose(out);
	}
	else
	    PC("Unable to open file %s for writing!", luaName);
    }
}

Eina_Bool compilerSetup(gameGlobals *game)
{
    int i;

    // Figure out what numbers lemon gave to our tokens.
    for (i = 0; LSL_Tokens[i].token != NULL; i++)
    {
	if (lowestToken > LSL_Tokens[i].type)
	    lowestToken = LSL_Tokens[i].type;
    }
    tokens = calloc(i + 1, sizeof(LSL_Token *));
    if (tokens)
    {
	// Sort the token table.
	for (i = 0; LSL_Tokens[i].token != NULL; i++)
	{
	    int j = LSL_Tokens[i].type - lowestToken;

	    tokens[j] = &(LSL_Tokens[i]);
	}
	return EINA_TRUE;
    }
    else
	PC("No memory for tokens!");

    return EINA_FALSE;
}

Eina_Bool compileLSL(gameGlobals *game, char *script)
{
    Eina_Bool result = EINA_FALSE;
    LuaSL_compiler compiler;
    void *pParser = ParseAlloc(malloc);
    int yv;

// Parse the  LSL script, validating it and reporting errors.
//   Just pass all constants and function names through to Lua, assume they are globals there.

    memset(&compiler, 0, sizeof(LuaSL_compiler));
    compiler.game = game;

    strncpy(compiler.fileName, script, PATH_MAX - 1);
    compiler.fileName[PATH_MAX - 1] = '\0';
    compiler.file = fopen(compiler.fileName, "r");
    if (NULL == compiler.file)
    {
	PE("Error opening file %s.", compiler.fileName);
	return FALSE;
    }
    PI("Opened %s.", compiler.fileName);
    compiler.ast = NULL;
    compiler.lval = calloc(1, sizeof(LSL_Leaf));
    // Text editors usually start counting at 1, even programmers editors.
    compiler.column = 1;
    compiler.line = 1;

    if (yylex_init_extra(&compiler, &(compiler.scanner)))
	return result;
#ifdef LUASL_DEBUG
    yyset_debug(1, compiler.scanner);
    ParseTrace(stdout, "LSL_lemon ");
#endif
    yyset_in(compiler.file, compiler.scanner);
    // on EOF yylex will return 0
    while((yv = yylex(compiler.lval, compiler.scanner)) != 0)
    {
	Parse(pParser, yv, compiler.lval, &compiler);
	if (LSL_SCRIPT == yv)
	    break;
	compiler.lval = calloc(1, sizeof(LSL_Leaf));
    }

    yylex_destroy(compiler.scanner);
    Parse (pParser, 0, compiler.lval, &compiler);
    ParseFree(pParser, free);
    doneParsing(&compiler);

// Take the result of the parse, and convert it into Lua source.
//   Each LSL script becomes a Lua state.
//   LSL states are handled as Lua tables, with each LSL state function being a table function in a common metatable.
//   LL and OS functions are likely to be C functions. 

// Compile the Lua source by the Lua compiler.

    if (NULL != compiler.file)
    {
	fclose(compiler.file);
	compiler.file = NULL;
    }
    burnLeaf(compiler.ast);

    return result;
}


// Code for running it stand alone, and with files input on the command line.
#if 0
static int nextFile(LuaSL_yyparseParam *param)
{
    if (NULL != param->file)
    {
	fclose(param->file);
	param->file = NULL;
    }
    if (--(param->argc) > 0 && *++(param->argv) != '\0')
    {
	strncpy(param->fileName, *(param->argv), PATH_MAX - 1);
	param->fileName[PATH_MAX - 1] = '\0';
	param->file = fopen(param->fileName, "r");
	if (NULL == param->file)
	{
	    PE(stderr, "Error opening file %s.", param->fileName);
	    return FALSE;
	}
	PE("Opened %s.", param->fileName);
	burnLeaf(param->ast);
	param->ast = NULL;
	param->lval = calloc(1, sizeof(LSL_Leaf));
	// Text editors usually start counting at 1, even programmers editors.
	param->column = 1;
	param->line = 1;
	return TRUE;
    }
/*
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
*/

    return FALSE;
}

char *test[] = {"test2.lsl", "test2.lsl"};

int main(int argc, char **argv)
{
//    char *programName = argv[0];
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
	LuaSL_yyparseParam param;

	// Sort the token table.
	for (i = 0; LSL_Tokens[i].token != NULL; i++)
	{
	    int j = LSL_Tokens[i].type - lowestToken;

	    tokens[j] = &(LSL_Tokens[i]);
	}

	// First time setup.
	if (1 == argc)
	{
	    // Fake a test file if there is none.  Mostly for ddd.
	    argc++;
	    argv = test;
	}
	memset(&param, 0, sizeof(param));
	param.argc = argc;
	param.argv = argv;

	// Loop through the files.
	while (nextFile(&param))
	{
	    void *pParser = ParseAlloc(malloc);
	    int yv;

	    if (yylex_init_extra(&param, &(param.scanner)))
		return 1;
#ifdef LUASL_DEBUG
	    yyset_debug(1, param.scanner);
	    ParseTrace(stdout, "LSL_lemon ");
#endif
	    yyset_in(param.file, param.scanner);
	    // on EOF yylex will return 0
	    while((yv = yylex(param.lval, param.scanner)) != 0)
	    {
		Parse(pParser, yv, param.lval, &param);
		if (LSL_SCRIPT == yv)
		    break;
		param.lval = calloc(1, sizeof(LSL_Leaf));
	    }

	    yylex_destroy(param.scanner);
	    Parse (pParser, 0, param.lval, &param);
	    ParseFree(pParser, free);
	    doneParsing(&param);
	}
    }
    else
    {
	fprintf(stderr, "No memory for tokens!");
	return 1;
    }

    return 0;
}
#endif
