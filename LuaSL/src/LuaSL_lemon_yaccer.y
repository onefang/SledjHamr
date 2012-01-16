%include {
#include "LuaSL_LSL_tree.h"
}

%extra_argument {LuaSL_yyparseParam *param}

%stack_size 1024

%token_type {LSL_Leaf *}
%default_type  {LSL_Leaf *}
%token_destructor {burnLeaf($$);}
%default_destructor {burnLeaf($$);}

// The start symbol, just coz we need one.
// Lemon does not like the start symbol to be on the RHS, so give it a dummy start symbol.

program ::= script LSL_SCRIPT(A).						{ if (NULL != A) A->left = param->ast;  param->ast = A; }

// Various forms of "space".  The lexer takes care of them for us.

%nonassoc LSL_SPACE LSL_COMMENT LSL_COMMENT_LINE LSL_UNKNOWN.

// Basic script structure.

%nonassoc LSL_SCRIPT.
script ::= script state.
script ::= script function.
script ::= script statement(A).							{ if (NULL != A) A->left = param->ast;  param->ast = A; }
script ::= .

// State definitions.

%nonassoc LSL_BLOCK_OPEN LSL_BLOCK_CLOSE LSL_STATE.
stateBlock ::= LSL_BLOCK_OPEN functionList LSL_BLOCK_CLOSE.
state ::= LSL_IDENTIFIER stateBlock.

// Function definitions.

%nonassoc LSL_PARAMETER LSL_FUNCTION.
functionList ::= functionList function.
functionList ::= .

parameter ::= type LSL_IDENTIFIER.
parameterList ::= parameterList LSL_COMMA parameter.
parameterList ::= parameter.
parameterList ::= .
function ::= LSL_IDENTIFIER LSL_PARENTHESIS_OPEN parameterList LSL_PARENTHESIS_CLOSE funcBlock.		// Causes a conflict when it's an empty parameterList with calling the same type of function.
function ::= type LSL_IDENTIFIER LSL_PARENTHESIS_OPEN parameterList LSL_PARENTHESIS_CLOSE funcBlock.

// Blocks.

block ::= funcBlock.
block ::= statement.
funcBlock ::= LSL_BLOCK_OPEN statementList LSL_BLOCK_CLOSE.

// Various forms of statement.

%nonassoc LSL_STATEMENT.
statementList ::= statementList statement.
statementList ::= .

%nonassoc LSL_DO LSL_FOR LSL_ELSE_IF LSL_IF LSL_JUMP LSL_RETURN LSL_STATE_CHANGE LSL_WHILE.
%nonassoc LSL_ELSE.
statement ::= LSL_DO block LSL_WHILE LSL_PARENTHESIS_OPEN expr LSL_PARENTHESIS_CLOSE LSL_STATEMENT.
statement ::= LSL_FOR LSL_PARENTHESIS_OPEN expr LSL_STATEMENT expr LSL_STATEMENT expr LSL_PARENTHESIS_CLOSE block.

ifBlock ::= ifBlock LSL_ELSE block.
ifBlock ::= block.
statement ::= LSL_IF LSL_PARENTHESIS_OPEN expr LSL_PARENTHESIS_CLOSE ifBlock.	[LSL_ELSE]	// The [LSL_ELSE] part causes a conflict.

statement ::= LSL_JUMP LSL_IDENTIFIER LSL_STATEMENT.
statement ::= LSL_RETURN expr LSL_STATEMENT.
statement ::= LSL_RETURN LSL_STATEMENT.
statement ::= LSL_STATE_CHANGE LSL_IDENTIFIER LSL_STATEMENT.
statement ::= LSL_WHILE LSL_PARENTHESIS_OPEN expr LSL_PARENTHESIS_CLOSE block.

%nonassoc LSL_LABEL.
statement ::= LSL_LABEL LSL_IDENTIFIER LSL_STATEMENT.

// This might be bogus, or might be valid LSL, but it let us test the expression parser by evaluating them.
statement(A) ::= expr(B) LSL_STATEMENT(D).					{ A = addStatement(D, LSL_EXPRESSION, B); }

// Various forms of expression.

exprList ::= exprList LSL_COMMA expr.
exprList ::= expr.
exprList ::= .

