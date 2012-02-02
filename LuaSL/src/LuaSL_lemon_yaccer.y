%include {
#include "LuaSL.h"
}

%extra_argument {LuaSL_compiler *compiler}

%stack_size 1024

%token_type {LSL_Leaf *}
%default_type  {LSL_Leaf *}
%token_destructor {burnLeaf($$);}
%default_destructor {burnLeaf($$);}

// The start symbol, just coz we need one.

// Lemon does not like the start symbol to be on the RHS, so give it a dummy start symbol.
program ::= script LSL_SCRIPT(A).						{ if (NULL != A) A->left = compiler->ast;  compiler->ast = A; }

// Various forms of "space".  The lexer takes care of them for us.

%nonassoc LSL_SPACE LSL_COMMENT LSL_COMMENT_LINE LSL_UNKNOWN.

// Basic script structure.

%nonassoc LSL_SCRIPT.
script ::= script state(A).							{ if (NULL != A) A->left = compiler->ast;  compiler->ast = A; }
script ::= script functionBody(A).						{ if (NULL != A) A->left = compiler->ast;  compiler->ast = A; }
script ::= script statement(A).							{ if (NULL != A) A->left = compiler->ast;  compiler->ast = A; }
script ::= .

// State definitions.

%nonassoc LSL_BLOCK_OPEN LSL_BLOCK_CLOSE LSL_STATE.
stateBlock(A) ::= beginBlock(L) functionList(B) LSL_BLOCK_CLOSE(R).		{ A = addBlock(compiler, L, B, R); }
state(A) ::= LSL_DEFAULT(I) stateBlock(B).					{ A = addState(compiler, NULL, I, B); }
state(A) ::= LSL_STATE_CHANGE(S) LSL_IDENTIFIER(I) stateBlock(B).		{ A = addState(compiler, S, I, B); }

// Function definitions.

%nonassoc LSL_PARAMETER LSL_PARAMETER_LIST LSL_FUNCTION.
functionList(A) ::= functionList(B) functionBody(C).				{ A = collectStatements(compiler, B, C); }
functionList(A) ::= functionBody(C).						{ A = collectStatements(compiler, NULL, C); }
// No such thing as a function list with no functions.
//functionList(A) ::= .								{ A = collectStatements(compiler, NULL, NULL); }

functionBody(A) ::= function(F) block(B).					{ A = addFunctionBody(compiler, F, B); }	// addFunctionBody returns an implied addStatement(compiler, NULL, F, NULL, F, NULL, NULL);

parameterList(A) ::= parameterList(B) LSL_COMMA(C) parameter(D).		{ A = collectParameters(compiler, B, C, D); }
parameterList(A) ::= parameter(D).						{ A = collectParameters(compiler, NULL, NULL, D); }
parameterList(A) ::= .								{ A = collectParameters(compiler, NULL, NULL, NULL); }
parameter(A) ::= type(B) LSL_IDENTIFIER(C).					{ A = addParameter(compiler, B, C); }
function(A) ::= LSL_IDENTIFIER(C) LSL_PARENTHESIS_OPEN(D) parameterList(E) LSL_PARENTHESIS_CLOSE(F).		{ A = addFunction(compiler, NULL, C, D, E, F); }
function(A) ::= type(B) LSL_IDENTIFIER(C) LSL_PARENTHESIS_OPEN(D) parameterList(E) LSL_PARENTHESIS_CLOSE(F).	{ A = addFunction(compiler, B, C, D, E, F); }

// Blocks.

block(A) ::= beginBlock(L) statementList(B) LSL_BLOCK_CLOSE(R).			{ A = addBlock(compiler, L, B, R); }

// Various forms of statement.

%nonassoc LSL_STATEMENT.
statementList(A) ::= statementList(B) statement(C).				{ A = collectStatements(compiler, B, C); }
// This causes infinite loops (and about 150 conflicts) with - if () single statement;
//statementList(A) ::= statement(C).						{ A = collectStatements(compiler, NULL, C); }
// Yes, you can have an empty block.
statementList(A) ::= .								{ A = collectStatements(compiler, NULL, NULL); }

