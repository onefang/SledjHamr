#include "LuaSL.h"


static LSL_Leaf *evaluateFloatToken(LSL_Leaf  *content, LSL_Leaf *left, LSL_Leaf *right);
static LSL_Leaf *evaluateIntegerToken(LSL_Leaf  *content, LSL_Leaf *left, LSL_Leaf *right);
static LSL_Leaf *evaluateNoToken(LSL_Leaf *content, LSL_Leaf *left, LSL_Leaf *right);
static LSL_Leaf *evaluateOperationToken(LSL_Leaf  *content, LSL_Leaf *left, LSL_Leaf *right);
static LSL_Leaf *eveluateParenthesisToken(LSL_Leaf *content, LSL_Leaf *left, LSL_Leaf *right);
static LSL_Leaf *evaluateStatementToken(LSL_Leaf *content, LSL_Leaf *left, LSL_Leaf *right);
static void outputBlockToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputFloatToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputFunctionToken(FILE *file, outputMode mode, LSL_Leaf *content);
static void outputFunctionCallToken(FILE *file, outputMode mode, LSL_Leaf *content);
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
// LUA   - Short circiuts boolean operations, and goes left to right.
// LUA   - "and" returns its first argument if it is false, otherwise, it returns its second argument. "or" returns its first argument if it is not false, otherwise it returns its second argument.
//           Note that the above means that "and/or" can return any type.
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
// LUA   - comparisons are always false if they are different types.  Tables, userdata, and functions are compared by reference.  Strings campare in alphabetical order, depending on current locale.
// LUA   - really only has three conditionals, as it translates a ~= b to not (a == b), a > b to b < a, and a >= b to b <= a.
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

// LUA precedence - (it has no bit operators, at least not until 5.2, but LuaJIT has them.)
// or
// and
// < > <= >= ~= ==
// ..
// + -
// * /
// not negate
// exponentiation (^)

    {LSL_TYPECAST_CLOSE,	ST_NONE,		")",	LSL_RIGHT2LEFT | LSL_UNARY,		NULL, evaluateNoToken},
    {LSL_TYPECAST_OPEN,		ST_NONE,		"(",	LSL_RIGHT2LEFT | LSL_UNARY,		outputParenthesisToken, evaluateOperationToken},
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
    {LSL_FUNCTION_CALL,		ST_NONE,	"funccall",	LSL_NONE,				outputFunctionCallToken, NULL},
    {LSL_IDENTIFIER,		ST_NONE,	"identifier",	LSL_NONE,				outputIdentifierToken, NULL},

    {LSL_LABEL,			ST_NONE,	"@",		LSL_NONE,				NULL, NULL},

    {LSL_DO,			ST_NONE,	"do",		LSL_NONE,				NULL, NULL},
    {LSL_FOR,			ST_NONE,	"for",		LSL_NONE,				NULL, NULL},
    {LSL_ELSE,			ST_NONE,	"else",		LSL_NONE,				NULL, NULL},
    {LSL_IF,			ST_NONE,	"if",		LSL_NONE,				NULL, NULL},
    {LSL_JUMP,			ST_NONE,	"jump",		LSL_NONE,				NULL, NULL},
    {LSL_RETURN,		ST_NONE,	"return",	LSL_NONE,				NULL, NULL},
    {LSL_STATE_CHANGE,		ST_NONE,	"state",	LSL_NONE,				NULL, NULL},
    {LSL_WHILE,			ST_NONE,	"while",	LSL_NONE,				NULL, NULL},
    {LSL_STATEMENT,		ST_NONE,	";",		LSL_NOIGNORE,				outputStatementToken, evaluateStatementToken},

    {LSL_BLOCK_CLOSE,		ST_NONE,	"}",		LSL_NONE,				NULL, NULL},
    {LSL_BLOCK_OPEN,		ST_NONE,	"{",		LSL_NONE,				outputBlockToken, NULL},
    {LSL_PARAMETER,		ST_NONE,	"parameter",	LSL_NONE,				outputIdentifierToken, NULL},
    {LSL_PARAMETER_LIST,	ST_NONE,	"plist",	LSL_NONE,				outputParameterListToken, NULL},
    {LSL_FUNCTION,		ST_NONE,	"function",	LSL_NONE,				outputFunctionToken, NULL},
    {LSL_DEFAULT,		ST_NONE,	"default",	LSL_NONE,				outputStateToken, NULL},
    {LSL_STATE,			ST_NONE,	"state",	LSL_NONE,				outputStateToken, NULL},
    {LSL_SCRIPT,		ST_NONE,	"",		LSL_NONE,				NULL,  NULL},

    {LSL_UNKNOWN,		ST_NONE,	"unknown",	LSL_NONE,				NULL, NULL},

    // A sentinal.
    {999999, ST_NONE, NULL, LSL_NONE, NULL, NULL}
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
//	if (LUASL_DIFF_CHECK)
//	    eina_strbuf_free(leaf->ignorableText);
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
	if (NULL == func)
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
	    var = eina_hash_find(constants.variables, name);
	if (NULL == var)
	    var = eina_hash_find(compiler->script.variables, name);
    }

    return var;
}

