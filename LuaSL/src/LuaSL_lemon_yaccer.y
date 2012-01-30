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
state(S) ::= LSL_DEFAULT(I) stateBlock(B).					{ S = addState(compiler, I, B); }
state(S) ::= LSL_STATE_CHANGE LSL_IDENTIFIER(I) stateBlock(B).			{ S = addState(compiler, I, B); }

// Function definitions.

%nonassoc LSL_PARAMETER LSL_PARAMETER_LIST LSL_FUNCTION.
functionList(A) ::= functionList(B) functionBody(C).				{ A = collectStatements(compiler, B, C); }
functionList(A) ::= functionBody(C).						{ A = collectStatements(compiler, NULL, C); }
functionList(A) ::= .								{ A = collectStatements(compiler, NULL, NULL); }

functionBody(A) ::= function(F) funcBlock(B).					{ A = addFunctionBody(compiler, F, B); }	// addFunctionBody has an implied addStatement(compiler, NULL, LSL_FUNCTION, NULL, B, NULL, NULL);

parameterList(A) ::= parameterList(B) LSL_COMMA(C) parameter(D).		{ A = collectParameters(compiler, B, C, D); }
parameterList(A) ::= parameter(D).						{ A = collectParameters(compiler, NULL, NULL, D); }
parameterList(A) ::= .								{ A = collectParameters(compiler, NULL, NULL, NULL); }
parameter(A) ::= type(B) LSL_IDENTIFIER(C).					{ A = addParameter(compiler, B, C); }
// Causes a conflict when it's an empty parameterList with calling the same type of function.
function(A) ::= LSL_IDENTIFIER(C) LSL_PARENTHESIS_OPEN(D) parameterList(E) LSL_PARENTHESIS_CLOSE(F).		{ A = addFunction(compiler, NULL, C, D, E, F); }
function(A) ::= type(B) LSL_IDENTIFIER(C) LSL_PARENTHESIS_OPEN(D) parameterList(E) LSL_PARENTHESIS_CLOSE(F).	{ A = addFunction(compiler, B, C, D, E, F); }

// Blocks.

block(A) ::= funcBlock(B).							{ A = B; }
block(A) ::= statementList(B).							{ A = B; }
// Perhaps change this to block?  No ,this is what differentiates it from a single statement, which functions can't handle.
funcBlock(A) ::= beginBlock(L) statementList(B) LSL_BLOCK_CLOSE(R).		{ A = addBlock(compiler, L, B, R); }

// Various forms of statement.

%nonassoc LSL_STATEMENT.
statementList(A) ::= statementList(B) statement(C).				{ A = collectStatements(compiler, B, C); }
// This causes infinite loops (and about 150 conflicts) with - if () single statement;
//statementList(A) ::= statement(C).						{ A = collectStatements(compiler, NULL, C); }
statementList(A) ::= .								{ A = collectStatements(compiler, NULL, NULL); }

%nonassoc LSL_DO LSL_FOR LSL_ELSE_IF LSL_IF LSL_JUMP LSL_RETURN LSL_STATE_CHANGE LSL_WHILE.
%nonassoc LSL_ELSE.
statement(A) ::= LSL_DO(F) block(B) LSL_WHILE(W) LSL_PARENTHESIS_OPEN(L) expr(E) LSL_PARENTHESIS_CLOSE(R) LSL_STATEMENT(S).				{ A = addStatement(compiler, S,    F->toKen->type, L, E, R, B, W); }
statement(A) ::= LSL_FOR(F) LSL_PARENTHESIS_OPEN(L) expr(E0) LSL_STATEMENT(S0) expr(E1) LSL_STATEMENT(S1) expr(E2) LSL_PARENTHESIS_CLOSE(R) block(B).	{ A = addStatement(compiler, NULL, F->toKen->type, L, E1, R, B, NULL); }	// three expressions, two semi colons

statement(A) ::= ifBlock(B).								{ A = B; }
//ifBlock ::= ifBlock LSL_ELSE block.
statement ::= LSL_ELSE block.
ifBlock(A) ::= LSL_IF(F) LSL_PARENTHESIS_OPEN(L) expr(E) LSL_PARENTHESIS_CLOSE(R) block(B).	/*[LSL_ELSE]*/						{ A = addStatement(compiler, NULL, F->toKen->type, L, E, R, B, NULL); }		// optional else, optional else if
ifBlock(A) ::= LSL_IF(F) LSL_PARENTHESIS_OPEN(L) expr(E) LSL_PARENTHESIS_CLOSE(R) statement(S).	/*[LSL_ELSE]*/						{ A = addStatement(compiler, S, F->toKen->type, L, E, R, NULL, NULL); }		// optional else, optional else if

