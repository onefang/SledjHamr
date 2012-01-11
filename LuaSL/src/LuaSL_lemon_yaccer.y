%include {
#include "LuaSL_LSL_tree.h"
}

%extra_argument { LuaSL_yyparseParam *param }

%stack_size 256

%token_type {LSL_Leaf *}
%default_type  {LSL_Leaf *}
%token_destructor { burnLeaf($$); }


program ::= script LSL_SCRIPT(A).					{ A->left = param->ast;  param->ast = A; }  // Lemon does not like the start symbol to be on the RHS, so give it a dummy one.


%left  LSL_BOOL_AND.
expr(A) ::= expr(B) LSL_BOOL_AND(D) expr(C).				{ A = addOperation(D, D->token->type,		B, C); }
%left  LSL_BOOL_OR.
expr(A) ::= expr(B) LSL_BOOL_OR(D) expr(C).				{ A = addOperation(D, LSL_BOOL_OR,		B, C); }
%left  LSL_BIT_AND LSL_BIT_XOR LSL_BIT_OR.
expr(A) ::= expr(B) LSL_BIT_OR(D) expr(C).				{ A = addOperation(D, LSL_BIT_OR,		B, C); }
expr(A) ::= expr(B) LSL_BIT_XOR(D) expr(C).				{ A = addOperation(D, LSL_BIT_XOR,		B, C); }
expr(A) ::= expr(B) LSL_BIT_AND(D) expr(C).				{ A = addOperation(D, LSL_BIT_AND,		B, C); }
%left  LSL_EQUAL LSL_NOT_EQUAL.
expr(A) ::= expr(B) LSL_NOT_EQUAL(D) expr(C).				{ A = addOperation(D, LSL_NOT_EQUAL,		B, C); }
expr(A) ::= expr(B) LSL_EQUAL(D) expr(C).				{ A = addOperation(D, LSL_EQUAL,		B, C); }
%left  LSL_LESS_THAN LSL_GREATER_THAN LSL_LESS_EQUAL LSL_GREATER_EQUAL.
expr(A) ::= expr(B) LSL_GREATER_EQUAL(D) expr(C).			{ A = addOperation(D, LSL_GREATER_EQUAL,	B, C); }
expr(A) ::= expr(B) LSL_LESS_EQUAL(D) expr(C).				{ A = addOperation(D, LSL_LESS_EQUAL,		B, C); }
expr(A) ::= expr(B) LSL_GREATER_THAN(D) expr(C).			{ A = addOperation(D, LSL_GREATER_THAN,		B, C); }
expr(A) ::= expr(B) LSL_LESS_THAN(D) expr(C).				{ A = addOperation(D, LSL_LESS_THAN,		B, C); }
%left  LSL_LEFT_SHIFT LSL_RIGHT_SHIFT.
expr(A) ::= expr(B) LSL_RIGHT_SHIFT(D) expr(C).				{ A = addOperation(D, LSL_RIGHT_SHIFT,		B, C); }
expr(A) ::= expr(B) LSL_LEFT_SHIFT(D) expr(C).				{ A = addOperation(D, LSL_LEFT_SHIFT,		B, C); }
%left  LSL_SUBTRACT LSL_ADD.
expr(A) ::= expr(B) LSL_ADD(D) expr(C).					{ A = addOperation(D, LSL_ADD,			B, C); }
expr(A) ::= expr(B) LSL_SUBTRACT(D) expr(C).				{ A = addOperation(D, LSL_SUBTRACT,		B, C); }
%left  LSL_DIVIDE LSL_MODULO LSL_MULTIPLY.
expr(A) ::= expr(B) LSL_MULTIPLY(D) expr(C).				{ A = addOperation(D, LSL_MULTIPLY,		B, C); }
expr(A) ::= expr(B) LSL_MODULO(D) expr(C).				{ A = addOperation(D, LSL_MODULO,		B, C); }
expr(A) ::= expr(B) LSL_DIVIDE(D) expr(C).				{ A = addOperation(D, LSL_DIVIDE,		B, C); }
%right LSL_BIT_NOT LSL_BOOL_NOT LSL_NEGATION.
expr(A) ::= LSL_BIT_NOT(D) expr(B).					{ A = addOperation(D, LSL_BIT_NOT,		NULL, B); }
expr(A) ::= LSL_BOOL_NOT(D) expr(B).					{ A = addOperation(D, LSL_BOOL_NOT,		NULL, B); }
expr(A) ::= LSL_SUBTRACT(D) expr(B).		[LSL_NEGATION]		{ A = addOperation(D, LSL_NEGATION,		NULL, B); }
%left  LSL_ANGLE_OPEN LSL_ANGLE_CLOSE.
%nonassoc LSL_BRACKET_OPEN LSL_BRACKET_CLOSE.
%nonassoc LSL_PARENTHESIS_OPEN LSL_PARENTHESIS_CLOSE LSL_EXPRESSION.
expr(A) ::= LSL_PARENTHESIS_OPEN(B) expr(C) LSL_PARENTHESIS_CLOSE(D).	{ A = addParenthesis(B, C, D); }
%right LSL_ASSIGNMENT_ADD LSL_ASSIGNMENT_SUBTRACT LSL_ASSIGNMENT_MULTIPLY LSL_ASSIGNMENT_MODULO LSL_ASSIGNMENT_DIVIDE LSL_ASSIGNMENT_PLAIN.
%right LSL_DOT.
%right LSL_DECREMENT_PRE LSL_INCREMENT_PRE.
%nonassoc LSL_COMMA.

%nonassoc  LSL_FLOAT.
%nonassoc LSL_INTEGER.
expr(A) ::= LSL_INTEGER(B).						{ A = B; }

%nonassoc LSL_TYPE_FLOAT LSL_TYPE_INTEGER LSL_TYPE_KEY LSL_TYPE_LIST LSL_TYPE_ROTATION LSL_TYPE_STRING LSL_TYPE_VECTOR.

%nonassoc LSL_DO LSL_FOR LSL_ELSE LSL_IF LSL_JUMP LSL_RETURN LSL_STATE_CHANGE LSL_WHILE.

%nonassoc LSL_LABEL.

%nonassoc LSL_BLOCK_OPEN LSL_BLOCK_CLOSE.

%nonassoc LSL_STATEMENT.
statement(A) ::= expr(B) LSL_STATEMENT(D).				{ A = addStatement(D, LSL_EXPRESSION,		B); }

%nonassoc LSL_SPACE LSL_COMMENT LSL_COMMENT_LINE LSL_IDENTIFIER LSL_SCRIPT LSL_UNKNOWN.
script ::= script statement(A).						{ A->left = param->ast;  param->ast = A; }
script ::= statement(A).						{ A->left = param->ast;  param->ast = A; }


%parse_accept {printf("Parsing complete.\n");}

%parse_failure {
    fprintf(stderr,"Giving up.  Parser is hopelessly lost...\n");
}

%stack_overflow {
    fprintf(stderr,"Giving up.  Parser stack overflow!\n");
}

%syntax_error {
    fprintf(stderr,"Syntax error!\n");
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

*/

