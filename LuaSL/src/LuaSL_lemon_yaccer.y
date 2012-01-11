%include {
#include "LuaSL_LSL_tree.h"
}

%extra_argument {LuaSL_yyparseParam *param}

%stack_size 256

%token_type {LSL_Leaf *}
%default_type  {LSL_Leaf *}
%token_destructor {burnLeaf($$);}


program ::= script LSL_SCRIPT(A).						{ A->left = param->ast;  param->ast = A; }  // Lemon does not like the start symbol to be on the RHS, so give it a dummy one.


%left  LSL_BOOL_AND.
expr(A) ::= expr(B) LSL_BOOL_AND(C)		expr(D).			{ A = addOperation(B, C, D); }
%left  LSL_BOOL_OR.
expr(A) ::= expr(B) LSL_BOOL_OR(C)		expr(D).			{ A = addOperation(B, C, D); }

%left  LSL_BIT_AND LSL_BIT_XOR LSL_BIT_OR.
expr(A) ::= expr(B) LSL_BIT_OR(C)		expr(D).			{ A = addOperation(B, C, D); }
expr(A) ::= expr(B) LSL_BIT_XOR(C)		expr(D).			{ A = addOperation(B, C, D); }
expr(A) ::= expr(B) LSL_BIT_AND(C)		expr(D).			{ A = addOperation(B, C, D); }

%left  LSL_EQUAL LSL_NOT_EQUAL.
expr(A) ::= expr(B) LSL_NOT_EQUAL(C)		expr(D).			{ A = addOperation(B, C, D); }
expr(A) ::= expr(B) LSL_EQUAL(C)		expr(D).			{ A = addOperation(B, C, D); }
%left  LSL_LESS_THAN LSL_GREATER_THAN LSL_LESS_EQUAL LSL_GREATER_EQUAL.
expr(A) ::= expr(B) LSL_GREATER_EQUAL(C)	expr(D).			{ A = addOperation(B, C, D); }
expr(A) ::= expr(B) LSL_LESS_EQUAL(C)		expr(D).			{ A = addOperation(B, C, D); }
expr(A) ::= expr(B) LSL_GREATER_THAN(C)		expr(D).			{ A = addOperation(B, C, D); }
expr(A) ::= expr(B) LSL_LESS_THAN(C)		expr(D).			{ A = addOperation(B, C, D); }

%left  LSL_LEFT_SHIFT LSL_RIGHT_SHIFT.
expr(A) ::= expr(B) LSL_RIGHT_SHIFT(C)		expr(D).			{ A = addOperation(B, C, D); }
expr(A) ::= expr(B) LSL_LEFT_SHIFT(C)		expr(D).			{ A = addOperation(B, C, D); }

%left  LSL_SUBTRACT LSL_ADD.
expr(A) ::= expr(B) LSL_ADD(C)			expr(D).			{ A = addOperation(B, C, D); }
expr(A) ::= expr(B) LSL_SUBTRACT(C)		expr(D).			{ A = addOperation(B, C, D); }
%left  LSL_DIVIDE LSL_MODULO LSL_MULTIPLY.
expr(A) ::= expr(B) LSL_MULTIPLY(C)		expr(D).			{ A = addOperation(B, C, D); }
expr(A) ::= expr(B) LSL_MODULO(C)		expr(D).			{ A = addOperation(B, C, D); }
expr(A) ::= expr(B) LSL_DIVIDE(C)		expr(D).			{ A = addOperation(B, C, D); }

%right LSL_BIT_NOT LSL_BOOL_NOT LSL_NEGATION.
expr(A) ::= LSL_BIT_NOT(B)			expr(C).			{ A = addOperation(NULL, B, C); }
expr(A) ::= LSL_BOOL_NOT(B)			expr(C).			{ A = addOperation(NULL, B, C); }
expr(A) ::= LSL_SUBTRACT(B)			expr(C).	[LSL_NEGATION]	{ A = addOperation(NULL, B, C); }

%left  LSL_ANGLE_OPEN LSL_ANGLE_CLOSE.
%nonassoc LSL_BRACKET_OPEN LSL_BRACKET_CLOSE.
%nonassoc LSL_PARENTHESIS_OPEN LSL_PARENTHESIS_CLOSE LSL_EXPRESSION.
expr(A) ::= LSL_PARENTHESIS_OPEN(B)	expr(C) LSL_PARENTHESIS_CLOSE(D).	{ A = addParenthesis(B, C, D); }

%right LSL_ASSIGNMENT_ADD LSL_ASSIGNMENT_SUBTRACT LSL_ASSIGNMENT_MULTIPLY LSL_ASSIGNMENT_MODULO LSL_ASSIGNMENT_DIVIDE LSL_ASSIGNMENT_PLAIN.
%right LSL_DOT.
%right LSL_DECREMENT_PRE LSL_INCREMENT_PRE.
%nonassoc LSL_COMMA.

%nonassoc  LSL_FLOAT.
%nonassoc LSL_INTEGER.
expr(A) ::= LSL_INTEGER(B).							{ A = B; }

%nonassoc LSL_TYPE_FLOAT LSL_TYPE_INTEGER LSL_TYPE_KEY LSL_TYPE_LIST LSL_TYPE_ROTATION LSL_TYPE_STRING LSL_TYPE_VECTOR.

%nonassoc LSL_DO LSL_FOR LSL_ELSE LSL_IF LSL_JUMP LSL_RETURN LSL_STATE_CHANGE LSL_WHILE.

%nonassoc LSL_LABEL.

%nonassoc LSL_BLOCK_OPEN LSL_BLOCK_CLOSE.

%nonassoc LSL_STATEMENT.
statement(A) ::= expr(B) LSL_STATEMENT(D).					{ A = addStatement(D, LSL_EXPRESSION, B); }

%nonassoc LSL_SPACE LSL_COMMENT LSL_COMMENT_LINE LSL_IDENTIFIER LSL_SCRIPT LSL_UNKNOWN.
script ::= script statement(A).							{ A->left = param->ast;  param->ast = A; }
script ::= statement(A).							{ A->left = param->ast;  param->ast = A; }


%parse_accept {printf("Parsing complete.\n");}

%parse_failure {fprintf(stderr,"Giving up.  Parser is hopelessly lost...\n");}

%stack_overflow {fprintf(stderr,"Giving up.  Parser stack overflow!\n");}

%syntax_error {fprintf(stderr,"Syntax error!\n");}

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

*/