LSL_Leaf *checkVariable(LuaSL_compiler *compiler, LSL_Leaf *identifier)
{
    gameGlobals *game = compiler->game;

    if (identifier)
    {
	LSL_Leaf *var = findVariable(compiler, identifier->value.stringValue);

	if (var)
	{
	    if (LUASL_DEBUG)
		PI("Found %s!", identifier->value.stringValue);
	    identifier->value.identifierValue = var->value.identifierValue;
	    identifier->basicType = var->basicType;
	}
	else 
	    PE("NOT found %s @ line %d, column %d!", identifier->value.stringValue, identifier->line, identifier->column);
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
	    }
	    else
		lType = left->basicType;
	    if (OT_undeclared == lType)
	    {
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
	    }
	    else
		rType = right->basicType;
	    if (OT_undeclared == rType)
	    {
		PW("Undeclared identifier issue, deferring this until the second pass. @ line %d, column %d.", lval->line, lval->column);
		lval->basicType = OT_undeclared;
		return lval;
	    }
	    if (OT_vector < rType)
		rType = allowed[rType].result;
	}

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

	    PE("Invalid operation [%s(%s) %s %s(%s)] @ line %d, column %d!", leftType, leftToken, lval->toKen->toKen, rightType, rightToken, lval->line, lval->column);
	}
    }

    return lval;
}

LSL_Leaf *addBlock(LuaSL_compiler *compiler, LSL_Leaf *left, LSL_Leaf *lval, LSL_Leaf *right)
{
    // Damn, look ahead.  The } symbol is getting read (and thus endBlock called) before the last statement in the block is reduced (which actually calls the add*() functions).
    compiler->currentBlock = compiler->currentBlock->outerBlock;
    return lval;
}

LSL_Leaf *addCrement(LuaSL_compiler *compiler, LSL_Leaf *variable, LSL_Leaf *crement)
{
    if ((variable) && (crement))
    {
	crement->basicType = variable->basicType;
    }

    return crement;
}

LSL_Leaf *addParameter(LuaSL_compiler *compiler, LSL_Leaf *type, LSL_Leaf *identifier)
{
    LSL_Identifier *result = calloc(1, sizeof(LSL_Identifier));

    if ( (identifier) && (result))
    {
	result->name.text = identifier->value.stringValue;
	result->name.ignorableText = identifier->ignorableText;
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
		func->name.ignorableText = identifier->ignorableText;
		identifier->toKen = tokens[LSL_FUNCTION - lowestToken];
		identifier->value.functionValue = func;
		func->type = type;
		if (type)
		    identifier->basicType = type->basicType;
		else
		    identifier->basicType = OT_nothing;
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
	function->value.functionValue->block = block;
	statement = addStatement(compiler, NULL, LSL_FUNCTION, NULL, function, NULL, NULL, NULL);
    }

    return statement;
}

LSL_Leaf *addFunctionCall(LuaSL_compiler *compiler, LSL_Leaf *identifier, LSL_Leaf *open, LSL_Leaf *params, LSL_Leaf *close)
{
    LSL_Leaf *func = findFunction(compiler, identifier->value.stringValue);
    LSL_FunctionCall *call = calloc(1, sizeof(LSL_FunctionCall));

    identifier->toKen = tokens[LSL_UNKNOWN - lowestToken];

    if (func)
    {
	if (call)
	{
	    call->function = func->value.functionValue;
	    eina_clist_element_init(&(call->dangler));
	    call->call = identifier;
	}
	identifier->value.functionCallValue = call;
	// TODO - Put the params in call.
//	eina_inarray_setup(&(cal->vars), sizeof(LSL_Text), 3);
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
	identifier->basicType = OT_undeclared;
	compiler->undeclared = TRUE;
    }

    return identifier;
}