%nonassoc LSL_DO LSL_FOR LSL_IF LSL_JUMP LSL_RETURN LSL_STATE_CHANGE LSL_WHILE.
%nonassoc LSL_ELSE.
statement(A) ::= LSL_DO(F) block(B) LSL_WHILE(W) LSL_PARENTHESIS_OPEN(L) expr(E) LSL_PARENTHESIS_CLOSE(R) LSL_STATEMENT(S).				{ A = addStatement(compiler, S,    F, L,    E,    R, B,    W); }
statement(A) ::= LSL_FOR(F) LSL_PARENTHESIS_OPEN(L) expr(E0) LSL_STATEMENT(S0) expr(E1) LSL_STATEMENT(S1) expr(E2) LSL_PARENTHESIS_CLOSE(R) block(B).		{ A = addFor(compiler, NULL, F, L, E0, S0, E1, S1, E2, R, B); }
statement(A) ::= LSL_FOR(F) LSL_PARENTHESIS_OPEN(L) expr(E0) LSL_STATEMENT(S0) expr(E1) LSL_STATEMENT(S1) expr(E2) LSL_PARENTHESIS_CLOSE(R) statement(S).	{ A = addFor(compiler, NULL, F, L, E0, S0, E1, S1, E2, R, S); }

statement(A) ::= ifBlock(B).																{ A = B; }
ifBlock(A) ::= ifBlock(B) elseBlock(E).															{ A = addIfElse(compiler, B, E); }
ifBlock(A) ::= LSL_IF(F) LSL_PARENTHESIS_OPEN(L) expr(E) LSL_PARENTHESIS_CLOSE(R) block(B).	[LSL_ELSE]						{ A = addStatement(compiler, NULL, F, L,    E,    R,    B,    NULL); }
ifBlock(A) ::= LSL_IF(F) LSL_PARENTHESIS_OPEN(L) expr(E) LSL_PARENTHESIS_CLOSE(R) statement(S).	[LSL_ELSE]						{ A = addStatement(compiler, NULL, F, L,    E,    R,    S,    NULL); }
elseBlock(A) ::= LSL_ELSE(F) block(B).															{ A = addStatement(compiler, NULL, F, NULL, NULL, NULL, B,    NULL); }
elseBlock(A) ::= LSL_ELSE(F) statement(S).														{ A = addStatement(compiler, NULL, F, NULL, NULL, NULL, S,    NULL); }

statement(A) ::= LSL_JUMP(F) LSL_IDENTIFIER(I) LSL_STATEMENT(S).											{ A = addStatement(compiler, S,    F, NULL, NULL, NULL, NULL, I); }
statement(A) ::= LSL_RETURN(F) expr(E) LSL_STATEMENT(S).												{ A = addStatement(compiler, S,    F, NULL, E,    NULL, NULL, NULL); }
statement(A) ::= LSL_RETURN(F) LSL_STATEMENT(S).													{ A = addStatement(compiler, S,    F, NULL, NULL, NULL, NULL, NULL); }
statement(A) ::= LSL_STATE_CHANGE(F) LSL_DEFAULT(I) LSL_STATEMENT(S).											{ A = addStatement(compiler, S,    F, NULL, NULL, NULL, NULL, I); }
statement(A) ::= LSL_STATE_CHANGE(F) LSL_IDENTIFIER(I) LSL_STATEMENT(S).										{ A = addStatement(compiler, S,    F, NULL, NULL, NULL, NULL, I); }
statement(A) ::= LSL_WHILE(F) LSL_PARENTHESIS_OPEN(L) expr(E) LSL_PARENTHESIS_CLOSE(R) block(B).							{ A = addStatement(compiler, NULL, F, L,    E,    R,    B,    NULL); }
statement(A) ::= LSL_WHILE(F) LSL_PARENTHESIS_OPEN(L) expr(E) LSL_PARENTHESIS_CLOSE(R) statement(S).							{ A = addStatement(compiler, NULL, F, L,    E,    R,    S,    NULL); }

%nonassoc LSL_LABEL.
statement(A) ::= LSL_LABEL(F) LSL_IDENTIFIER(I) LSL_STATEMENT(S).											{ A = addStatement(compiler, S,    F, NULL, NULL, NULL, NULL, I); }