%right  LSL_BOOL_AND.
expr(A) ::= expr(B) LSL_BOOL_AND(C)		expr(D).			{ A = addOperation(B, C, D); }
%right  LSL_BOOL_OR.
expr(A) ::= expr(B) LSL_BOOL_OR(C)		expr(D).			{ A = addOperation(B, C, D); }

%left  LSL_BIT_AND LSL_BIT_XOR LSL_BIT_OR.
expr(A) ::= expr(B) LSL_BIT_OR(C)		expr(D).			{ A = addOperation(B, C, D); }
expr(A) ::= expr(B) LSL_BIT_XOR(C)		expr(D).			{ A = addOperation(B, C, D); }
expr(A) ::= expr(B) LSL_BIT_AND(C)		expr(D).			{ A = addOperation(B, C, D); }

%right  LSL_EQUAL LSL_NOT_EQUAL.
expr(A) ::= expr(B) LSL_NOT_EQUAL(C)		expr(D).			{ A = addOperation(B, C, D); }
expr(A) ::= expr(B) LSL_EQUAL(C)		expr(D).			{ A = addOperation(B, C, D); }
%right  LSL_LESS_THAN LSL_GREATER_THAN LSL_LESS_EQUAL LSL_GREATER_EQUAL.
expr(A) ::= expr(B) LSL_GREATER_EQUAL(C)	expr(D).			{ A = addOperation(B, C, D); }
expr(A) ::= expr(B) LSL_LESS_EQUAL(C)		expr(D).			{ A = addOperation(B, C, D); }
expr(A) ::= expr(B) LSL_GREATER_THAN(C)		expr(D).			{ A = addOperation(B, C, D); }
expr(A) ::= expr(B) LSL_LESS_THAN(C)		expr(D).			{ A = addOperation(B, C, D); }

%left  LSL_LEFT_SHIFT LSL_RIGHT_SHIFT.
expr(A) ::= expr(B) LSL_RIGHT_SHIFT(C)		expr(D).			{ A = addOperation(B, C, D); }
expr(A) ::= expr(B) LSL_LEFT_SHIFT(C)		expr(D).			{ A = addOperation(B, C, D); }

%left  LSL_SUBTRACT LSL_ADD LSL_CONCATENATE.
expr(A) ::= expr(B) LSL_ADD(C)			expr(D).			{ A = addOperation(B, C, D); }
expr(A) ::= expr(B) LSL_SUBTRACT(C)		expr(D).			{ A = addOperation(B, C, D); }
%left  LSL_DIVIDE LSL_MODULO LSL_MULTIPLY LSL_DOT_PRODUCT LSL_CROSS_PRODUCT.
expr(A) ::= expr(B) LSL_MULTIPLY(C)		expr(D).			{ A = addOperation(B, C, D); }
expr(A) ::= expr(B) LSL_MODULO(C)		expr(D).			{ A = addOperation(B, C, D); }
expr(A) ::= expr(B) LSL_DIVIDE(C)		expr(D).			{ A = addOperation(B, C, D); }

%right LSL_BIT_NOT LSL_BOOL_NOT LSL_NEGATION.
expr(A) ::= LSL_BIT_NOT(B)			expr(C).			{ A = addOperation(NULL, B, C); }
expr(A) ::= LSL_BOOL_NOT(B)			expr(C).			{ A = addOperation(NULL, B, C); }
expr(A) ::= LSL_SUBTRACT(B)			expr(C).	[LSL_NEGATION]	{ A = addOperation(NULL, B, C); }

// Types, typecasts, and expression reordering.

%right  LSL_TYPECAST_OPEN LSL_TYPECAST_CLOSE.
%nonassoc LSL_TYPE_FLOAT LSL_TYPE_INTEGER LSL_TYPE_KEY LSL_TYPE_LIST LSL_TYPE_ROTATION LSL_TYPE_STRING LSL_TYPE_VECTOR.
type ::= LSL_TYPE_FLOAT.
type ::= LSL_TYPE_INTEGER.
type ::= LSL_TYPE_KEY.
type ::= LSL_TYPE_LIST.
type ::= LSL_TYPE_ROTATION.
type ::= LSL_TYPE_STRING.
type ::= LSL_TYPE_VECTOR.

%left  LSL_ANGLE_OPEN LSL_ANGLE_CLOSE.
%nonassoc LSL_BRACKET_OPEN LSL_BRACKET_CLOSE.
%nonassoc LSL_PARENTHESIS_OPEN LSL_PARENTHESIS_CLOSE LSL_EXPRESSION.