LSL_Leaf *addParenthesis(LSL_Leaf *lval, LSL_Leaf *expr, LSL_Type type, LSL_Leaf *rval)
{
    LSL_Parenthesis *parens = calloc(1, sizeof(LSL_Parenthesis));

    if (parens)
    {
	parens->contents = expr;
	parens->type = type;
#if LUASL_DIFF_CHECK
	parens->rightIgnorableText = rval->ignorableText;
	// Actualy, at this point, rval is no longer needed.
//	rval->ignorableText = eina_strbuf_new();
#endif
	if (lval)
	{
	    lval->value.parenthesis = parens;
	    if (expr)
		lval->basicType = expr->basicType;
	}
    }
    return lval;
}

LSL_Leaf *addState(LuaSL_compiler *compiler, LSL_Leaf *state, LSL_Leaf *identifier, LSL_Leaf *block)
{
    LSL_State *result = calloc(1, sizeof(LSL_State));

    if ((identifier) && (result))
    {
	result->name.text = identifier->value.stringValue;
	result->name.ignorableText = identifier->ignorableText;
	result->block = block;
	if (state)
	{
	    result->state.text = state->toKen->toKen;
	    result->state.ignorableText = state->ignorableText;
	}
	identifier->value.stateValue = result;
	identifier->toKen = tokens[LSL_STATE - lowestToken];
	eina_hash_add(compiler->script.states, result->name.text, identifier);
    }

    return identifier;
}

LSL_Leaf *addStatement(LuaSL_compiler *compiler, LSL_Leaf *lval, LSL_Type type, LSL_Leaf *left, LSL_Leaf *expr, LSL_Leaf *right, LSL_Leaf *block, LSL_Leaf *identifier)
{
    gameGlobals *game = compiler->game;
    LSL_Statement *stat = calloc(1, sizeof(LSL_Statement));

    if (NULL == lval)
	lval = newLeaf(LSL_STATEMENT, NULL, NULL);

    if (stat)
    {
	stat->type = type;
	stat->expressions = expr;
	stat->block = block;
	eina_clist_element_init(&(stat->statement));
	if (identifier)
	    stat->identifier = identifier->value.identifierValue;
	if (left)
	{
	    LSL_Leaf *parens = addParenthesis(left, expr, LSL_EXPRESSION, right);

	    if (parens)
		stat->parenthesis = parens->value.parenthesis;
	}

	switch (type)
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
		break;
	    }
	    case LSL_IF :
	    {
		break;
	    }
	    case LSL_ELSE :
	    {
		break;
	    }
	    case LSL_JUMP :
	    {
		break;
	    }
	    case LSL_RETURN :
	    {
		break;
	    }
	    case LSL_STATE_CHANGE :
	    {
		break;
	    }
	    case LSL_WHILE :
	    {
		stat->identifier = NULL;
		// TODO - need to stash the while's white space somewhere.
		break;
	    }
	    case LSL_IDENTIFIER :
	    {
		break;
	    }
	    default :
	    {
		PE("Should not be here %d.", type);
		break;
	    }
	}

	if (lval)
	    lval->value.statementValue = stat;
    }


    return lval;
}

LSL_Leaf *collectStatements(LuaSL_compiler *compiler, LSL_Leaf *list, LSL_Leaf *statement)
{
    boolean wasNull = FALSE;
    if (NULL == list)
    {
	list = newLeaf(LSL_BLOCK_OPEN, NULL, NULL);
	if (list)
	{
	    wasNull = TRUE;
	}
    }

    if (list)
    {
	if (statement)
	{
	    if (!wasNull)
	    list->value.blockValue = compiler->currentBlock;	// Maybe NULL.
	    if (list->value.blockValue)
	    {
		eina_clist_add_tail(&(list->value.blockValue->statements), &(statement->value.statementValue->statement));
	    }
	}
    }

    return list;
}