beginBlock(A) ::= LSL_BLOCK_OPEN(B).															{ A = beginBlock(compiler, B); }

// This might be bogus, or might be valid LSL, but it lets us test the expression parser by evaluating them.
statement(A) ::= expr(E) LSL_STATEMENT(S).														{ A = addStatement(compiler, S,    S, NULL, E,    NULL, NULL, NULL); }

// Various forms of expression.

// Used for function call params, and list contents.
exprList(A) ::= exprList(B) LSL_COMMA(C) expr(D).				{ A = collectArguments(compiler, B, C, D); }
exprList(A) ::= expr(D).							{ A = collectArguments(compiler, NULL, NULL, D); }
exprList(A) ::= .								{ A = collectArguments(compiler, NULL, NULL, NULL); }

%right  LSL_BOOL_AND.
expr(A) ::= expr(B) LSL_BOOL_AND(C)		expr(D).			{ A = addOperation(compiler, B, C, D); }
%right  LSL_BOOL_OR.
expr(A) ::= expr(B) LSL_BOOL_OR(C)		expr(D).			{ A = addOperation(compiler, B, C, D); }

%left  LSL_BIT_AND LSL_BIT_XOR LSL_BIT_OR.
expr(A) ::= expr(B) LSL_BIT_OR(C)		expr(D).			{ A = addOperation(compiler, B, C, D); }
expr(A) ::= expr(B) LSL_BIT_XOR(C)		expr(D).			{ A = addOperation(compiler, B, C, D); }
expr(A) ::= expr(B) LSL_BIT_AND(C)		expr(D).			{ A = addOperation(compiler, B, C, D); }

%right  LSL_EQUAL LSL_NOT_EQUAL.
expr(A) ::= expr(B) LSL_NOT_EQUAL(C)		expr(D).			{ A = addOperation(compiler, B, C, D); }
expr(A) ::= expr(B) LSL_EQUAL(C)		expr(D).			{ A = addOperation(compiler, B, C, D); }
%right  LSL_LESS_THAN LSL_GREATER_THAN LSL_LESS_EQUAL LSL_GREATER_EQUAL.
expr(A) ::= expr(B) LSL_GREATER_EQUAL(C)	expr(D).			{ A = addOperation(compiler, B, C, D); }
expr(A) ::= expr(B) LSL_LESS_EQUAL(C)		expr(D).			{ A = addOperation(compiler, B, C, D); }
expr(A) ::= expr(B) LSL_GREATER_THAN(C)		expr(D).			{ A = addOperation(compiler, B, C, D); }
expr(A) ::= expr(B) LSL_LESS_THAN(C)		expr(D).			{ A = addOperation(compiler, B, C, D); }

%left  LSL_LEFT_SHIFT LSL_RIGHT_SHIFT.
expr(A) ::= expr(B) LSL_RIGHT_SHIFT(C)		expr(D).			{ A = addOperation(compiler, B, C, D); }
expr(A) ::= expr(B) LSL_LEFT_SHIFT(C)		expr(D).			{ A = addOperation(compiler, B, C, D); }

%left  LSL_SUBTRACT LSL_ADD LSL_CONCATENATE.
expr(A) ::= expr(B) LSL_ADD(C)			expr(D).			{ A = addOperation(compiler, B, C, D); }
expr(A) ::= expr(B) LSL_SUBTRACT(C)		expr(D).			{ A = addOperation(compiler, B, C, D); }
%left  LSL_DIVIDE LSL_MODULO LSL_MULTIPLY LSL_DOT_PRODUCT LSL_CROSS_PRODUCT.
expr(A) ::= expr(B) LSL_MULTIPLY(C)		expr(D).			{ A = addOperation(compiler, B, C, D); }
expr(A) ::= expr(B) LSL_MODULO(C)		expr(D).			{ A = addOperation(compiler, B, C, D); }
expr(A) ::= expr(B) LSL_DIVIDE(C)		expr(D).			{ A = addOperation(compiler, B, C, D); }