expr(A) ::= LSL_PARENTHESIS_OPEN(B)	expr(C) LSL_PARENTHESIS_CLOSE(D).	{ A = addParenthesis(B, C, D); }
expr(A) ::= LSL_PARENTHESIS_OPEN(B)	type(C)	LSL_PARENTHESIS_CLOSE(D) expr(E).	{ A = addTypecast(B, C, D, E); }

// Function call.

expr ::= LSL_IDENTIFIER LSL_PARENTHESIS_OPEN exprList LSL_PARENTHESIS_CLOSE.		// Casuses a conflict when exprList is empty with a function definition with no type and no parameters.

// Variables and dealing with them.

expr ::= identifier.

%right LSL_ASSIGNMENT_CONCATENATE LSL_ASSIGNMENT_ADD LSL_ASSIGNMENT_SUBTRACT LSL_ASSIGNMENT_MULTIPLY LSL_ASSIGNMENT_MODULO LSL_ASSIGNMENT_DIVIDE LSL_ASSIGNMENT_PLAIN.
expr ::= identifier LSL_ASSIGNMENT_CONCATENATE expr.
expr ::= identifier LSL_ASSIGNMENT_ADD expr.
expr ::= identifier LSL_ASSIGNMENT_SUBTRACT expr.
expr ::= identifier LSL_ASSIGNMENT_MULTIPLY expr.
expr ::= identifier LSL_ASSIGNMENT_MODULO expr.
expr ::= identifier LSL_ASSIGNMENT_DIVIDE expr.
expr ::= identifier LSL_ASSIGNMENT_PLAIN expr.

// Hmm think this can have commas seperating the assignment parts.
statement ::= type LSL_IDENTIFIER LSL_ASSIGNMENT_PLAIN expr LSL_STATEMENT.
statement ::= type LSL_IDENTIFIER LSL_STATEMENT.

%right LSL_DOT LSL_IDENTIFIER.
identifier ::= identifier LSL_DOT LSL_IDENTIFIER.
identifier ::= LSL_IDENTIFIER.

%right LSL_DECREMENT_PRE LSL_INCREMENT_PRE LSL_DECREMENT_POST LSL_INCREMENT_POST.
expr ::= identifier LSL_DECREMENT_PRE.
expr ::= identifier LSL_INCREMENT_PRE.
expr ::= LSL_DECREMENT_PRE identifier.
expr ::= LSL_INCREMENT_PRE identifier.

%nonassoc LSL_COMMA.

// Values.

%nonassoc  LSL_FLOAT.
expr(A) ::= LSL_FLOAT(B).							{ B->basicType = OT_float; A = B; }
%nonassoc LSL_INTEGER.
expr(A) ::= LSL_INTEGER(B).							{ B->basicType = OT_integer; A = B; }
%nonassoc  LSL_KEY.
expr(A) ::= LSL_KEY(B).								{ B->basicType = OT_key; A = B; }
%nonassoc  LSL_LIST.
expr ::= LSL_BRACKET_OPEN exprList LSL_BRACKET_CLOSE.						[LSL_BRACKET_OPEN]
%nonassoc  LSL_ROTATION.
expr ::= LSL_ANGLE_OPEN expr LSL_COMMA expr LSL_COMMA expr LSL_COMMA expr LSL_ANGLE_CLOSE.	[LSL_ANGLE_OPEN]
%nonassoc  LSL_STRING.
expr(A) ::= LSL_STRING(B).							{ B->basicType = OT_string; A = B; }
%nonassoc  LSL_VECTOR.
expr ::= LSL_VECTOR.
expr ::= LSL_ANGLE_OPEN expr LSL_COMMA expr LSL_COMMA expr LSL_ANGLE_CLOSE.			[LSL_ANGLE_OPEN]


// Parser callbacks.

%parse_accept {printf("Parsing complete.\n");}

%parse_failure {fprintf(stderr,"Giving up.  Parser is hopelessly lost!\n");}

%stack_overflow {fprintf(stderr,"*******************************************************************Giving up.  Parser stack overflow @ line %04d column %04d\n", yypMinor->yy0->line, yypMinor->yy0->column);}  // Gotta love consistancy, if it ever happens.

%syntax_error {fprintf(stderr,"*******************************************************************Syntax error @ line %04d column %04d\n", yyminor.yy0->line, yyminor.yy0->column);}


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