LSL_Leaf *addTypecast(LSL_Leaf *lval, LSL_Leaf *type, LSL_Leaf *rval, LSL_Leaf *expr)
{
    addParenthesis(lval, expr, LSL_TYPECAST_OPEN, rval);
    if (lval)
    {
	if (type)
	    lval->basicType = type->basicType;
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
	result->name.ignorableText = identifier->ignorableText;
	result->value.toKen = tokens[LSL_UNKNOWN - lowestToken];
	identifier->value.identifierValue = result;
	identifier->left = type;
	identifier->right = assignment;
	if (assignment)
	    assignment->right = expr;
	if (type)
	{
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
	blok->outerBlock = compiler->currentBlock;
	compiler->currentBlock = blok;
	blok->function = compiler->currentFunction;
	compiler->currentFunction = NULL;
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

static LSL_Leaf *evaluateLeaf(LSL_Leaf *leaf, LSL_Leaf *left, LSL_Leaf *right)
{
    LSL_Leaf *result = NULL;

    if (leaf)
    {
	LSL_Leaf *lresult = NULL;
	LSL_Leaf *rresult = NULL;

	if (LSL_RIGHT2LEFT & leaf->toKen->flags)
	{
	    rresult = evaluateLeaf(leaf->right, left, right);
	    if (!(LSL_UNARY & leaf->toKen->flags))
		lresult = evaluateLeaf(leaf->left, left, right);
	}
	else // Assume left to right.
	{
	    lresult = evaluateLeaf(leaf->left, left, right);
	    if (!(LSL_UNARY & leaf->toKen->flags))
		rresult = evaluateLeaf(leaf->right, left, right);
	}

	if (leaf->toKen->evaluate)
	    result = leaf->toKen->evaluate(leaf, lresult, rresult);
	else
	{
	    result = newLeaf(LSL_UNKNOWN, NULL, NULL);
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
    LSL_Leaf *result = newLeaf(LSL_FLOAT, NULL, NULL);

    if (content && result)
    {
	if (LUASL_DEBUG)
	    printf(" <%g> ", content->value.floatValue);
	memcpy(result, content, sizeof(LSL_Leaf));
	result->basicType = OT_float;
    }
    return result;
}

static LSL_Leaf *evaluateIntegerToken(LSL_Leaf *content, LSL_Leaf *left, LSL_Leaf *right)
{
    LSL_Leaf *result = newLeaf(LSL_INTEGER, NULL, NULL);

    if (content && result)
    {
	if (LUASL_DEBUG)
	    printf(" <%d> ", content->value.integerValue);
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
On the other hand, in Lua, only false and nil are false, everything else is true.
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
    LSL_Leaf *result = newLeaf(LSL_UNKNOWN, NULL, NULL);

    if (content && result)
    {
	if (LUASL_DEBUG)
	    printf(" [%s] ", content->toKen->toKen);

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
		switch (result->toKen->type)
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
		if (LUASL_DEBUG)
		    printf(" (=%g) ", result->value.floatValue);
		break;
	    }

	    case OT_integer :
	    {
		switch (result->toKen->type)
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
		if (LUASL_DEBUG)
		    printf(" (=%d) ", result->value.integerValue);
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
	switch (content->value.statementValue->type)
	{
	    case LSL_EXPRESSION :
	    {
		result = evaluateLeaf(content->value.statementValue->expressions, left, right);
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
		break;
	    }
	    case LSL_IF :
	    {
		break;
	    }
	    case LSL_ELSE :
	    {
		    break;
	    }
	    case LSL_JUMP :
	    {
		break;
	    }
	    case LSL_RETURN :
	    {
		break;
	    }
	    case LSL_STATE_CHANGE :
	    {
		break;
	    }
	    case LSL_WHILE :
	    {
		break;
	    }
	    case LSL_IDENTIFIER :
	    {
		break;
	    }
	    default :
	    {
//		PE("Should not be here %d.", type);
		break;
	    }
	}

	if (result)
	{
	    switch (result->basicType)
	    {
		case OT_float   :  printf("Result is the float %g.\n", result->value.floatValue);  break;
		case OT_integer :  printf("Result is the integer %d.\n", result->value.integerValue);  break;
		default         :  /*printf("\nResult of an unknown type [%d] %d!\n", result->basicType, result->value.integerValue);*/  break;
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

static void outputText(FILE *file, LSL_Text *text, boolean ignore)
{
	    if (text->text)
	    {
#if LUASL_DIFF_CHECK
		if (ignore && (text->ignorableText))
		    fwrite(eina_strbuf_string_get(text->ignorableText), 1, eina_strbuf_length_get(text->ignorableText), file);
#endif
		fprintf(file, "%s", text->text);
	    }
}

static void outputLeaf(FILE *file, outputMode mode, LSL_Leaf *leaf)
{
    if (leaf)
    {
	outputLeaf(file, mode, leaf->left);
#if LUASL_DIFF_CHECK
	if ((!(LSL_NOIGNORE & leaf->toKen->flags)) && (leaf->ignorableText))
	    fwrite(eina_strbuf_string_get(leaf->ignorableText), 1, eina_strbuf_length_get(leaf->ignorableText), file);
#endif
	if (leaf->toKen->output)
	    leaf->toKen->output(file, mode, leaf);
	else
	    fprintf(file, "%s", leaf->toKen->toKen);
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
	LSL_Leaf *param = NULL;
	int first = TRUE;

	outputLeaf(file, mode, func->type);
	outputText(file, &(func->name), !(LSL_NOIGNORE & content->toKen->flags));
//	fprintf(file, "%s(", func->name);
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
	outputLeaf(file, mode, func->block);
	if (!LUASL_DIFF_CHECK)
	    fprintf(file, "\n");
    }
}

static void outputFunctionCallToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
    {
	LSL_FunctionCall *call = content->value.functionCallValue;
	LSL_Function *func = call->function;
	outputText(file, &(func->name), !(LSL_NOIGNORE & content->toKen->flags));
//	fprintf(file, "%s(", func->name);
	fprintf(file, "(");
	// TODO - print params here.
	fprintf(file, ")");
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
	outputText(file, &(content->value.identifierValue->name), !(LSL_NOIGNORE & content->toKen->flags));
}

static void outputParameterListToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
	outputLeaf(file, mode, content->value.listValue);
}

static void outputRawParenthesisToken(FILE *file, outputMode mode, LSL_Parenthesis *parenthesis, const char *typeName)
{
	fprintf(file, "(");
// TODO, if it's a parameter list, deal with it via the leaf -> function -> vars.
	if (LSL_TYPECAST_OPEN == parenthesis->type)
	    fprintf(file, "%s", typeName);	// TODO - We are missing the type ignorable text here.
	else
	    outputLeaf(file, mode, parenthesis->contents);
#if LUASL_DIFF_CHECK
	fprintf(file, "%s)", eina_strbuf_string_get(parenthesis->rightIgnorableText));
#else
	fprintf(file, ")");
#endif
	if (LSL_TYPECAST_OPEN == parenthesis->type)
	    outputLeaf(file, mode, parenthesis->contents);
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
	    outputText(file, &(state->state), !(LSL_NOIGNORE & content->toKen->flags));
	    outputText(file, &(state->name), !(LSL_NOIGNORE & content->toKen->flags));
	    outputLeaf(file, mode, state->block);
	    fprintf(file, "\n");
	}
    }
}

// Circular references, so declare this one first.
static void outputBlockToken(FILE *file, outputMode mode, LSL_Leaf *content);

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
		isBlock = TRUE;
		fprintf(file, "%s", tokens[statement->type - lowestToken]->toKen);
		break;
	    }
	    case LSL_IF :
	    {
		isBlock = TRUE;
		fprintf(file, "%s", tokens[statement->type - lowestToken]->toKen);
		break;
	    }
	    case LSL_ELSE :
	    {
		fprintf(file, "%s", tokens[statement->type - lowestToken]->toKen);
		break;
	    }
	    case LSL_JUMP :
	    {
		fprintf(file, "%s", tokens[statement->type - lowestToken]->toKen);
		break;
	    }
	    case LSL_RETURN :
	    {
		fprintf(file, "%s", tokens[statement->type - lowestToken]->toKen);
		break;
	    }
	    case LSL_STATE_CHANGE :
	    {
		fprintf(file, "%s", tokens[statement->type - lowestToken]->toKen);
		break;
	    }
	    case LSL_WHILE :
	    {
		isBlock = TRUE;
		fprintf(file, "%s", tokens[statement->type - lowestToken]->toKen);
		break;
	    }
	    case LSL_IDENTIFIER :
	    {
		break;
	    }
	    default :
	    {
		fprintf(file, "@@Should not be here %s.@@", tokens[statement->type - lowestToken]->toKen);
		break;
	    }
	}

	if (statement->parenthesis)
	    outputRawParenthesisToken(file, mode, statement->parenthesis, "");
	else
	    outputLeaf(file, mode, statement->expressions);

	if (statement->block)
	    outputBlockToken(file, mode, statement->block);

	if (!isBlock)
	{
	    fprintf(file, ";");
	    if (!LUASL_DIFF_CHECK)
		fprintf(file, "\n");
	}
    }
}

static void outputStatementToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
    {
	outputRawStatement(file, mode, content->value.statementValue);
#if LUASL_DIFF_CHECK
	if (content->ignorableText)
	    fwrite(eina_strbuf_string_get(content->ignorableText), 1, eina_strbuf_length_get(content->ignorableText), file);
#endif
    }
}

static void outputBlockToken(FILE *file, outputMode mode, LSL_Leaf *content)
{
    if (content)
    {
	if (LUASL_DIFF_CHECK)
	    fprintf(file, "\n{");
	else
	    fprintf(file, "\n{\n");
	if (content->value.blockValue)
	{
	    LSL_Statement *stat = NULL;

	    EINA_CLIST_FOR_EACH_ENTRY(stat, &(content->value.blockValue->statements), LSL_Statement, statement)
	    {
		outputRawStatement(file, mode, stat);
	    }
	}
	if (LUASL_DIFF_CHECK)
	    fprintf(file, "\n}");
	else
	    fprintf(file, "}");
    }
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

//	outputLeaf(stdout, OM_LSL, compiler->ast);
//	printf("\n");
	evaluateLeaf(compiler->ast, NULL, NULL);
//	printf("\n");

	if (LUASL_DIFF_CHECK)
	{
	    strcpy(outName, compiler->fileName);
	    strcat(outName, "2");
	    out = fopen(outName, "w");
	    if (out)
	    {
		char diffName[PATH_MAX];
//		int count;

		strcpy(diffName, compiler->fileName);
		strcat(diffName, ".diff");
		outputLeaf(out, OM_LSL, compiler->ast);
		fclose(out);
		sprintf(buffer, "diff \"%s\" \"%s\" > \"%s\"", compiler->fileName, outName, diffName);
//		count = system(buffer);
//		if (0 != count)
//		    PE("LSL output file is different - %s!", outName);
//		else
//		    result = TRUE;
	    }
	    else
		PC("Unable to open file %s for writing!", outName);
	}
	strcpy(luaName, compiler->fileName);
	strcat(luaName, ".lua");
	out = fopen(luaName, "w");
	if (out)
	{
	    outputLeaf(out, OM_LUA, compiler->ast);
	    fclose(out);
	}
	else
	    PC("Unable to open file %s for writing!", luaName);
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

// Parse the  LSL script, validating it and reporting errors.
//   Just pass all constants and function names through to Lua, assume they are globals there.

    memset(&compiler, 0, sizeof(LuaSL_compiler));
    compiler.game = game;
    compiler.script.functions = eina_hash_stringshared_new(burnLeaf);
    compiler.script.states = eina_hash_stringshared_new(burnLeaf);
    compiler.script.variables = eina_hash_stringshared_new(burnLeaf);
    eina_clist_init(&(compiler.danglingCalls));
#if LUASL_DIFF_CHECK
    compiler.ignorableText = eina_strbuf_new();
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
    // Text editors usually start counting at 1, even programmers editors.
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
    {
	result = doneParsing(&compiler);

// Take the result of the parse, and convert it into Lua source.
//   Each LSL script becomes a Lua state.
//   LSL states are handled as Lua tables, with each LSL state function being a table function in a common metatable.
//   LL and OS functions are likely to be C functions. 

// Compile the Lua source by the Lua compiler.

    }

    if (NULL != compiler.file)
    {
	fclose(compiler.file);
	compiler.file = NULL;
    }

    if (!doConstants)
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
	param->lval = newLeaf(LSL_UNKNOWN, NULL, NULL);
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
    for (i = 0; LSL_Tokens[i].toKen != NULL; i++)
    {
	if (lowestToken > LSL_Tokens[i].type)
	    lowestToken = LSL_Tokens[i].type;
    }
    tokens = calloc(i + 1, sizeof(LSL_Token *));
    if (tokens)
    {
	LuaSL_yyparseParam param;

	// Sort the token table.
	for (i = 0; LSL_Tokens[i].toKen != NULL; i++)
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
	    if (LUASL_DEBUG)
	    {
		yyset_debug(1, param.scanner);
		ParseTrace(stdout, "LSL_lemon ");
	    }
	    yyset_in(param.file, param.scanner);
	    // on EOF yylex will return 0
	    while((yv = yylex(param.lval, param.scanner)) != 0)
	    {
		Parse(pParser, yv, param.lval, &param);
		if (LSL_SCRIPT == yv)
		    break;
		param.lval = newLeaf(LSL_UNKNOWN, NULL, NULL);
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

