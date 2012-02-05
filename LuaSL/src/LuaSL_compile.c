
#include "LuaSL.h"

/* TODO - problem de jour
*/


static void outputBitOp(FILE *file, outputMode mode, LSL_Leaf *leaf);
static void outputBlockToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputCrementsToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputFloatToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputFunctionToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputFunctionCallToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputIntegerToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputIdentifierToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputListToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputParameterListToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputParenthesisToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputStateToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputStatementToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputStringToken(FILE *file, outputMode mode, LSL_Leaf *content);

LSL_Token LSL_Tokens[] =
{
    // Various forms of "space".
    {LSL_COMMENT,		ST_NONE,		"/*",	LSL_NONE,				NULL},
    {LSL_COMMENT_LINE,		ST_NONE,		"//",	LSL_NONE,				NULL},
    {LSL_SPACE,			ST_NONE,		" ",	LSL_NONE,				NULL},

    // Operators, in order of precedence, low to high
    // Left to right, unless otherwise stated.
    // According to http://wiki.secondlife.com/wiki/Category:LSL_Operators, which was obsoleted by http://wiki.secondlife.com/wiki/LSL_Operators but that has less info.

    {LSL_ASSIGNMENT_CONCATENATE,ST_CONCATENATION,	"+=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL},
    {LSL_ASSIGNMENT_ADD,	ST_CONCATENATION,	"+=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL},
    {LSL_ASSIGNMENT_SUBTRACT,	ST_ASSIGNMENT,		"-=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL},
    {LSL_ASSIGNMENT_MULTIPLY,	ST_ASSIGNMENT,		"*=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL},
    {LSL_ASSIGNMENT_MODULO,	ST_MODULO,		"%=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL},
    {LSL_ASSIGNMENT_DIVIDE,	ST_ASSIGNMENT,		"/=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL},
    {LSL_ASSIGNMENT_PLAIN,	ST_CONCATENATION,	"=",	LSL_RIGHT2LEFT | LSL_ASSIGNMENT,	NULL},

    {LSL_BOOL_AND,		ST_BOOLEAN,		"&&",	LSL_RIGHT2LEFT,				NULL},
// QUIRK - Seems to be some disagreement about BOOL_AND/BOOL_OR precedence.  Either they are equal, or OR is higher.
// QUIRK - No boolean short circuiting.
// LUA   - Short circiuts boolean operations, and goes left to right.
// LUA   - "and" returns its first argument if it is false, otherwise, it returns its second argument. "or" returns its first argument if it is not false, otherwise it returns its second argument.
//           Note that the above means that "and/or" can return any type.
    {LSL_BOOL_OR,		ST_BOOLEAN,		"||",	LSL_RIGHT2LEFT,				NULL},
    {LSL_BIT_OR,		ST_BITWISE,		"|",	LSL_LEFT2RIGHT,				outputBitOp},
    {LSL_BIT_XOR,		ST_BITWISE,		"^",	LSL_LEFT2RIGHT,				outputBitOp},
    {LSL_BIT_AND,		ST_BITWISE,		"&",	LSL_LEFT2RIGHT,				outputBitOp},
// QUIRK - Booleans and conditionals are executed right to left.  Or maybe not, depending on who you believe.
    {LSL_NOT_EQUAL,		ST_EQUALITY,		"!=",	LSL_RIGHT2LEFT,				NULL},
    {LSL_EQUAL,			ST_EQUALITY,		"==",	LSL_RIGHT2LEFT,				NULL},
    {LSL_GREATER_EQUAL,		ST_COMPARISON,		">=",	LSL_RIGHT2LEFT,				NULL},
    {LSL_LESS_EQUAL,		ST_COMPARISON,		"<=",	LSL_RIGHT2LEFT,				NULL},
    {LSL_GREATER_THAN,		ST_COMPARISON,		">",	LSL_RIGHT2LEFT,				NULL},
    {LSL_LESS_THAN,		ST_COMPARISON,		"<",	LSL_RIGHT2LEFT,				NULL},
// LUA   - comparisons are always false if they are different types.  Tables, userdata, and functions are compared by reference.  Strings compare in alphabetical order, depending on current locale.
// LUA   - really only has three conditionals, as it translates a ~= b to not (a == b), a > b to b < a, and a >= b to b <= a.
    {LSL_RIGHT_SHIFT,		ST_BITWISE,		">>",	LSL_LEFT2RIGHT,				outputBitOp},
    {LSL_LEFT_SHIFT,		ST_BITWISE,		"<<",	LSL_LEFT2RIGHT,				outputBitOp},
    {LSL_CONCATENATE,		ST_ADD,			"+",	LSL_LEFT2RIGHT,				NULL},
    {LSL_ADD,			ST_ADD,			"+",	LSL_LEFT2RIGHT,				NULL},
    {LSL_SUBTRACT,		ST_SUBTRACT,		"-",	LSL_LEFT2RIGHT,				NULL},
    {LSL_CROSS_PRODUCT,		ST_NONE,		"%",	LSL_LEFT2RIGHT,				NULL},
    {LSL_DOT_PRODUCT,		ST_NONE,		"*",	LSL_LEFT2RIGHT,				NULL},
    {LSL_MULTIPLY,		ST_MULTIPLY,		"*",	LSL_LEFT2RIGHT,				NULL},
    {LSL_MODULO,		ST_MODULO,		"%",	LSL_LEFT2RIGHT,				NULL},
    {LSL_DIVIDE,		ST_MULTIPLY,		"/",	LSL_LEFT2RIGHT,				NULL},
    {LSL_NEGATION,		ST_NEGATE,		"-",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL},
    {LSL_BOOL_NOT,		ST_BOOL_NOT,		"!",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL},
    {LSL_BIT_NOT,		ST_BIT_NOT,		"~",	LSL_RIGHT2LEFT | LSL_UNARY,		outputBitOp},

// LUA precedence - (it has no bit operators, at least not until 5.2, but LuaJIT has them as table functions.)
// or
// and
// < > <= >= ~= ==
// ..
// + -
// * /
// not negate
// exponentiation (^)

    {LSL_TYPECAST_CLOSE,	ST_NONE,		")",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL},
    {LSL_TYPECAST_OPEN,		ST_NONE,		"(",	LSL_RIGHT2LEFT | LSL_UNARY,		outputParenthesisToken},
    {LSL_ANGLE_CLOSE,		ST_NONE,		">",	LSL_LEFT2RIGHT | LSL_CREATION,		NULL},
    {LSL_ANGLE_OPEN,		ST_NONE,		"<",	LSL_LEFT2RIGHT | LSL_CREATION,		NULL},
    {LSL_BRACKET_CLOSE,		ST_NONE,		"]",	LSL_INNER2OUTER | LSL_CREATION,		NULL},
    {LSL_BRACKET_OPEN,		ST_NONE,		"[",	LSL_INNER2OUTER | LSL_CREATION,		NULL},
    {LSL_PARENTHESIS_CLOSE,	ST_NONE,		")",	LSL_INNER2OUTER,			NULL},
    {LSL_PARENTHESIS_OPEN,	ST_NONE,		"(",	LSL_INNER2OUTER,			outputParenthesisToken},
    {LSL_DOT,			ST_NONE,		".",	LSL_RIGHT2LEFT,				NULL},
    {LSL_DECREMENT_POST,	ST_NONE,		"--",	LSL_RIGHT2LEFT | LSL_UNARY,		outputCrementsToken},
    {LSL_DECREMENT_PRE,		ST_NONE,		"--",	LSL_RIGHT2LEFT | LSL_UNARY,		outputCrementsToken},
    {LSL_INCREMENT_POST,	ST_NONE,		"++",	LSL_RIGHT2LEFT | LSL_UNARY,		outputCrementsToken},
    {LSL_INCREMENT_PRE,		ST_NONE,		"++",	LSL_RIGHT2LEFT | LSL_UNARY,		outputCrementsToken},
    {LSL_COMMA,			ST_NONE,		",",	LSL_LEFT2RIGHT,				NULL},

    {LSL_EXPRESSION,		ST_NONE,	"expression",	LSL_NONE	,			NULL},

    // Types.
    {LSL_FLOAT,			ST_NONE,	"float",	LSL_NONE,				outputFloatToken},
    {LSL_INTEGER,		ST_NONE,	"integer",	LSL_NONE,				outputIntegerToken},
    {LSL_KEY,			ST_NONE,	"key",		LSL_NONE,				outputStringToken},
    {LSL_LIST,			ST_NONE,	"list",		LSL_NONE,				outputListToken},
    {LSL_ROTATION,		ST_NONE,	"rotation",	LSL_NONE,				outputListToken},
    {LSL_STRING,		ST_NONE,	"string",	LSL_NONE,				outputStringToken},
    {LSL_VECTOR,		ST_NONE,	"vector",	LSL_NONE,				outputListToken},

    // Types names.
    {LSL_TYPE_FLOAT,		ST_NONE,	"float",	LSL_TYPE,				NULL},
    {LSL_TYPE_INTEGER,		ST_NONE,	"integer",	LSL_TYPE,				NULL},
    {LSL_TYPE_KEY,		ST_NONE,	"key",		LSL_TYPE,				NULL},
    {LSL_TYPE_LIST,		ST_NONE,	"list",		LSL_TYPE,				NULL},
    {LSL_TYPE_ROTATION,		ST_NONE,	"rotation",	LSL_TYPE,				NULL},
    {LSL_TYPE_STRING,		ST_NONE,	"string",	LSL_TYPE,				NULL},
    {LSL_TYPE_VECTOR,		ST_NONE,	"vector",	LSL_TYPE,				NULL},

    // Then the rest of the syntax tokens.
    {LSL_FUNCTION_CALL,		ST_NONE,	"funccall",	LSL_NONE,				outputFunctionCallToken},
    {LSL_IDENTIFIER,		ST_NONE,	"identifier",	LSL_NONE,				outputIdentifierToken},
    {LSL_VARIABLE,		ST_NONE,	"variable",	LSL_NONE,				outputIdentifierToken},

    {LSL_LABEL,			ST_NONE,	"@",		LSL_NONE,				NULL},

    {LSL_DO,			ST_NONE,	"do",		LSL_NONE,				NULL},
    {LSL_FOR,			ST_NONE,	"for",		LSL_NONE,				NULL},
    {LSL_ELSE,			ST_NONE,	"else",		LSL_NONE,				NULL},
    {LSL_ELSEIF,		ST_NONE,	"elseif",	LSL_NONE,				NULL},
    {LSL_IF,			ST_NONE,	"if",		LSL_NONE,				NULL},
    {LSL_JUMP,			ST_NONE,	"jump",		LSL_NONE,				NULL},
    {LSL_RETURN,		ST_NONE,	"return",	LSL_NONE,				NULL},
    {LSL_STATE_CHANGE,		ST_NONE,	"state",	LSL_NONE,				NULL},
    {LSL_WHILE,			ST_NONE,	"while",	LSL_NONE,				NULL},
    {LSL_STATEMENT,		ST_NONE,	";",		LSL_NOIGNORE,				outputStatementToken},

    {LSL_BLOCK_CLOSE,		ST_NONE,	"}",		LSL_NONE,				NULL},
    {LSL_BLOCK_OPEN,		ST_NONE,	"{",		LSL_NONE,				outputBlockToken},
    {LSL_PARAMETER,		ST_NONE,	"parameter",	LSL_NONE,				outputIdentifierToken},
    {LSL_PARAMETER_LIST,	ST_NONE,	"plist",	LSL_NONE,				outputParameterListToken},
    {LSL_FUNCTION,		ST_NONE,	"function",	LSL_NONE,				outputFunctionToken},
    {LSL_DEFAULT,		ST_NONE,	"default",	LSL_NONE,				outputStateToken},
    {LSL_STATE,			ST_NONE,	"state",	LSL_NONE,				outputStateToken},
    {LSL_SCRIPT,		ST_NONE,	"",		LSL_NONE,				NULL},

    {LSL_UNKNOWN,		ST_NONE,	"unknown",	LSL_NONE,				NULL},

    // A sentinal.
    {999999, ST_NONE, NULL, LSL_NONE, NULL}
};

// VERY IMPORTANT to keep this in sync with enum opType from LuaSL_LSL_tree.h!
allowedTypes allowed[] =
{
    {OT_nothing,	"nothing",	(ST_NONE)},																//

    {OT_bool,		"boolean",	(ST_BOOL_NOT)},																// bool				!
    {OT_integer,	"integer",	(ST_BOOL_NOT | ST_BIT_NOT | ST_NEGATE)},												// int				! - ~
    {OT_float,		"float",	(ST_BOOL_NOT | ST_NEGATE)},														// float			! -
    {OT_key,		"key",		(ST_BOOL_NOT)},																// key				!
    {OT_list,		"list",		(ST_NONE)},																//
    {OT_rotation,	"rotation",	(ST_NONE)},																//
    {OT_string,		"string",	(ST_BOOL_NOT)},																// string			!
    {OT_vector,		"vector",	(ST_NONE)},																//
    {OT_other,		"other",	(ST_NONE)},																//

    {OT_bool,		"boolean",	(ST_BOOLEAN | ST_EQUALITY)},														// bool		bool		          == !=           =                            && ||

    {OT_integer,	"integer",	(ST_MULTIPLY | ST_ADD | ST_SUBTRACT | ST_EQUALITY | ST_COMPARISON | ST_CONCATENATION | ST_ASSIGNMENT | ST_MODULO | ST_BITWISE)},	// int		boolean		* / + - % == != < > <= >= = += -= *= /= %= & | ^ << >>
    {OT_integer,	"integer",	(ST_MULTIPLY | ST_ADD | ST_SUBTRACT | ST_EQUALITY | ST_COMPARISON | ST_CONCATENATION | ST_ASSIGNMENT | ST_MODULO | ST_BITWISE)},	// int		int		* / + - % == != < > <= >= = += -= *= /= %= & | ^ << >>
    {OT_float,		"float",	(ST_MULTIPLY | ST_ADD | ST_SUBTRACT | ST_EQUALITY | ST_COMPARISON | ST_CONCATENATION | ST_ASSIGNMENT)},					// int		float		cast to float float
    {OT_float,		"float",	(ST_MULTIPLY | ST_ADD | ST_SUBTRACT | ST_EQUALITY | ST_COMPARISON | ST_CONCATENATION | ST_ASSIGNMENT)},					// float	int		cast to float float
    {OT_float,		"float",	(ST_MULTIPLY | ST_ADD | ST_SUBTRACT | ST_EQUALITY | ST_COMPARISON | ST_CONCATENATION | ST_ASSIGNMENT)},					// float	float		* / + -   == != < > <= >= = += -= *= /=

    {OT_string,		"string",	(ST_ADD | ST_EQUALITY | ST_CONCATENATION)},												// key		key		cast to string string
    {OT_string,		"string",	(ST_ADD | ST_EQUALITY | ST_CONCATENATION)},												// key		string		cast to string string
    {OT_string,		"string",	(ST_ADD | ST_EQUALITY | ST_CONCATENATION)},												// string	key		cast to string string
    {OT_string,		"string",	(ST_ADD | ST_EQUALITY | ST_CONCATENATION)},												// string	string		    +     == !=           = +=

    {OT_list,		"list",		(ST_ADD | ST_EQUALITY   | ST_CONCATENATION | ST_ASSIGNMENT )},										// list		list		    +     == !=           = +=
    {OT_list,		"list",		(ST_ADD | ST_COMPARISON | ST_CONCATENATION | ST_ASSIGNMENT )},										// list		boolean		    +           < > <= >= = +=
    {OT_list,		"list",		(ST_ADD | ST_COMPARISON | ST_CONCATENATION | ST_ASSIGNMENT )},										// list		integer		    +           < > <= >= = +=
    {OT_list,		"list",		(ST_ADD | ST_COMPARISON | ST_CONCATENATION | ST_ASSIGNMENT )},										// list		float		    +           < > <= >= = +=
    {OT_list,		"list",		(ST_ADD | ST_COMPARISON | ST_CONCATENATION | ST_ASSIGNMENT )},										// list		string		    +           < > <= >= = +=
    {OT_integer,	"integer",	(ST_ADD | ST_COMPARISON)},														// integer	list		    +           < > <= >=
    {OT_float,		"float",	(ST_ADD | ST_COMPARISON)},														// float	list		    +           < > <= >=
    {OT_list,		"list",		(ST_ADD | ST_CONCATENATION)},														// list		other		    +                     = +=

    {OT_vector,		"vector",	(ST_MULTIPLY | ST_ADD | ST_SUBTRACT | ST_EQUALITY | ST_CONCATENATION | ST_ASSIGNMENT | ST_MODULO)},					// vector	vector		* / + - % == !=           = += -= *= /= %=
    {OT_vector,		"vector",	(ST_MULTIPLY | ST_ASSIGNMENT)},														// vector	float		* /                               *= /=
    {OT_vector,		"vector",	(ST_MULTIPLY)},																// vector	rotation	* /

    {OT_rotation,	"rotation",	(ST_MULTIPLY | ST_ADD | ST_SUBTRACT | ST_EQUALITY | ST_CONCATENATION | ST_ASSIGNMENT)},							// rotation	rotation	* / + -   == !=           = += -= *= /=

    {OT_other,		"other",	(ST_NONE)},																//
    {OT_undeclared,	"undeclared",	(ST_NONE)},																//
    {OT_invalid,	"invalid",	(ST_NONE)}																//
};

opType opExpr[][10] =
{
    {OT_nothing,  OT_bool,     OT_integer,  OT_float,       OT_key,       OT_list,      OT_rotation,         OT_string,       OT_vector,       OT_other},
    {OT_bool,     OT_boolBool, OT_invalid,  OT_invalid,     OT_invalid,   OT_invalid,   OT_invalid,          OT_invalid,      OT_invalid,      OT_invalid},
    {OT_integer,  OT_intBool,  OT_intInt,   OT_intFloat,    OT_invalid,   OT_intList,   OT_invalid,          OT_invalid,      OT_invalid,      OT_invalid},
    {OT_float,    OT_invalid,  OT_floatInt, OT_floatFloat,  OT_invalid,   OT_floatList, OT_invalid,          OT_invalid,      OT_invalid,      OT_invalid},
    {OT_key,      OT_invalid,  OT_invalid,  OT_invalid,     OT_keyKey,    OT_invalid,   OT_invalid,          OT_keyString,    OT_invalid,      OT_invalid},
    {OT_list,     OT_listBool, OT_listInt,  OT_listFloat,   OT_invalid,   OT_listList,  OT_invalid,          OT_listString,   OT_invalid,      OT_listOther},
    {OT_rotation, OT_invalid,  OT_invalid,  OT_invalid,     OT_invalid,   OT_invalid,   OT_rotationRotation, OT_invalid,      OT_invalid,      OT_invalid},
    {OT_string,   OT_invalid,  OT_invalid,  OT_invalid,     OT_stringKey, OT_invalid,   OT_invalid,          OT_stringString, OT_invalid,      OT_invalid},
    {OT_vector,   OT_invalid,  OT_invalid,  OT_vectorFloat, OT_invalid,   OT_invalid,   OT_vectorRotation,   OT_invalid,      OT_vectorVector, OT_invalid},
    {OT_other,    OT_invalid,  OT_invalid,  OT_invalid,     OT_invalid,   OT_invalid,   OT_invalid,          OT_invalid,      OT_invalid,      OT_otherOther}
};


LSL_Token **tokens = NULL;
LSL_Script constants;
int lowestToken = 999999;


static LSL_Leaf *newLeaf(LSL_Type type, LSL_Leaf *left, LSL_Leaf *right)
{
    LSL_Leaf *leaf = calloc(1, sizeof(LSL_Leaf));

    if (leaf)
    {
	leaf->left = left;
	leaf->right = right;
	leaf->toKen = tokens[type - lowestToken];
    }

    return leaf;
}

void burnLeaf(void *data)
{
    LSL_Leaf *leaf = data;

    if (leaf)
    {
// TODO - the problem here is that lemon wants to free these after a reduce, but we might want to keep them around.  Should ref count them or something.
//	burnLeaf(leaf->left);
//	burnLeaf(leaf->right);
	// TODO - Should free up the value to.
//#if LUASL_DIFF_CHECK
//	   eina_strbuf_free(leaf->ignorable);
//#endif
//	free(leaf);
    }
}

static LSL_Leaf *findFunction(LuaSL_compiler *compiler, const char *name)
{
    LSL_Leaf  *func = NULL;

    if (name)
    {
	if (NULL == func)
	    func = eina_hash_find(constants.functions, name);
	if (NULL != func)
	{
	    func->flags |= MF_LSLCONST;
	    func->value.functionValue->flags |= MF_LSLCONST;
	}
	else
	    func = eina_hash_find(compiler->script.functions, name);

    }

    return func;
}

static LSL_Leaf *findVariable(LuaSL_compiler *compiler, const char *name)
{
    LSL_Leaf  *var = NULL;

    if (name)
    {
	LSL_Block *block = compiler->currentBlock;

	while ((block) && (NULL == var))
	{
	    if (block->function)
	    {
		LSL_Leaf *param = NULL;
		EINA_INARRAY_FOREACH((&(block->function->vars)), param)
		{
		    if ((param) && (LSL_PARAMETER == param->toKen->type))
		    {
//			if (name == param->value.identifierValue->name.text)		// Assuming they are stringshares.
			if (0 == strcmp(name, param->value.identifierValue->name.text))	// Not assuming they are stringeshares.  They should be.
			    var = param;
		    }
		}
	    }
	    if ((NULL == var) && block->variables)
		var = eina_hash_find(block->variables, name);
	    block = block->outerBlock;
	}

	if (NULL == var)
	{
	    var = eina_hash_find(constants.variables, name);
	    if (var)
	    {
		var->flags |= MF_LSLCONST;
		var->value.identifierValue->flags |= MF_LSLCONST;
	    }
	}
	if (NULL == var)
	    var = eina_hash_find(compiler->script.variables, name);
    }

    return var;
}

LSL_Leaf *checkVariable(LuaSL_compiler *compiler, LSL_Leaf *identifier, LSL_Leaf *dot, LSL_Leaf *sub)
{
    gameGlobals *game = compiler->game;
    const char *search;

    if (dot)
	search = identifier->value.identifierValue->name.text;
    else
	search = identifier->value.stringValue;

    if (identifier)
    {
	LSL_Leaf *var = findVariable(compiler, search);

	if (var)
	{
	    if (LUASL_DEBUG)
		PI("Found %s!", identifier->value.stringValue);
	    identifier->value.identifierValue = var->value.identifierValue;
	    identifier->basicType = var->basicType;
	    if ((dot) && (sub))
	    {
		LSL_Identifier *id = calloc(1, sizeof(LSL_Identifier));

		if (id)
		{
		    memcpy(id, var->value.identifierValue, sizeof(LSL_Identifier));
		    identifier->value.identifierValue = id;
		    if (LSL_ROTATION == var->toKen->type)
		    {
			// TODO - check if it's one of x, y, z, or s.
		    }
		    if (LSL_VECTOR == var->toKen->type)
		    {
			// TODO - check if it's one of x, y, or z.
		    }
		    identifier->value.identifierValue->sub = sub->value.stringValue;
		    identifier->basicType = OT_float;
		}
	    }
	}
	else
	{
	    compiler->script.bugCount++;
	    PE("NOT found %s @ line %d, column %d!", identifier->value.stringValue, identifier->line, identifier->column);
	}
    }

    return identifier;
}

LSL_Leaf *addOperation(LuaSL_compiler *compiler, LSL_Leaf *left, LSL_Leaf *lval, LSL_Leaf *right)
{
    gameGlobals *game = compiler->game;

    if (lval)
    {
	opType lType, rType;

	lval->left = left;
	lval->right = right;

	// Convert subtract to negate if needed.
	if ((NULL == left) && (LSL_SUBTRACT == lval->toKen->type))
	    lval->toKen = tokens[LSL_NEGATION - lowestToken];

	// Try to figure out what type of operation this is.
	if (NULL == left)
	    lType = OT_nothing;
	else
	{
	    if ((left->toKen) && (LSL_IDENTIFIER == left->toKen->type) && (left->value.identifierValue))
	    {
		LSL_Leaf *var = findVariable(compiler, left->value.identifierValue->name.text);

		if (var)
		    lType = var->basicType;
		if (left->value.identifierValue->sub)
		{
		    // TODO - keep an eye on this, but I think all the sub types are floats.
		    lType = OT_float;
		}
	    }
	    else
		lType = left->basicType;
	    if (OT_undeclared == lType)
	    {
		compiler->script.warningCount++;
		PW("Undeclared identifier issue, deferring this until the second pass. @ line %d, column %d.", lval->line, lval->column);
		lval->basicType = OT_undeclared;
		return lval;
	    }
	    if (OT_vector < lType)
		lType = allowed[lType].result;
	}
	if (NULL == right)
	    rType = OT_nothing;
	else
	{
	    if ((right->toKen) && (LSL_IDENTIFIER == right->toKen->type) && (right->value.identifierValue))
	    {
		LSL_Leaf *var = findVariable(compiler, right->value.identifierValue->name.text);

		if (var)
		    rType = var->basicType;
		if (right->value.identifierValue->sub)
		{
		    // TODO - keep an eye on this, but I think all the sub types are floats.
		    rType = OT_float;
		}
	    }
	    else
		rType = right->basicType;
	    if (OT_undeclared == rType)
	    {
		compiler->script.warningCount++;
		PW("Undeclared identifier issue, deferring this until the second pass. @ line %d, column %d.", lval->line, lval->column);
		lval->basicType = OT_undeclared;
		return lval;
	    }
	    if (OT_vector < rType)
		rType = allowed[rType].result;
	}

	// Convert add to concatenate if needed.
	if ((LSL_ADD == lval->toKen->type) && (OT_string == lType) && (OT_string == rType))
	    lval->toKen = tokens[LSL_CONCATENATE - lowestToken];

	switch (lval->toKen->subType)
	{
	    case ST_BOOLEAN :
	    case ST_COMPARISON :
	    case ST_EQUALITY :
		lval->basicType = OT_bool;
		break;
	    default :
		// The basic lookup.
		lval->basicType = opExpr[lType][rType];
		if (OT_invalid != lval->basicType)
		{
		    // Check if it's an allowed operation.
		    if (0 == (lval->toKen->subType & allowed[lval->basicType].subTypes))
			lval->basicType = OT_invalid;
		    else
		    {
			// Double check the corner cases.
			switch (lval->toKen->subType)
			{
			    case ST_MULTIPLY :
				if (OT_vectorVector == lval->basicType)
				{
				    if (LSL_MULTIPLY == lval->toKen->type)
				    {
					lval->basicType = OT_float;
				    	lval->toKen = tokens[LSL_DOT_PRODUCT - lowestToken];
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
		break;
	}

	/* Flag assignments for the "assignments are statements, which can't happen inside expressions" test.
	 *
	 * Assignments in the middle of expressions are legal in LSL, but not in Lua.
	 * The big complication is that they often happen in the conditionals of flow control statements.  That's a big bitch.
	 *
	 * So things like -
	 *
	 *    while ((x = doSomething()) == foo)
	 *    {
	 *	buggerAround();
	 *    }
	 *
	 * Turns into -
	 *
	 *    x = doSomething();
	 *    while (x == foo)
	 *    {
	 *	buggerAround();
	 *	x = doSomething();
	 *    }
	 *
	 * http://lua-users.org/wiki/StatementsInExpressions was helpful.  Which suggests something like this -
	 *
	 *    while ( (function() x = doSomething(); return x; end)() == foo)
	 *    {
	 *	buggerAround();
	 *    }
	 *
	 * The remaining problem is when to recognise the need to do that.
	 * That's what this code and the matching code in addParenthesis() does.
	 */
	// TODO - Only got one of these in my test scripts, so leave all this debugging shit in until it's been tested more.
	if (left)
	{
	    if (left->flags & MF_ASSIGNEXP)
	    {
//if ((left) && (right))
//    printf("%s %s %s\n", left->toKen->toKen, lval->toKen->toKen, right->toKen->toKen);
//else if (left)
//    printf("%s %s NORIGHT\n", left->toKen->toKen, lval->toKen->toKen);
//else if (right)
//    printf("NOLEFT %s %s\n", lval->toKen->toKen, right->toKen->toKen);
//else
//    printf("NOLEFT %s NORIGHT\n", lval->toKen->toKen);
//		printf("############################################################################## left\n");
		left->flags |= MF_WRAPFUNC;
		if (LSL_PARENTHESIS_OPEN == left->toKen->type)
		    left->value.parenthesis->flags |= MF_WRAPFUNC;
	    }
	}
	if (lval)
	{
//	    if (lval->flags & MF_ASSIGNEXP)
//		printf("############################################################################## lval %s %s\n", left->toKen->toKen, right->toKen->toKen);
	    if (LSL_ASSIGNMENT & lval->toKen->flags)
	    {
		lval->flags |= MF_ASSIGNEXP;
////		printf("******************* lval %s %s\n", left->toKen->toKen, right->toKen->toKen);
		if (LSL_IDENTIFIER == left->toKen->type)  // It always should be.
		{
		    left->flags |= MF_ASSIGNEXP;
////		    printf("$$$$$$$$$$$$$$$$$ lval\n");
		}
	    }
	}
	// TODO - Don't think I have to do this on the right.
	if (right)
	{
	    if (right->flags & MF_ASSIGNEXP)
	    {
if ((left) && (right))
    printf("%s %s %s\n", left->toKen->toKen, lval->toKen->toKen, right->toKen->toKen);
else if (left)
    printf("%s %s NORIGHT\n", left->toKen->toKen, lval->toKen->toKen);
else if (right)
    printf("NOLEFT %s %s\n", lval->toKen->toKen, right->toKen->toKen);
else
    printf("NOLEFT %s NORIGHT\n", lval->toKen->toKen);
		printf("############################################################################## right\n");
		right->flags |= MF_WRAPFUNC;
	    }
	}

	if (OT_invalid == lval->basicType)
	{
	    const char *leftType = "", *rightType = "", *leftToken = "", *rightToken = "";

	    if (left)
	    {
		if (left->toKen)
		    leftToken = left->toKen->toKen;
		else
		    PE("BROKEN LEFT TOKEN!!!!!!!!!!!!!!!!!!");
		leftType = allowed[left->basicType].name;
	    }
	    if (right)
	    {
		if (right->toKen)
		    rightToken = right->toKen->toKen;
		else
		    PE("BROKEN RIGHT TOKEN!!!!!!!!!!!!!!!!!!");
		rightType = allowed[right->basicType].name;
	    }

	    compiler->script.bugCount++;
	    PE("Invalid operation [%s(%s) %s %s(%s)] @ line %d, column %d!", leftType, leftToken, lval->toKen->toKen, rightType, rightToken, lval->line, lval->column);
	}
    }

    return lval;
}

LSL_Leaf *addBlock(LuaSL_compiler *compiler, LSL_Leaf *left, LSL_Leaf *lval, LSL_Leaf *right)
{
    // Damn, look ahead.  The } symbol is getting read (and thus endBlock called) before the last statement in the block is reduced (which actually calls the add*() functions).
    compiler->currentBlock = compiler->currentBlock->outerBlock;
#if LUASL_DIFF_CHECK
    if ((left) && (right))
    {
	left->value.blockValue->closeIgnorable = right->ignorable;
	right->ignorable = NULL;
    }
#endif
    return lval;
}

LSL_Leaf *addCrement(LuaSL_compiler *compiler, LSL_Leaf *variable, LSL_Leaf *crement, LSL_Type type)
{
    if ((variable) && (crement))
    {
	crement->value.identifierValue = variable->value.identifierValue;
#if LUASL_DIFF_CHECK
	crement->value.identifierValue->ignorable = variable->ignorable;
	variable->ignorable = NULL;
#endif
	crement->basicType = variable->basicType;
	crement->toKen = tokens[type - lowestToken];
	switch (crement->toKen->type)
	{
	    case LSL_DECREMENT_PRE  :  variable->value.identifierValue->flags |= MF_PREDEC;  break;
	    case LSL_INCREMENT_PRE  :  variable->value.identifierValue->flags |= MF_PREINC;  break;
	    case LSL_DECREMENT_POST :  variable->value.identifierValue->flags |= MF_POSTDEC;  break;
	    case LSL_INCREMENT_POST :  variable->value.identifierValue->flags |= MF_POSTINC;  break;
	}
	variable->value.identifierValue->definition->flags = variable->value.identifierValue->flags;
    }

    return crement;
}

LSL_Leaf *addParameter(LuaSL_compiler *compiler, LSL_Leaf *type, LSL_Leaf *identifier)
{
    LSL_Identifier *result = calloc(1, sizeof(LSL_Identifier));

    if ( (identifier) && (result))
    {
	result->name.text = identifier->value.stringValue;
#if LUASL_DIFF_CHECK
	result->name.ignorable = identifier->ignorable;
	identifier->ignorable = NULL;
#endif
	result->value.toKen = tokens[LSL_UNKNOWN - lowestToken];
	identifier->value.identifierValue = result;
	identifier->toKen = tokens[LSL_PARAMETER - lowestToken];
	identifier->left = type;
	if (type)
	{
	    identifier->basicType = type->basicType;
	    result->value.basicType = type->basicType;
	    result->value.toKen = type->toKen;	// This is the LSL_TYPE_* toKen instead of the LSL_* toKen.  Not sure if that's a problem.
	}
    }
    return identifier;
}

LSL_Leaf *collectParameters(LuaSL_compiler *compiler, LSL_Leaf *list, LSL_Leaf *comma, LSL_Leaf *newParam)
{
    LSL_Function *func = NULL;

    if (NULL == list)
	list = newLeaf(LSL_FUNCTION, NULL, NULL);

    if (list)
    {
	func = list->value.functionValue;
	if (NULL == func)
	{
	    func = calloc(1, sizeof(LSL_Function));
	    if (func)
	    {
		list->value.functionValue = func;
		eina_inarray_setup(&(func->vars), sizeof(LSL_Leaf), 3);
	    }
	}

	if (func)
	{
	    if (newParam)
	    {
		if (LUASL_DIFF_CHECK)
		{
		    // Stash the comma for diff later.
		    if (comma)
			eina_inarray_append(&(func->vars), comma);
		}
		eina_inarray_append(&(func->vars), newParam);
		// At this point, pointers to newParams are not pointing to the one in func->vars, AND newParam is no longer needed.
	    }
	}
    }
    return list;
}

LSL_Leaf *addFunction(LuaSL_compiler *compiler, LSL_Leaf *type, LSL_Leaf *identifier, LSL_Leaf *open, LSL_Leaf *params, LSL_Leaf *close)
{
    LSL_Function *func = NULL;

    if (params)
    {
	func = params->value.functionValue;
	// At this point, params is no longer needed, except if we are doing diff.
	// open and close are not needed either if we are not doing diff.
	if (func)
	{
	    if (identifier)
	    {
		func->name.text = identifier->value.stringValue;
#if LUASL_DIFF_CHECK
		func->name.ignorable = identifier->ignorable;
		identifier->ignorable = NULL;
#endif
		identifier->toKen = tokens[LSL_FUNCTION - lowestToken];
		identifier->value.functionValue = func;
		if (type)
		{
		    func->type.text = type->toKen->toKen;
#if LUASL_DIFF_CHECK
		    func->type.ignorable = type->ignorable;
		    type->ignorable = NULL;
#endif
		    identifier->basicType = type->basicType;
		}
		else
		    identifier->basicType = OT_nothing;
		if (compiler->inState)
		    eina_hash_add(compiler->state.handlers, func->name.text, func);
		else
		    eina_hash_add(compiler->script.functions, func->name.text, identifier);
#if LUASL_DIFF_CHECK
//		func->params = addParenthesis(open, params, LSL_PARAMETER_LIST, close);
#endif
	    }
	    compiler->currentFunction = func;
	}
    }

    return identifier;
}

LSL_Leaf *addFunctionBody(LuaSL_compiler *compiler, LSL_Leaf *function, LSL_Leaf *block)
{
    LSL_Leaf *statement = NULL;

    if (function)
    {
	function->value.functionValue->block = block->value.blockValue;
	statement = addStatement(compiler, NULL, function, NULL, function, NULL, NULL, NULL);
    }

    return statement;
}

LSL_Leaf *collectArguments(LuaSL_compiler *compiler, LSL_Leaf *list, LSL_Leaf *comma, LSL_Leaf *arg)
{
    LSL_FunctionCall *call = NULL;

    if (NULL == list)
	list = newLeaf(LSL_FUNCTION_CALL, NULL, NULL);

    if (list)
    {
	call = list->value.functionCallValue;
	if (NULL == call)
	{
	    call = calloc(1, sizeof(LSL_FunctionCall));
	    if (call)
	    {
		list->value.functionCallValue = call;
		eina_inarray_setup(&(call->params), sizeof(LSL_Leaf), 3);
	    }
	}

	if (call)
	{
	    if (arg)
	    {
		if (LUASL_DIFF_CHECK)
		{
		    // Stash the comma for diff later.
		    if (comma)
			eina_inarray_append(&(call->params), comma);
		}
		eina_inarray_append(&(call->params), arg);
		// At this point, pointers to arg are not pointing to the one in call->params, AND arg is no longer needed.
	    }
	}
    }
    return list;
}

LSL_Leaf *addFunctionCall(LuaSL_compiler *compiler, LSL_Leaf *identifier, LSL_Leaf *open, LSL_Leaf *params, LSL_Leaf *close)
{
    LSL_Leaf *func = findFunction(compiler, identifier->value.stringValue);
    LSL_FunctionCall *call = NULL;

    if (params)
    {
	call = params->value.functionCallValue;
    }
    else
	call = calloc(1, sizeof(LSL_FunctionCall));

    if (func)
    {
	if (call)
	{
	    call->function = func->value.functionValue;
	    eina_clist_element_init(&(call->dangler));
	}
	identifier->value.functionCallValue = call;
	identifier->toKen = tokens[LSL_FUNCTION_CALL - lowestToken];
	identifier->basicType = func->basicType;
    }
    else
    {
	// It may be declared later, so store it and check later.
	if (call)
	{
	    eina_clist_add_tail(&(compiler->danglingCalls), &(call->dangler));
	    call->call = identifier;
	}
	// Here the identifier stringValue needs to be kept for later searching.
	identifier->toKen = tokens[LSL_UNKNOWN - lowestToken];
	identifier->basicType = OT_undeclared;
	compiler->undeclared = TRUE;
    }

    return identifier;
}

LSL_Leaf *addList(LSL_Leaf *left, LSL_Leaf *list, LSL_Leaf *right)
{
    left = addParenthesis(left, list, LSL_LIST, right);
    left->toKen = tokens[LSL_LIST - lowestToken];
    left->basicType = OT_list;
    return left;
}

LSL_Leaf *addRotVec(LSL_Leaf *left, LSL_Leaf *list, LSL_Leaf *right)
{
    LSL_Type type = LSL_ROTATION;
    opType  otype = OT_rotation;

    // TODO - count the members of list to see if it's a vector.
    left = addParenthesis(left, list, type, right);
    left->toKen = tokens[type - lowestToken];
    left->basicType = otype;
    return left;
}

LSL_Leaf *addNumby(LSL_Leaf *numby)
{
    LSL_Numby *num = calloc(1, sizeof(LSL_Numby));

    if ((numby) && (num))
    {
	num->text.text = numby->value.stringValue;
#if LUASL_DIFF_CHECK
	num->text.ignorable = numby->ignorable;
	numby->ignorable = NULL;
#endif
	switch (numby->toKen->type)
	{
	    case LSL_FLOAT :
	    {
		num->value.floatValue = atof(num->text.text);
		numby->basicType = OT_float;
		break;
	    }
	    case LSL_INTEGER :
	    {
		num->value.integerValue = atoi(num->text.text);
		numby->basicType = OT_integer;
		break;
	    }
	    default:
		break;
	}
	numby->value.numbyValue = num;
	num->type = numby->basicType;
    }
    return numby;
}

LSL_Leaf *addParenthesis(LSL_Leaf *lval, LSL_Leaf *expr, LSL_Type type, LSL_Leaf *rval)
{
    LSL_Parenthesis *parens = calloc(1, sizeof(LSL_Parenthesis));

    if (parens)
    {
	parens->contents = expr;
	parens->type = type;
#if LUASL_DIFF_CHECK
	parens->rightIgnorable = rval->ignorable;
	// Actualy, at this point, rval is no longer needed.
	rval->ignorable = NULL;
#endif
	if (lval)
	{
	    lval->value.parenthesis = parens;
	    if (expr)
	    {
		lval->basicType = expr->basicType;
		// Propagate these flags inwards and outwards.
		if (MF_ASSIGNEXP & expr->flags)
		    lval->flags |= MF_ASSIGNEXP;
		if (MF_WRAPFUNC & expr->flags)
		    parens->flags |= MF_WRAPFUNC;
	    }
	}
    }
    return lval;
}

LSL_Leaf *addState(LuaSL_compiler *compiler, LSL_Leaf *state, LSL_Leaf *identifier, LSL_Leaf *block)
{
    LSL_State *result = calloc(1, sizeof(LSL_State));

    if ((identifier) && (result))
    {
	Eina_Iterator *handlers;
	LSL_Function *func;

	memcpy(result, &(compiler->state), sizeof(LSL_State));
	compiler->state.block = NULL;
	compiler->state.handlers = NULL;
	result->name.text = identifier->value.stringValue;
#if LUASL_DIFF_CHECK
	result->name.ignorable = identifier->ignorable;
	identifier->ignorable = NULL;
#endif
	handlers = eina_hash_iterator_data_new(result->handlers);
	while(eina_iterator_next(handlers, (void **) &func))
	{
	    func->state = result->name.text;
	}
	result->block = block->value.blockValue;
	if (state)
	{
	    result->state.text = state->toKen->toKen;
#if LUASL_DIFF_CHECK
	    result->state.ignorable = state->ignorable;
	    state->ignorable = NULL;
#endif
	}
	identifier->value.stateValue = result;
	identifier->toKen = tokens[LSL_STATE - lowestToken];
	eina_hash_add(compiler->script.states, result->name.text, identifier);
	compiler->inState = FALSE;
    }

    return identifier;
}

LSL_Leaf *addIfElse(LuaSL_compiler *compiler, LSL_Leaf *ifBlock, LSL_Leaf *elseBlock)
{
    if (ifBlock->value.statementValue->elseBlock)
    {
	LSL_Statement *oldElseIf = ifBlock->value.statementValue->elseBlock;

	while (oldElseIf->elseBlock)
	    oldElseIf = oldElseIf->elseBlock;

	oldElseIf->elseBlock = elseBlock->value.statementValue;
    }
    else
	ifBlock->value.statementValue->elseBlock = elseBlock->value.statementValue;
    return ifBlock;
}

LSL_Leaf *addFor(LuaSL_compiler *compiler, LSL_Leaf *lval, LSL_Leaf *flow, LSL_Leaf *left, LSL_Leaf *expr0, LSL_Leaf *stat0, LSL_Leaf *expr1, LSL_Leaf *stat1, LSL_Leaf *expr2, LSL_Leaf *right, LSL_Leaf *block)
{
    LSL_Leaf **exprs = calloc(5, sizeof(LSL_Leaf *));

    if (exprs)
    {
	lval = addStatement(compiler, lval, flow, left, expr0, right, block, NULL);
	exprs[0] = expr0;
	exprs[1] = stat0;
	exprs[2] = expr1;
	exprs[3] = stat1;
	exprs[4] = expr2;
	lval->value.statementValue->expressions = (LSL_Leaf *) exprs;
    }
    return lval;
}

LSL_Leaf *addStatement(LuaSL_compiler *compiler, LSL_Leaf *lval, LSL_Leaf *flow, LSL_Leaf *left, LSL_Leaf *expr, LSL_Leaf *right, LSL_Leaf *block, LSL_Leaf *identifier)
{
    gameGlobals *game = compiler->game;
    LSL_Statement *stat = calloc(1, sizeof(LSL_Statement));
    boolean justOne = FALSE;

    if (NULL == lval)
	lval = newLeaf(LSL_STATEMENT, NULL, NULL);

    if (stat)
    {
	stat->type = flow->toKen->type;
	stat->expressions = expr;
	if (block)
	{
	    if (LSL_BLOCK_OPEN == block->toKen->type)
		stat->block = block->value.blockValue;
	    else
		stat->single = block->value.statementValue;
	}
	eina_clist_element_init(&(stat->statement));
	if (identifier)
	{
	    stat->identifier.text = identifier->value.stringValue;
#if LUASL_DIFF_CHECK
	    stat->identifier.ignorable = identifier->ignorable;
	    identifier->ignorable = NULL;
#endif
	}
	if (left)
	{
	    LSL_Leaf *parens = addParenthesis(left, expr, LSL_EXPRESSION, right);

	    if (parens)
		stat->parenthesis = parens->value.parenthesis;
	}

	switch (stat->type)
	{
	    case LSL_EXPRESSION :
	    {
		break;
	    }
	    case LSL_FUNCTION :
	    {
		break;
	    }
	    case LSL_DO :
	    {
		break;
	    }
	    case LSL_FOR :
	    {
		justOne = TRUE;
		break;
	    }
	    case LSL_IF :
	    {
		justOne = TRUE;
		break;
	    }
	    case LSL_ELSE :
	    {
		justOne = TRUE;
		break;
	    }
	    case LSL_ELSEIF :
	    {
		justOne = TRUE;
		break;
	    }
	    case LSL_JUMP :
	    {
		justOne = TRUE;
		break;
	    }
	    case LSL_LABEL :
	    {
		justOne = TRUE;
		break;
	    }
	    case LSL_RETURN :
	    {
		justOne = TRUE;
		break;
	    }
	    case LSL_STATE_CHANGE :
	    {
		justOne = TRUE;
		break;
	    }
	    case LSL_STATEMENT :
	    {
		break;
	    }
	    case LSL_WHILE :
	    {
		stat->identifier.text = NULL;
		justOne = TRUE;
		break;
	    }
	    case LSL_IDENTIFIER :
	    {
		break;
	    }
	    case LSL_VARIABLE :
	    {
		if (identifier)
		{
		    stat->identifier.text = identifier->value.identifierValue->name.text;
		    identifier->value.identifierValue->definition = stat;
		    stat->flags = identifier->value.identifierValue->flags;
		}
		break;
	    }
	    default :
	    {
		compiler->script.bugCount++;
		PE("Should not be here %d.", stat->type);
		break;
	    }
	}

#if LUASL_DIFF_CHECK
	if (justOne && (flow))
	{
	    stat->ignorable = calloc(2, sizeof(Eina_Strbuf *));
	    if (stat->ignorable)
	    {
		stat->ignorable[1] = flow->ignorable;
		flow->ignorable = NULL;
	    }
	}
#endif

	if (lval)
	{
#if LUASL_DIFF_CHECK
	    if (NULL == stat->ignorable)
		stat->ignorable = calloc(1, sizeof(Eina_Strbuf *));
	    if (stat->ignorable)
	    {
		stat->ignorable[0] = lval->ignorable;
		lval->ignorable = NULL;
	    }
#endif
	    lval->value.statementValue = stat;
	}

#if LUASL_DIFF_CHECK
	if (left)
	{
	    if (NULL == stat->ignorable)
		stat->ignorable = calloc(3, sizeof(Eina_Strbuf *));
	    else
		stat->ignorable = realloc(stat->ignorable, 3 * sizeof(Eina_Strbuf *));
	    if (stat->ignorable)
	    {
		stat->ignorable[2] = left->ignorable;
		left->ignorable = NULL;
	    }
	}
#endif
    }

    return lval;
}

LSL_Leaf *collectStatements(LuaSL_compiler *compiler, LSL_Leaf *list, LSL_Leaf *statement)
{
// Guess this is not needed after all, and seemed to cause the "states with only one function get dropped" bug.
//    boolean wasNull = FALSE;

    if (NULL == list)
    {
	list = newLeaf(LSL_BLOCK_OPEN, NULL, NULL);
//	wasNull = TRUE;
    }

    if (list)
    {
	if (statement)
	{
//	    if (!wasNull)
		list->value.blockValue = compiler->currentBlock;	// Maybe NULL.

	    if ((compiler->inState) && (LSL_FUNCTION == statement->value.statementValue->type))
	    {
		    eina_clist_add_tail(&(compiler->state.block->statements), &(statement->value.statementValue->statement));
	    }
	    else if (list->value.blockValue)
	    {
		eina_clist_add_tail(&(list->value.blockValue->statements), &(statement->value.statementValue->statement));
	    }
	}
    }

    return list;
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
			    Strings in hexadecimal format will work, same in Lua (though Lua can't handle "0x", but "0x0" is fine).
			keys <-> string
			    No other typecasting can be done with keys.
			float -> string
			    You get a bunch of trailing 0s.

QUIRK - I have seen cases where a double explicit typecast was needed in SL, but was considered to be invalid syntax in OS.

Any binary operation involving a float and an integer implicitly casts the integer to float.

A boolean operation deals with TRUE (1) and FALSE (0).  Any non zero value is a TRUE (generally sigh).
On the other hand, in Lua, only false and nil are false, everything else is true.  0 is true.  sigh
Bitwise operations only apply to integers.  Right shifts are arithmetic, not logical.

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

LSL_Leaf *addTypecast(LSL_Leaf *lval, LSL_Leaf *type, LSL_Leaf *rval, LSL_Leaf *expr)
{
    addParenthesis(lval, expr, LSL_TYPECAST_OPEN, rval);
    if (lval)
    {
	if (type)
	{
	    lval->basicType = type->basicType;
	    if ((expr) && (OT_integer == type->basicType))  // TODO - Should be from string, but I guess I'm not propagating basic types up from function calls and parenthesis?
		lval->value.parenthesis->flags |= MF_TYPECAST;
	}
	// Actualy, at this point, type is no longer needed.
	lval->toKen = tokens[LSL_TYPECAST_OPEN - lowestToken];
    }
//    if (rval)
//	rval->toKen = tokens[LSL_TYPECAST_CLOSE - lowestToken];

    return lval;
}

LSL_Leaf *addVariable(LuaSL_compiler *compiler, LSL_Leaf *type, LSL_Leaf *identifier, LSL_Leaf *assignment, LSL_Leaf *expr)
{
    LSL_Identifier *result = calloc(1, sizeof(LSL_Identifier));

    if ( (identifier) && (result))
    {
	result->name.text = identifier->value.stringValue;
#if LUASL_DIFF_CHECK
	result->name.ignorable = identifier->ignorable;
	identifier->ignorable = NULL;
#endif
	result->value.toKen = tokens[LSL_UNKNOWN - lowestToken];
	identifier->value.identifierValue = result;
	identifier->toKen = tokens[LSL_VARIABLE - lowestToken];
	identifier->left = type;
	identifier->right = assignment;
	if (assignment)
	    assignment->right = expr;
	else
	    identifier->flags |= MF_NOASSIGN;
	if (type)
	{
	    if (compiler->currentBlock)
	    {
		identifier->flags |= MF_LOCAL;
		result->flags |= MF_LOCAL;
		type->flags |= MF_LOCAL;
	    }
	    identifier->basicType = type->basicType;
	    result->value.basicType = type->basicType;
	    result->value.toKen = type->toKen;	// This is the LSL_TYPE_* toKen instead of the LSL_* toKen.  Not sure if that's a problem.
	}
	if (compiler->currentBlock)
	    eina_hash_add(compiler->currentBlock->variables, result->name.text, identifier);
	else
	    eina_hash_add(compiler->script.variables, result->name.text, identifier);
    }

    return identifier;
}

LSL_Leaf *beginBlock(LuaSL_compiler *compiler, LSL_Leaf *block)
{
    LSL_Block *blok = calloc(1, sizeof(LSL_Block));

    if (blok)
    {
	eina_clist_init(&(blok->statements));
	blok->variables = eina_hash_stringshared_new(burnLeaf);
	block->value.blockValue = blok;
	if ((NULL == compiler->currentBlock) && (NULL == compiler->currentFunction))
	{
	    compiler->inState = TRUE;
	    compiler->state.block=blok;
	    compiler->state.handlers = eina_hash_stringshared_new(free);
	}
	blok->outerBlock = compiler->currentBlock;
	compiler->currentBlock = blok;
	blok->function = compiler->currentFunction;
	compiler->currentFunction = NULL;
#if LUASL_DIFF_CHECK
	blok->openIgnorable = block->ignorable;
	block->ignorable = NULL;
#endif
    }
    return block;
}

static void secondPass(LuaSL_compiler *compiler, LSL_Leaf *leaf)
{
    if (leaf)
    {
	secondPass(compiler, leaf->left);
	if (OT_undeclared == leaf->basicType)
	    leaf = addOperation(compiler, leaf->left, leaf, leaf->right);
	secondPass(compiler, leaf->right);
    }
}

static void outputLeaf(FILE *file, outputMode mode, LSL_Leaf *leaf)
{
    if (leaf)
    {
	if ((OM_LUA == mode) &&(ST_BITWISE != leaf->toKen->subType))
	    outputLeaf(file, mode, leaf->left);
#if LUASL_DIFF_CHECK
	if ((!(LSL_NOIGNORE & leaf->toKen->flags)) && (leaf->ignorable))
	    fwrite(eina_strbuf_string_get(leaf->ignorable), 1, eina_strbuf_length_get(leaf->ignorable), file);
#endif
	if (leaf->toKen->output)
	    leaf->toKen->output(file, mode, leaf);
	else
	{
	    if (OM_LUA == mode)
	    {
		if (MF_WRAPFUNC & leaf->flags)
		{
// TODO - Leaving this here in case we trip over one.
if ((leaf->left) && (leaf->right))
    printf("%s %s %s\n", leaf->left->toKen->toKen, leaf->toKen->toKen, leaf->right->toKen->toKen);
else if (leaf->left)
    printf("%s %s NORIGHT\n", leaf->left->toKen->toKen, leaf->toKen->toKen);
else if (leaf->right)
    printf("NOLEFT %s %s\n", leaf->toKen->toKen, leaf->right->toKen->toKen);
else
    printf("NOLEFT %s NORIGHT\n", leaf->toKen->toKen);
		}
		if ((LSL_ASSIGNMENT & leaf->toKen->flags) && (LSL_ASSIGNMENT_PLAIN != leaf->toKen->type))
		{
		    if (leaf->left->value.identifierValue->sub)
			fprintf(file, " --[[%s]] = %s.%s %.1s ", leaf->toKen->toKen, leaf->left->value.identifierValue->name.text, leaf->left->value.identifierValue->sub, leaf->toKen->toKen);
		    else
			fprintf(file, " --[[%s]] = %s %.1s ", leaf->toKen->toKen, leaf->left->value.identifierValue->name.text, leaf->toKen->toKen);
		}
		else if (LSL_TYPE & leaf->toKen->flags)
		{
		    if (MF_LOCAL & leaf->flags)
			fprintf(file, "  local ");
		    fprintf(file, " --[[%s]] ", leaf->toKen->toKen);
		}
		else if (LSL_BOOL_AND == leaf->toKen->type)
		    fprintf(file, " and ");
		else if (LSL_BOOL_OR == leaf->toKen->type)
		    fprintf(file, " or ");
		else if (LSL_BOOL_NOT == leaf->toKen->type)
		    fprintf(file, " not ");
		else if (LSL_CONCATENATE == leaf->toKen->type)
		    fprintf(file, " .. ");
		else if (LSL_NOT_EQUAL == leaf->toKen->type)
		    fprintf(file, " ~= ");
		else
		    fprintf(file, "%s", leaf->toKen->toKen);
	    }
	    else
		fprintf(file, "%s", leaf->toKen->toKen);
	}
	if ((OM_LUA == mode) &&(ST_BITWISE != leaf->toKen->subType))
	    outputLeaf(file, mode, leaf->right);
    }
}

// Circular references, so declare this one first.
static void outputRawStatement(FILE *file, outputMode mode, LSL_Statement *statement);

static void outputRawBlock(FILE *file, outputMode mode, LSL_Block *block, boolean doEnd)
{
    if (block)
    {
	LSL_Statement *stat = NULL;

#if LUASL_DIFF_CHECK
	if (block->openIgnorable)
	    fwrite(eina_strbuf_string_get(block->openIgnorable), 1, eina_strbuf_length_get(block->openIgnorable), file);
	if (OM_LSL == mode)
	    fprintf(file, "{");
#else
	if (OM_LSL == mode)
	    fprintf(file, "\n{\n");
	else if (doEnd && (OM_LUA == mode))
	    fprintf(file, "\n");
#endif
	EINA_CLIST_FOR_EACH_ENTRY(stat, &(block->statements), LSL_Statement, statement)
	{
		outputRawStatement(file, mode, stat);
	}
#if LUASL_DIFF_CHECK
	if (block->closeIgnorable)
	    fwrite(eina_strbuf_string_get(block->closeIgnorable), 1, eina_strbuf_length_get(block->closeIgnorable), file);
#endif
	if (OM_LSL == mode)
	    fprintf(file, "}");
	else if (doEnd && (OM_LUA == mode))
	    fprintf(file, "end ");
    }
}

// TODO - should clean this up by refactoring the bits in the switch outside.
static void outputRawParenthesisToken(FILE *file, outputMode mode, LSL_Parenthesis *parenthesis, const char *typeName)
{
    if ((OM_LUA == mode) && (LSL_TYPECAST_OPEN == parenthesis->type))
    {
	if (MF_TYPECAST & parenthesis->flags)
	    fprintf(file, " _LSL.%sTypecast(", typeName);
	else
	    fprintf(file, " --[[%s]] ", typeName);
	outputLeaf(file, mode, parenthesis->contents);
	if (MF_TYPECAST & parenthesis->flags)
	    fprintf(file, ") ");
	return;
    }

    if ((OM_LUA == mode) && (MF_WRAPFUNC & parenthesis->flags))
	fprintf(file, " (function() ");
    else
	fprintf(file, "(");
    if (LSL_TYPECAST_OPEN == parenthesis->type)
	fprintf(file, "%s", typeName);	// TODO - We are missing the type ignorable text here.
    else
	outputLeaf(file, mode, parenthesis->contents);
    if ((OM_LUA == mode) && (MF_WRAPFUNC & parenthesis->flags))
	fprintf(file, "; return x; end)() ");
    else
    {
#if LUASL_DIFF_CHECK
	fprintf(file, "%s)", eina_strbuf_string_get(parenthesis->rightIgnorable));
#else
	fprintf(file, ")");
#endif
    }

    if (LSL_TYPECAST_OPEN == parenthesis->type)
	outputLeaf(file, mode, parenthesis->contents);
}

static void outputText(FILE *file, LSL_Text *text, boolean ignore)
{
	    if (text->text)
	    {
#if LUASL_DIFF_CHECK
		if (ignore && (text->ignorable))
		    fwrite(eina_strbuf_string_get(text->ignorable), 1, eina_strbuf_length_get(text->ignorable), file);
#endif
		fprintf(file, "%s", text->text);
	    }
}

static void outputRawStatement(FILE *file, outputMode mode, LSL_Statement *statement)
{
    boolean isBlock = FALSE;

    if (statement)
    {
	switch (statement->type)
	{
	    case LSL_EXPRESSION :
	    {
		break;
	    }
	    case LSL_FUNCTION :
	    {
		isBlock = TRUE;
		break;
	    }
	    case LSL_DO :
	    {
		fprintf(file, "%s", tokens[statement->type - lowestToken]->toKen);
		break;
	    }
	    case LSL_FOR :
	    {
#if LUASL_DIFF_CHECK
		if ((statement->ignorable) && (statement->ignorable[1]))
		    fwrite(eina_strbuf_string_get(statement->ignorable[1]), 1, eina_strbuf_length_get(statement->ignorable[1]), file);
#endif
		if (OM_LSL == mode)
		{
		    isBlock = TRUE;
		    fprintf(file, "%s", tokens[statement->type - lowestToken]->toKen);
		}
		else if (OM_LUA == mode)
		{
		    LSL_Leaf **exprs = (LSL_Leaf **) statement->expressions;

		    outputLeaf(file, mode, exprs[0]);
		    fprintf(file, ";\nwhile (");
		    outputLeaf(file, mode, exprs[2]);
#if LUASL_DIFF_CHECK
		    fprintf(file, "%s)\n", eina_strbuf_string_get(statement->parenthesis->rightIgnorable));
#else
		    fprintf(file, ") do\n");
#endif
		    if (statement->block)
			outputRawBlock(file, mode, statement->block, FALSE);
		    if (statement->single)
			outputRawStatement(file, mode, statement->single);
		    fprintf(file, "\n");
		    outputLeaf(file, mode, exprs[4]);
		    fprintf(file, ";\nend\n");
		    return;
		}
		break;
	    }
	    case LSL_IF :
	    case LSL_ELSE :
	    case LSL_ELSEIF :
	    {
		isBlock = TRUE;
#if LUASL_DIFF_CHECK
		if ((statement->ignorable) && (statement->ignorable[1]))
		    fwrite(eina_strbuf_string_get(statement->ignorable[1]), 1, eina_strbuf_length_get(statement->ignorable[1]), file);
#endif
		fprintf(file, "%s", tokens[statement->type - lowestToken]->toKen);
		if (OM_LUA == mode)
		{
		    fprintf(file, " ");
		    if (LSL_ELSE != statement->type)
		    {
			if (statement->parenthesis)
			    outputRawParenthesisToken(file, mode, statement->parenthesis, "");
			else
			    outputLeaf(file, mode, statement->expressions);
			fprintf(file, " then\n");
		    }
		    if (statement->block)
			outputRawBlock(file, mode, statement->block, FALSE);
		    if (statement->single)
			outputRawStatement(file, mode, statement->single);
		    if (statement->elseBlock)
			outputRawStatement(file, mode, statement->elseBlock);
		    if (LSL_IF == statement->type)
		    {
#if 1
			fprintf(file, " end\n");
#else
			fprintf(file, " end --[[");
			if (statement->parenthesis)
			    outputRawParenthesisToken(file, mode, statement->parenthesis, "");
			else
			    outputLeaf(file, mode, statement->expressions);
			fprintf(file, "]]\n");
#endif
		    }
		    return;
		}
		break;
	    }
	    case LSL_JUMP :
	    {
#if LUASL_DIFF_CHECK
		if ((statement->ignorable) && (statement->ignorable[1]))
		    fwrite(eina_strbuf_string_get(statement->ignorable[1]), 1, eina_strbuf_length_get(statement->ignorable[1]), file);
#endif
		fprintf(file, "%s", tokens[statement->type - lowestToken]->toKen);
		break;
	    }
	    case LSL_LABEL :
	    {
#if LUASL_DIFF_CHECK
		if ((statement->ignorable) && (statement->ignorable[1]))
		    fwrite(eina_strbuf_string_get(statement->ignorable[1]), 1, eina_strbuf_length_get(statement->ignorable[1]), file);
#endif
		fprintf(file, "%s", tokens[statement->type - lowestToken]->toKen);
		break;
	    }
	    case LSL_RETURN :
	    {
#if LUASL_DIFF_CHECK
	    if ((statement->ignorable) && (statement->ignorable[1]))
		fwrite(eina_strbuf_string_get(statement->ignorable[1]), 1, eina_strbuf_length_get(statement->ignorable[1]), file);
#endif
		fprintf(file, "%s", tokens[statement->type - lowestToken]->toKen);
		break;
	    }
	    case LSL_STATE_CHANGE :
	    {
#if LUASL_DIFF_CHECK
	    if ((statement->ignorable) && (statement->ignorable[1]))
		fwrite(eina_strbuf_string_get(statement->ignorable[1]), 1, eina_strbuf_length_get(statement->ignorable[1]), file);
#endif
		if (OM_LSL == mode)
		{
		    fprintf(file, "%s", tokens[statement->type - lowestToken]->toKen);
		    if (statement->identifier.text)
			outputText(file, &(statement->identifier), TRUE);
		}
		else if (OM_LUA == mode)
		{
		    fprintf(file, "_LSL.stateChange(_");
		    if (statement->identifier.text)
			outputText(file, &(statement->identifier), TRUE);
		    fprintf(file, "State)");
		}
		break;
	    }
	    case LSL_STATEMENT :
	    {
		break;
	    }
	    case LSL_WHILE :
	    {
		isBlock = TRUE;
#if LUASL_DIFF_CHECK
		if ((statement->ignorable) && (statement->ignorable[1]))
		    fwrite(eina_strbuf_string_get(statement->ignorable[1]), 1, eina_strbuf_length_get(statement->ignorable[1]), file);
#else
		if (OM_LUA == mode)
		    fprintf(file, "\n");
#endif
		fprintf(file, "%s", tokens[statement->type - lowestToken]->toKen);
		if (OM_LUA == mode)
		{
		    if (statement->parenthesis)
			outputRawParenthesisToken(file, mode, statement->parenthesis, "");
		    fprintf(file, " do ");
		    if (statement->block)
			outputRawBlock(file, mode, statement->block, TRUE);
		    if (statement->single)
			outputRawStatement(file, mode, statement->single);
		    fprintf(file, "\n");
		    return;
		}
		break;
	    }
	    case LSL_IDENTIFIER :
	    {
		break;
	    }
	    case LSL_VARIABLE :
	    {
		break;
	    }
	    default :
	    {
		fprintf(file, "@@Should not be here %s.@@", tokens[statement->type - lowestToken]->toKen);
		break;
	    }
	}

#if LUASL_DIFF_CHECK
	if ((statement->ignorable) && (statement->ignorable[2]))
	    fwrite(eina_strbuf_string_get(statement->ignorable[2]), 1, eina_strbuf_length_get(statement->ignorable[2]), file);
#else
	if (OM_LUA == mode)
	    fprintf(file, " ");
#endif
	if (LSL_FOR == statement->type)
	{
	    LSL_Leaf **exprs = (LSL_Leaf **) statement->expressions;
	    int i;

	    fprintf(file, "(");
	    for (i = 0; i < 5; i++)
	    {
		outputLeaf(file, mode, exprs[i]);
		if (i % 2)
		    fprintf(file, ";");
	    }
#if LUASL_DIFF_CHECK
	    fprintf(file, "%s)", eina_strbuf_string_get(statement->parenthesis->rightIgnorable));
#else
	    fprintf(file, ")");
#endif
	}
	else if (statement->parenthesis)
	    outputRawParenthesisToken(file, mode, statement->parenthesis, "");
	else
	    outputLeaf(file, mode, statement->expressions);

	if (statement->block)
	    outputRawBlock(file, mode, statement->block, TRUE);
	if (statement->single)
	    outputRawStatement(file, mode, statement->single);

#if LUASL_DIFF_CHECK
	if ((statement->ignorable) && (statement->ignorable[0]))
	    fwrite(eina_strbuf_string_get(statement->ignorable[0]), 1, eina_strbuf_length_get(statement->ignorable[0]), file);
#endif

	if (!isBlock)
	{
	    fprintf(file, ";");
	    if (!LUASL_DIFF_CHECK)
		fprintf(file, "\n");
	}

	if ((LSL_VARIABLE == statement->type) && (OM_LUA == mode) && (MF_LOCAL & statement->flags))
	{
	    const char *name = statement->identifier.text;

//	    if ((MF_PREDEC | MF_PREINC | MF_POSTDEC | MF_POSTINC)  & statement->flags)
//		fprintf(file, "\n");
	    if (MF_PREDEC  & statement->flags)	fprintf(file, "local function _preDecrement_%s() %s = %s - 1;  return %s;  end\n", name, name, name, name);
	    if (MF_PREINC  & statement->flags)	fprintf(file, "local function _preIncrement_%s() %s = %s + 1;  return %s;  end\n", name, name, name, name);
	    if (MF_POSTDEC & statement->flags)	fprintf(file, "local function _postDecrement_%s() local _temp = %s; %s = %s - 1;  return _temp;  end\n", name, name, name, name);
	    if (MF_POSTINC & statement->flags)	fprintf(file, "local function _postDecrement_%s() local _temp = %s; %s = %s + 1;  return _temp;  end\n", name, name, name, name);
	}

	if (statement->elseBlock)
	    outputRawStatement(file, mode, statement->elseBlock);
    }
}

static void outputBitOp(FILE *file, outputMode mode, LSL_Leaf *leaf)
{
    if (OM_LSL == mode)
	outputLeaf(file, mode, leaf);
    else if (OM_LUA == mode)
    {
	switch (leaf->toKen->type)
	{
	    case LSL_BIT_AND :		fprintf(file, " _bit.band(");	break;
	    case LSL_BIT_OR  :		fprintf(file, " _bit.bor(");	break;
	    case LSL_BIT_XOR :		fprintf(file, " _bit.xor(");	break;
	    case LSL_BIT_NOT :		fprintf(file, " _bit.bnot(");	break;
	    case LSL_LEFT_SHIFT :	fprintf(file, " _bit.lshift(");	break;
	    case LSL_RIGHT_SHIFT :	fprintf(file, " _bit.arshift(");	break;
	    default : break;
	}
	outputLeaf(file, mode, leaf->left);
	if (LSL_BIT_NOT != leaf->toKen->type)
	{
	    fprintf(file, ", ");
	    outputLeaf(file, mode, leaf->right);
	}
	fprintf(file, ") ");
    }
}

static void outputBlockToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
	outputRawBlock(file, mode, content->value.blockValue, TRUE);
}

static void outputCrementsToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
    {
	if (OM_LSL == mode)
	{
	    switch (content->toKen->type)
	    {
		case LSL_DECREMENT_PRE :
		case LSL_INCREMENT_PRE :
		{
		    fprintf(file, "%s", content->toKen->toKen);
#if LUASL_DIFF_CHECK
		    if (content->value.identifierValue->ignorable)
			fwrite(eina_strbuf_string_get(content->value.identifierValue->ignorable), 1, eina_strbuf_length_get(content->value.identifierValue->ignorable), file);
#endif
		    outputText(file, &(content->value.identifierValue->name), FALSE);
		    break;
		}
		case LSL_DECREMENT_POST :
		case LSL_INCREMENT_POST :
		{
#if LUASL_DIFF_CHECK
		    if (content->value.identifierValue->ignorable)
			fwrite(eina_strbuf_string_get(content->value.identifierValue->ignorable), 1, eina_strbuf_length_get(content->value.identifierValue->ignorable), file);
#endif
		    outputText(file, &(content->value.identifierValue->name), FALSE);
		    fprintf(file, "%s", content->toKen->toKen);
		    break;
		}
		default :
		    break;
	    }
	}
	else if (OM_LUA == mode)
	{
	    if (MF_LOCAL & content->value.identifierValue->flags)
		fprintf(file, " _");
	    else
		fprintf(file, " _LSL.");
	    switch (content->toKen->type)
	    {
		case LSL_DECREMENT_PRE :	fprintf(file, "preDecrement");	break;
		case LSL_INCREMENT_PRE :	fprintf(file, "preIncrement");	break;
		case LSL_DECREMENT_POST :	fprintf(file, "postDecrement");	break;
		case LSL_INCREMENT_POST :	fprintf(file, "postIncrement");	break;
		default :
		    break;
	    }
	    if (MF_LOCAL & content->value.identifierValue->flags)
		fprintf(file, "_");
	    else
		fprintf(file, "(\"");
#if LUASL_DIFF_CHECK
	    if (content->value.identifierValue->ignorable)
		fwrite(eina_strbuf_string_get(content->value.identifierValue->ignorable), 1, eina_strbuf_length_get(content->value.identifierValue->ignorable), file);
#endif
	    outputText(file, &(content->value.identifierValue->name), FALSE);
	    if (MF_LOCAL & content->value.identifierValue->flags)
		fprintf(file, "()");
	    else
		fprintf(file, "\")");
	}
    }
}

static void outputFloatToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
	outputText(file, &(content->value.numbyValue->text), !(LSL_NOIGNORE & content->toKen->flags));
}

static void outputFunctionToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
    {
	LSL_Function *func = content->value.functionValue;
	LSL_Leaf *param = NULL;
	int first = TRUE;

	if (OM_LSL == mode)
	{
	    outputText(file, &(func->type), !(LSL_NOIGNORE & content->toKen->flags));
	    outputText(file, &(func->name), !(LSL_NOIGNORE & content->toKen->flags));
	    // TODO - should print comma and parenthesis ignorables.
	    fprintf(file, "(");
	    EINA_INARRAY_FOREACH((&(func->vars)), param)
	    {
		if (!LUASL_DIFF_CHECK)
		{
		    if (!first)
			fprintf(file, ", ");
		}
		outputLeaf(file, mode, param);
		first = FALSE;
	    }
	    fprintf(file, ")");
	    outputRawBlock(file, mode, func->block, TRUE);
	    if (!LUASL_DIFF_CHECK)
		fprintf(file, "\n");
	}
	else if (OM_LUA == mode)
	{
	    if (func->state)
		fprintf(file, "\n\n_%sState.%s = function(", func->state, func->name.text);
	    else
	    {
		fprintf(file, "\n\nfunction ");
		if (func->type.text)
		    fprintf(file, " --[[%s]] ", func->type.text);
		fprintf(file, "%s(", func->name.text);
	    }
	    EINA_INARRAY_FOREACH((&(func->vars)), param)
	    {
		// TODO - comment out param types.
		if (!LUASL_DIFF_CHECK)
		{
		    if (!first)
			fprintf(file, ", ");
		}
		outputLeaf(file, mode, param);
		first = FALSE;
	    }
	    fprintf(file, ")");
	    outputRawBlock(file, mode, func->block, TRUE);
	}
    }
}

static void outputFunctionCallToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
    {
	LSL_FunctionCall *call = content->value.functionCallValue;
	LSL_Leaf *param = NULL;
	boolean first = TRUE;

	// TODO - should output it's own ignorable here.
	if ((OM_LUA == mode) && (MF_LSLCONST & call->function->flags))
	    fprintf(file, "_LSL.");
	outputText(file, &(call->function->name), FALSE);	// Don't output the function definitions ignorable.
	fprintf(file, "(");
	EINA_INARRAY_FOREACH((&(call->params)), param)
	{
	    if ((OM_LUA == mode) && (!first))
		fprintf(file, ", ");
	    outputLeaf(file, mode, param);
	    first = FALSE;
	}
	fprintf(file, ")");
    }
}

static void outputIntegerToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
	outputText(file, &(content->value.numbyValue->text), !(LSL_NOIGNORE & content->toKen->flags));
}

static void outputIdentifierToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
    {
	if (LSL_IDENTIFIER == content->toKen->type)
	{
	    if ((OM_LUA == mode) && (MF_LSLCONST & content->value.identifierValue->flags))
		fprintf(file, "_LSL.");
	    outputText(file, &(content->value.identifierValue->name), FALSE);
	    if (content->value.identifierValue->sub)
		fprintf(file, ".%s", content->value.identifierValue->sub);
	}
	else
	if ((LSL_VARIABLE == content->toKen->type) && (MF_NOASSIGN & content->flags))
	{
	    outputText(file, &(content->value.identifierValue->name), !(LSL_NOIGNORE & content->toKen->flags));
	    if (OM_LUA == mode)
	    {
		switch (content->basicType)
		{
		    case OT_bool	:  fprintf(file, " = false");			break;
		    case OT_integer	:  fprintf(file, " = 0");			break;
		    case OT_float	:  fprintf(file, " = 0.0");			break;
		    case OT_key		:  fprintf(file, " = _LSL.NULL_KEY");		break;
		    case OT_list	:  fprintf(file, " = {}");			break;
		    case OT_rotation	:  fprintf(file, " = _LSL.ZERO_ROTATION");	break;
		    case OT_string	:  fprintf(file, " = \"\"");			break;
		    case OT_vector	:  fprintf(file, " = _LSL.ZERO_VECTOR");	break;
		    default		:  fprintf(file, " = nil");			break;
		}
	    }
	}
	else
	    outputText(file, &(content->value.identifierValue->name), !(LSL_NOIGNORE & content->toKen->flags));
    }
}

static void outputListToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
    {
	LSL_Parenthesis *parens = content->value.parenthesis;

	if (parens->contents)
	{
	    LSL_FunctionCall *call = parens->contents->value.functionCallValue;
	    LSL_Leaf *param = NULL;
	    const char *ig = "";

	    // TODO - should output it's own ignorable here.
	    if (OM_LSL == mode)
	    {
		switch (parens->type)
		{
		    case LSL_LIST     :  fprintf(file, "[");  break;
		    case LSL_ROTATION :
		    case LSL_VECTOR   :  fprintf(file, "<");
		    default           :  break;
		}
	    }
	    else if (OM_LUA == mode)
	    {
		switch (parens->type)
		{
		    case LSL_LIST     :  fprintf(file, "{");  break;
		    case LSL_ROTATION :
		    case LSL_VECTOR   :  fprintf(file, "{");
		    default           :  break;
		}
	    }
	    EINA_INARRAY_FOREACH((&(call->params)), param)
	    {
		outputLeaf(file, mode, param);
		if (OM_LUA == mode)
		    fprintf(file, ", ");
	    }
#if LUASL_DIFF_CHECK
	    ig = eina_strbuf_string_get(parens->rightIgnorable);
#endif
	    if (OM_LSL == mode)
	    {
		switch (parens->type)
		{
		    case LSL_LIST     :  fprintf(file, "%s]", ig);  break;
		    case LSL_ROTATION :
		    case LSL_VECTOR   :  fprintf(file, "%s>", ig);
		    default           :  break;
		}
	    }
	    else if (OM_LUA == mode)
	    {
		switch (parens->type)
		{
		    case LSL_LIST     :  fprintf(file, "%s}", ig);  break;
		    case LSL_ROTATION :
		    case LSL_VECTOR   :  fprintf(file, "%s}", ig);
		    default           :  break;
		}
	    }
	}
    }
}

static void outputParameterListToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
	outputLeaf(file, mode, content->value.listValue);
    // TODO - Should go through the list, and output any crements functions we need at the top of the block.
}

static void outputParenthesisToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
	outputRawParenthesisToken(file, mode, content->value.parenthesis, allowed[content->basicType].name);
}

static void outputStateToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
    {
	LSL_State *state = content->value.stateValue;

	if (state)
	{
	    if (OM_LSL == mode)
	    {
		outputText(file, &(state->state), !(LSL_NOIGNORE & content->toKen->flags));
		outputText(file, &(state->name), !(LSL_NOIGNORE & content->toKen->flags));
		outputRawBlock(file, mode, state->block, TRUE);
	    }
	    else if (OM_LUA == mode)
	    {
		fprintf(file, "\n\n--[[state]] _");
		outputText(file, &(state->name), !(LSL_NOIGNORE & content->toKen->flags));
		fprintf(file, "State = {};");
		outputRawBlock(file, mode, state->block, FALSE);
	    }
	}
    }
}

static void outputStatementToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
	outputRawStatement(file, mode, content->value.statementValue);
}

static void outputStringToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
	fprintf(file, "%s", content->value.stringValue);	// The quotes are part of the string value already.
}