%right LSL_BIT_NOT LSL_BOOL_NOT LSL_NEGATION.
expr(A) ::= LSL_BIT_NOT(B)			expr(C).			{ A = addOperation(compiler, NULL, B, C); }
expr(A) ::= LSL_BOOL_NOT(B)			expr(C).			{ A = addOperation(compiler, NULL, B, C); }
expr(A) ::= LSL_SUBTRACT(B)			expr(C).	[LSL_NEGATION]	{ A = addOperation(compiler, NULL, B, C); }

// Types, typecasts, and expression reordering.

%right  LSL_TYPECAST_OPEN LSL_TYPECAST_CLOSE.
%nonassoc LSL_TYPE_FLOAT LSL_TYPE_INTEGER LSL_TYPE_KEY LSL_TYPE_LIST LSL_TYPE_ROTATION LSL_TYPE_STRING LSL_TYPE_VECTOR.
type(A) ::= LSL_TYPE_FLOAT(B).							{ B->basicType = OT_float;	A = B; }
type(A) ::= LSL_TYPE_INTEGER(B).						{ B->basicType = OT_integer;	A = B; }
type(A) ::= LSL_TYPE_KEY(B).							{ B->basicType = OT_key;	A = B; }
type(A) ::= LSL_TYPE_LIST(B).							{ B->basicType = OT_list;	A = B; }
type(A) ::= LSL_TYPE_ROTATION(B).						{ B->basicType = OT_rotation;	A = B; }
type(A) ::= LSL_TYPE_STRING(B).							{ B->basicType = OT_string;	A = B; }
type(A) ::= LSL_TYPE_VECTOR(B).							{ B->basicType = OT_vector;	A = B; }

%left  LSL_ANGLE_OPEN LSL_ANGLE_CLOSE.
%nonassoc LSL_BRACKET_OPEN LSL_BRACKET_CLOSE.
%nonassoc LSL_PARENTHESIS_OPEN LSL_PARENTHESIS_CLOSE LSL_EXPRESSION.

expr(A) ::= LSL_PARENTHESIS_OPEN(B)	expr(C) LSL_PARENTHESIS_CLOSE(D).		{ A = addParenthesis(B, C, LSL_EXPRESSION, D); }
expr(A) ::= LSL_PARENTHESIS_OPEN(B)	type(C)	LSL_PARENTHESIS_CLOSE(D) expr(E).	{ A = addTypecast(B, C, D, E); }

// Function call.

// Causes a conflict when exprList is empty with a function definition with no type and no parameters.
expr(A) ::= LSL_IDENTIFIER(B) LSL_PARENTHESIS_OPEN(C) exprList(D) LSL_PARENTHESIS_CLOSE(E).	{ A = addFunctionCall(compiler, B, C, D, E); }

// Variables and dealing with them.

expr(A) ::= identifier(B).							{ A = B; }

%right LSL_ASSIGNMENT_CONCATENATE LSL_ASSIGNMENT_ADD LSL_ASSIGNMENT_SUBTRACT LSL_ASSIGNMENT_MULTIPLY LSL_ASSIGNMENT_MODULO LSL_ASSIGNMENT_DIVIDE LSL_ASSIGNMENT_PLAIN.
// Yes, these can be expressions, and can happen in if statements and such.
expr(A) ::= identifier(B) LSL_ASSIGNMENT_CONCATENATE(C)	expr(D).		{ A = addOperation(compiler, B, C, D); }
expr(A) ::= identifier(B) LSL_ASSIGNMENT_ADD(C)		expr(D).		{ A = addOperation(compiler, B, C, D); }
expr(A) ::= identifier(B) LSL_ASSIGNMENT_SUBTRACT(C)	expr(D)	.		{ A = addOperation(compiler, B, C, D); }
expr(A) ::= identifier(B) LSL_ASSIGNMENT_MULTIPLY(C)	expr(D).		{ A = addOperation(compiler, B, C, D); }
expr(A) ::= identifier(B) LSL_ASSIGNMENT_MODULO(C)	expr(D).		{ A = addOperation(compiler, B, C, D); }
expr(A) ::= identifier(B) LSL_ASSIGNMENT_DIVIDE(C)	expr(D).		{ A = addOperation(compiler, B, C, D); }
expr(A) ::= identifier(B) LSL_ASSIGNMENT_PLAIN(C)	expr(D).		{ A = addOperation(compiler, B, C, D); }