statement(A) ::= LSL_JUMP(F) LSL_IDENTIFIER(I) LSL_STATEMENT(S).											{ A = addStatement(compiler, S,    F->toKen->type, NULL, NULL, NULL, NULL, I); }
statement(A) ::= LSL_RETURN(F) expr(E) LSL_STATEMENT(S).												{ A = addStatement(compiler, S,    F->toKen->type, NULL, E, NULL, NULL, NULL); }
statement(A) ::= LSL_RETURN(F) LSL_STATEMENT(S).													{ A = addStatement(compiler, S,    F->toKen->type, NULL, NULL, NULL, NULL, NULL); }
statement(A) ::= LSL_STATE_CHANGE(F) LSL_DEFAULT(I) LSL_STATEMENT(S).											{ A = addStatement(compiler, S,    F->toKen->type, NULL, NULL, NULL, NULL, I); }
statement(A) ::= LSL_STATE_CHANGE(F) LSL_IDENTIFIER(I) LSL_STATEMENT(S).										{ A = addStatement(compiler, S,    F->toKen->type, NULL, NULL, NULL, NULL, I); }
statement(A) ::= LSL_WHILE(F) LSL_PARENTHESIS_OPEN(L) expr(E) LSL_PARENTHESIS_CLOSE(R) block(B).							{ A = addStatement(compiler, NULL, F->toKen->type, L, E, R, B, NULL); }

%nonassoc LSL_LABEL.
statement(A) ::= LSL_LABEL(F) LSL_IDENTIFIER(I) LSL_STATEMENT(S).											{ A = addStatement(compiler, S, F->toKen->type, NULL, NULL, NULL, NULL, I); }

beginBlock(A) ::= LSL_BLOCK_OPEN(B).															{ A = beginBlock(compiler, B); }

// This might be bogus, or might be valid LSL, but it lets us test the expression parser by evaluating them.
statement(A) ::= expr(E) LSL_STATEMENT(S).														{ A = addStatement(compiler, S, LSL_EXPRESSION, NULL, E, NULL, NULL, NULL); }

// Various forms of expression.

// Used for function call params, and list contents.
exprList ::= exprList LSL_COMMA expr.
exprList ::= expr.
exprList ::= .

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
statement(A) ::= type(T) LSL_IDENTIFIER(I) LSL_ASSIGNMENT_PLAIN(D) expr(E) LSL_STATEMENT(S).	{ A = addStatement(compiler, S, LSL_IDENTIFIER, NULL, addVariable(compiler, T, I, D,    E),    NULL, NULL, I); }
statement(A) ::= type(T) LSL_IDENTIFIER(I) LSL_STATEMENT(S).					{ A = addStatement(compiler, S, LSL_IDENTIFIER, NULL, addVariable(compiler, T, I, NULL, NULL), NULL, NULL, I); }

%right LSL_DOT LSL_IDENTIFIER LSL_FUNCTION_CALL.
identifier(A) ::= identifier LSL_DOT LSL_IDENTIFIER(B).					{ A = checkVariable(compiler, B); A->basicType = OT_float; }	// Just a stub to get it to work for now.
identifier(A) ::= LSL_IDENTIFIER(B).							{ A = checkVariable(compiler, B); }

%right LSL_DECREMENT_PRE LSL_INCREMENT_PRE LSL_DECREMENT_POST LSL_INCREMENT_POST.
expr(A) ::= identifier(B) LSL_DECREMENT_PRE(C).						{ A = addCrement(compiler, B, C); }
expr(A) ::= identifier(B) LSL_INCREMENT_PRE(C).						{ A = addCrement(compiler, B, C); }
expr(A) ::= LSL_DECREMENT_PRE(C) identifier(B).						{ A = addCrement(compiler, B, C); }
expr(A) ::= LSL_INCREMENT_PRE(C) identifier(B).						{ A = addCrement(compiler, B, C); }

%nonassoc LSL_COMMA.

// Values.

%nonassoc  LSL_FLOAT.
expr(A) ::= LSL_FLOAT(B).							{ B->basicType = OT_float; A = B; }
%nonassoc LSL_INTEGER.
expr(A) ::= LSL_INTEGER(B).							{ B->basicType = OT_integer; A = B; }
%nonassoc  LSL_KEY.
expr(A) ::= LSL_KEY(B).								{ B->basicType = OT_key; A = B; }
%nonassoc  LSL_LIST.
expr(A) ::= LSL_BRACKET_OPEN(B) exprList LSL_BRACKET_CLOSE.	[LSL_BRACKET_OPEN]	{ B->basicType = OT_list; A = B; }	// Probably need a specific addList().
%nonassoc  LSL_ROTATION.
// Uses the same symbol for less than, greater than, and the rotation / vector delimiters.
expr ::= LSL_LESS_THAN expr LSL_COMMA expr LSL_COMMA expr LSL_COMMA expr LSL_GREATER_THAN.	[LSL_ANGLE_OPEN]
%nonassoc  LSL_STRING.
expr(A) ::= LSL_STRING(B).							{ B->basicType = OT_string; A = B; }
%nonassoc  LSL_VECTOR.
expr ::= LSL_VECTOR.
expr ::= LSL_LESS_THAN expr LSL_COMMA expr LSL_COMMA expr LSL_GREATER_THAN.			[LSL_ANGLE_OPEN]


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