static boolean doneParsing(LuaSL_compiler *compiler)
{
    gameGlobals *game = compiler->game;
    boolean result = FALSE;

    if (compiler->ast)
    {
	FILE *out;
	char buffer[PATH_MAX];
	char outName[PATH_MAX];
	char luaName[PATH_MAX];
	int count;

	if (LUASL_DIFF_CHECK)
	{
	    strcpy(outName, compiler->fileName);
	    strcat(outName, "2");
	    out = fopen(outName, "w");
	    if (out)
	    {
		char diffName[PATH_MAX];

		strcpy(diffName, compiler->fileName);
		strcat(diffName, ".diff");
		outputLeaf(out, OM_LSL, compiler->ast);
		fclose(out);
		sprintf(buffer, "diff -u \"%s\" \"%s\" > \"%s\"", compiler->fileName, outName, diffName);
		count = system(buffer);
		if (0 != count)
		    PE("LSL output file is different - %s!", outName);
		else
		    result = TRUE;
	    }
	    else
		PC("Unable to open file %s for writing!", outName);
	}
	strcpy(luaName, compiler->fileName);
	strcat(luaName, ".lua");
	out = fopen(luaName, "w");
	if (out)
	{
	    fprintf(out, "--// Generated code goes here.\n\n");
	    fprintf(out, "local _bit = require(\"bit\")\n");
	    fprintf(out, "local _LSL = require(\"LSL\")\n\n");
	    outputLeaf(out, OM_LUA, compiler->ast);
	    fprintf(out, "\n\n_LSL.stateChange(_defaultState)\n");  // This actually starts the script running.
	    fprintf(out, "\n--// End of generated code.\n\n");
	    fclose(out);
	    sprintf(buffer, "../../libraries/luajit-2.0/src/luajit \"%s\"", luaName);
	    count = system(buffer);
	    if (0 != count)
	    {
		compiler->script.bugCount++;
		PE("Lua compile stage failed for %s!", compiler->fileName);
	    }
	}
	else
	    PC("Unable to open file %s for writing!", luaName);
    }

    if (compiler->script.bugCount)
	PE("%d errors and %d warnings in %s", compiler->script.bugCount, compiler->script.warningCount, compiler->fileName);
    else
    {
	if (compiler->script.warningCount)
	    PW("%d errors and %d warnings in %s", compiler->script.bugCount, compiler->script.warningCount, compiler->fileName);
	else
	    PI("%d errors and %d warnings in %s", compiler->script.bugCount, compiler->script.warningCount, compiler->fileName);
	result = TRUE;
    }

    return result;
}