// Hmm think this can have commas seperating the assignment parts, or is that only in C?.  If so, best to separate them when converting to Lua, as it uses that syntax for something else.
// Well, not in OpenSim at least, nor in SL.  So we are safe.  B-)
// On the other hand, it might be legal to have comma separated bits in a for loop - for ((i = 1), (j=1); ...
statement(A) ::= type(T) LSL_IDENTIFIER(I) LSL_ASSIGNMENT_PLAIN(D) expr(E) LSL_STATEMENT(S).	{ A = addStatement(compiler, S, I, NULL, addVariable(compiler, T, I, D,    E),    NULL, NULL, I); }
statement(A) ::= type(T) LSL_IDENTIFIER(I) LSL_STATEMENT(S).					{ A = addStatement(compiler, S, I, NULL, addVariable(compiler, T, I, NULL, NULL), NULL, NULL, I); }

%right LSL_DOT LSL_IDENTIFIER LSL_FUNCTION_CALL LSL_VARIABLE.
identifier(A) ::= identifier(I) LSL_DOT(D) LSL_IDENTIFIER(B).				{ A = checkVariable(compiler, I, D,    B); }
identifier(A) ::= LSL_IDENTIFIER(B).							{ A = checkVariable(compiler, B, NULL, NULL); }

%right LSL_DECREMENT_PRE LSL_INCREMENT_PRE LSL_DECREMENT_POST LSL_INCREMENT_POST.
expr(A) ::= identifier(B) LSL_DECREMENT_PRE(C).						{ A = addCrement(compiler, B, C, LSL_DECREMENT_POST); }
expr(A) ::= identifier(B) LSL_INCREMENT_PRE(C).						{ A = addCrement(compiler, B, C, LSL_INCREMENT_POST); }
expr(A) ::= LSL_DECREMENT_PRE(C) identifier(B).						{ A = addCrement(compiler, B, C, LSL_DECREMENT_PRE); }
expr(A) ::= LSL_INCREMENT_PRE(C) identifier(B).						{ A = addCrement(compiler, B, C, LSL_INCREMENT_PRE); }

%nonassoc LSL_COMMA.

// Values.

%nonassoc  LSL_FLOAT.
expr(A) ::= LSL_FLOAT(B).									{ A = addNumby(B); }
%nonassoc LSL_INTEGER.
expr(A) ::= LSL_INTEGER(B).									{ A = addNumby(B); }
%nonassoc  LSL_KEY.
expr(A) ::= LSL_KEY(B).										{ B->basicType = OT_key; A = B; }
%nonassoc  LSL_LIST.
expr(A) ::= LSL_BRACKET_OPEN(L) exprList(E) LSL_BRACKET_CLOSE(R).	[LSL_BRACKET_OPEN]	{ A = addList(L, E, R); }
%nonassoc  LSL_ROTATION LSL_VECTOR.
// Uses the same symbol for less than, greater than, and the rotation / vector delimiters.
expr(A) ::= LSL_LESS_THAN(L) exprList(E) LSL_GREATER_THAN(R).		[LSL_ANGLE_OPEN]	{ A = addRotVec(L, E, R); }
%nonassoc  LSL_STRING.
expr(A) ::= LSL_STRING(B).									{ B->basicType = OT_string; A = B; }


// Parser callbacks.

%parse_accept
{
//    gameGlobals *game = compiler->game;

//    PI("Parsing complete.");
}

%parse_failure
{
    gameGlobals *game = compiler->game;

    PE("Giving up.  Parser is hopelessly lost!");
}

%stack_overflow
{
    gameGlobals *game = compiler->game;

    PE("Giving up.  Parser stack overflow @ line %d, column %d!", yypMinor->yy0->line, yypMinor->yy0->column);  // Gotta love consistancy, if it ever happens.
}

%syntax_error
{
    gameGlobals *game = compiler->game;

    PE("Syntax error @ line %d, column %d!", yyminor.yy0->line, yyminor.yy0->column);
}


/* Undocumented shit that might be useful later.  Pffft

** The next table maps tokens into fallback tokens.  If a construct
** like the following:
**.
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.

%wildcard
%code

%ifdef
%endif
%ifndef
%endif

*/