boolean compilerSetup(gameGlobals *game)
{
    int i;

    // Figure out what numbers lemon gave to our tokens.
    for (i = 0; LSL_Tokens[i].toKen != NULL; i++)
    {
	if (lowestToken > LSL_Tokens[i].type)
	    lowestToken = LSL_Tokens[i].type;
    }
    tokens = calloc(i + 1, sizeof(LSL_Token *));
    if (tokens)
    {
	char buf[PATH_MAX];

	// Sort the token table.
	for (i = 0; LSL_Tokens[i].toKen != NULL; i++)
	{
	    int j = LSL_Tokens[i].type - lowestToken;

	    tokens[j] = &(LSL_Tokens[i]);
	}

	// Compile the constants.
	snprintf(buf, sizeof(buf), "%s/src/constants.lsl", PACKAGE_DATA_DIR);
	compileLSL(game, buf, TRUE);

	return TRUE;
    }
    else
	PC("No memory for tokens!");

    return FALSE;
}

boolean compileLSL(gameGlobals *game, char *script, boolean doConstants)
{
    boolean result = FALSE;
    LuaSL_compiler compiler;
    void *pParser = ParseAlloc(malloc);
    int yv;

// Parse the LSL script, validating it and reporting errors.
//   Just pass all LSL constants and ll*() )function names through to Lua, assume they are globals there.

    memset(&compiler, 0, sizeof(LuaSL_compiler));
    compiler.game = game;
    compiler.script.functions = eina_hash_stringshared_new(burnLeaf);
    compiler.script.states = eina_hash_stringshared_new(burnLeaf);
    compiler.script.variables = eina_hash_stringshared_new(burnLeaf);
    eina_clist_init(&(compiler.danglingCalls));
#if LUASL_DIFF_CHECK
    compiler.ignorable = eina_strbuf_new();
#endif

    strncpy(compiler.fileName, script, PATH_MAX - 1);
    compiler.fileName[PATH_MAX - 1] = '\0';
    compiler.file = fopen(compiler.fileName, "r");
    if (NULL == compiler.file)
    {
	PE("Error opening file %s.", compiler.fileName);
	return FALSE;
    }
    printf("\n");
    PD("Compiling %s.", compiler.fileName);
    compiler.ast = NULL;
    compiler.lval = newLeaf(LSL_UNKNOWN, NULL, NULL);
    // Text editors usually start counting at 1, even programmers editors. mcedit is an exception, but you can deal with that yourself.
    compiler.column = 1;
    compiler.line = 1;

    if (yylex_init_extra(&compiler, &(compiler.scanner)))
	return result;
    if (LUASL_DEBUG)
    {
	yyset_debug(1, compiler.scanner);
	ParseTrace(stdout, "LSL_lemon ");
    }
    yyset_in(compiler.file, compiler.scanner);
    // on EOF yylex will return 0
    while((yv = yylex(compiler.lval, compiler.scanner)) != 0)
    {
	Parse(pParser, yv, compiler.lval, &compiler);
	if (LSL_SCRIPT == yv)
	    break;
	compiler.lval = newLeaf(LSL_UNKNOWN, NULL, NULL);
    }

    yylex_destroy(compiler.scanner);
    Parse (pParser, 0, compiler.lval, &compiler);
    ParseFree(pParser, free);

    if (compiler.undeclared)
    {
	PW("A second pass is needed to check if functions where used before they where declared.  To avoid this second pass, don't do that.");
	if (eina_clist_count(&(compiler.danglingCalls)))
	{
	    LSL_FunctionCall *call = NULL;

	    EINA_CLIST_FOR_EACH_ENTRY(call, &(compiler.danglingCalls), LSL_FunctionCall, dangler)
	    {
		LSL_Leaf *func = findFunction(&(compiler), call->call->value.stringValue);

		if (func)
		{
		    call->function = func->value.functionValue;
		    // Coz the leaf still had the stringValue from before.
		    call->call->value.functionCallValue = call;
		    call->call->toKen = tokens[LSL_FUNCTION_CALL - lowestToken];
		    call->call->basicType = func->basicType;
		}
		else
		    PE("Undeclared function %s called @ line %d, column %d!", call->call->value.stringValue, call->call->line, call->call->column);
	    }
	}
	secondPass(&compiler, compiler.ast);
	PD("Second pass completed.");
    }

    if (doConstants)
    {
	memcpy(&constants, &(compiler.script), sizeof(LSL_Script));
	result = TRUE;
    }
    else
	result = doneParsing(&compiler);

    if (NULL != compiler.file)
    {
	fclose(compiler.file);
	compiler.file = NULL;
    }

    if (!doConstants)
	burnLeaf(compiler.ast);

    return result;
}
