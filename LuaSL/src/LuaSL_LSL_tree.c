/*
 * Implementation of functions used to build the abstract syntax tree.
 */


#define LSL_Keywords_define
#define LSL_Tokens_define
#include "LuaSL_LSL_tree.h"
#include <stdlib.h>
#include <stdio.h>


/**
 * @brief Allocates space for an AST leaf
 * @return The expression or NULL if not enough memory
 */

/*
static LSL_AST *newLeaf(LSL_Type type, LSL_AST *left, LSL_AST *right)
{
    LSL_AST *leaf = malloc(sizeof(LSL_AST));

    if (leaf == NULL) return NULL;

    leaf->type = type;
    leaf->left = left;
    leaf->right = right;
    leaf->line = -1;
    leaf->character = -1;

    return leaf;
}
*/

void burnLeaf(LSL_AST *leaf)
{
    if (leaf == NULL) return;

    burnLeaf(leaf->left);
    burnLeaf(leaf->right);
    free(leaf);
}

static LSL_Expression *newLSLExpression(LSL_Type type, LSL_Expression *left, LSL_Expression *right)
{
    LSL_Expression *exp = malloc(sizeof(LSL_Expression));

    if (exp == NULL) return NULL;

    exp->type = type;
    exp->left = left;
    exp->right = right;
    exp->expression=0;

    return exp;
}

void burnLSLExpression(LSL_Expression *exp)
{
    if (exp == NULL) return;

    burnLSLExpression(exp->left);
    burnLSLExpression(exp->right);
    free(exp);
}

LSL_Expression *addInteger(int value)
{
    LSL_Expression *exp = newLSLExpression(LSL_INTEGER, NULL, NULL);

    if (exp)
	exp->value.integerValue = value;

    return exp;
}

LSL_Expression *addOperation(LSL_Operation type, LSL_Expression *left, LSL_Expression *right)
{
    LSL_Expression *exp = newLSLExpression(LSL_EXPRESSION, left, right);

    if (exp)
	exp->expression = type;

    return exp;
}

LSL_Expression *newTree(const char *expr)
{
    LuaSL_yyparseParam param;
    YY_BUFFER_STATE state;

#ifdef LUASL_DEBUG
    yydebug= 5;
#endif

    param.expression = NULL;
    if (yylex_init(&(param.scanner)))
	return NULL;

#ifdef LUASL_DEBUG
    yyset_debug(1, param.scanner);
#endif

    state = yy_scan_string(expr, param.scanner);
    if (yyparse(&param))
	return NULL;

    yy_delete_buffer(state, param.scanner);
    yylex_destroy(param.scanner);

    return param.expression;
}

int evaluateExpression(LSL_Expression *exp, int old)
{
    switch(exp->type)
    {
#ifdef LUASL_USE_ENUM
	case LSL_COMMENT	:
	case LSL_TYPE		:
	case LSL_NAME		:
	case LSL_IDENTIFIER	:
	    break;
	case LSL_FLOAT		: return (int) exp->value.floatValue;
#endif
	case LSL_INTEGER	:
#ifdef LUASL_DEBUG
	    printf("%d", exp->value.integerValue);
#endif
	    return exp->value.integerValue;
#ifdef LUASL_USE_ENUM
	case LSL_STRING		:
	case LSL_KEY		:
	case LSL_VECTOR		:
	case LSL_ROTATION	:
	case LSL_LIST		:
	case LSL_LABEL		:
	    break;
#endif
	case LSL_EXPRESSION	:
	{
	    switch (exp->expression)
	    {
#ifdef LUASL_USE_ENUM
		case LSL_COMMA			:
		case LSL_INCREMENT_PRE		:
		case LSL_INCREMENT_POST		:
		case LSL_DECREMENT_PRE		:
		case LSL_DECREMENT_POST		:
		case LSL_DOT			:
		case LSL_ASSIGNMENT_PLAIN	:
		case LSL_ASSIGNMENT_DIVIDE	:
		case LSL_ASSIGNMENT_MODULO	:
		case LSL_ASSIGNMENT_MULTIPLY	:
		case LSL_ASSIGNMENT_SUBTRACT	:
		case LSL_ASSIGNMENT_ADD		:
		case LSL_ASSIGNMENT_CONCATENATE	:
		case LSL_PARENTHESIS_OPEN	:
		case LSL_PARENTHESIS_CLOSE	:
		case LSL_BRACKET_OPEN		:
		case LSL_BRACKET_CLOSE		:
		case LSL_ANGLE_OPEN		:
		case LSL_ANGLE_CLOSE		:
		case LSL_TYPECAST		:
		case LSL_BIT_NOT		:
		case LSL_BOOL_NOT		:
		case LSL_NEGATION		:
		    break;
		case LSL_DIVIDE			: return evaluateExpression(exp->left, old) /  evaluateExpression(exp->right, old);
		case LSL_MODULO			: return evaluateExpression(exp->left, old) %  evaluateExpression(exp->right, old);
#endif
		case LSL_MULTIPLY		:
#ifdef LUASL_DEBUG
		    printf(" * ");
#endif
		    return evaluateExpression(exp->left, old) *  evaluateExpression(exp->right, old);
#ifdef LUASL_USE_ENUM
		case LSL_DOT_PRODUCT		: break;
		case LSL_CROSS_PRODUCT		: break;
		case LSL_SUBTRACT		: return evaluateExpression(exp->left, old) -  evaluateExpression(exp->right, old);
#endif
		case LSL_ADD			:
#ifdef LUASL_DEBUG
		    printf(" + ");
#endif
		    return evaluateExpression(exp->left, old) +  evaluateExpression(exp->right, old);
#ifdef LUASL_USE_ENUM
		case LSL_CONCATENATE		: break;
		case LSL_LEFT_SHIFT		: return evaluateExpression(exp->left, old) << evaluateExpression(exp->right, old);
		case LSL_RIGHT_SHIFT		: return evaluateExpression(exp->left, old) >> evaluateExpression(exp->right, old);
		case LSL_LESS_THAN		: return evaluateExpression(exp->left, old) <  evaluateExpression(exp->right, old);
		case LSL_GREATER_THAN		: return evaluateExpression(exp->left, old) >  evaluateExpression(exp->right, old);
		case LSL_LESS_EQUAL		: return evaluateExpression(exp->left, old) <= evaluateExpression(exp->right, old);
		case LSL_GREATER_EQUAL		: return evaluateExpression(exp->left, old) >= evaluateExpression(exp->right, old);
		case LSL_EQUAL			: return evaluateExpression(exp->left, old) == evaluateExpression(exp->right, old);
		case LSL_NOT_EQUAL		: return evaluateExpression(exp->left, old) != evaluateExpression(exp->right, old);
		case LSL_BIT_AND		: return evaluateExpression(exp->left, old) &  evaluateExpression(exp->right, old);
		case LSL_BIT_XOR		: return evaluateExpression(exp->left, old) ^  evaluateExpression(exp->right, old);
		case LSL_BIT_OR			: return evaluateExpression(exp->left, old) |  evaluateExpression(exp->right, old);
		case LSL_BOOL_OR		: return evaluateExpression(exp->left, old) || evaluateExpression(exp->right, old);
		case LSL_BOOL_AND		: return evaluateExpression(exp->left, old) && evaluateExpression(exp->right, old);
#endif
	    }
	    break;
	}
#ifdef LUASL_USE_ENUM
	case LSL_DO		:
	case LSL_FOR		:
	case LSL_IF		:
	case LSL_ELSE		:
	case LSL_ELSEIF		:
	case LSL_JUMP		:
	case LSL_STATE_CHANGE	:
	case LSL_WHILE		:
	case LSL_RETURN		:
	case LSL_STATEMENT	:
	case LSL_BLOCK		:
	case LSL_PARAMETER	:
	case LSL_FUNCTION	:
	case LSL_STATE		:
	case LSL_SCRIPT		:
	    break;
#endif
    }

    return old;

}

void outputExpression(LSL_Expression *exp)
{
    switch(exp->type)
    {
#ifdef LUASL_USE_ENUM
	case LSL_COMMENT	: return;
	case LSL_TYPE		: return;
	case LSL_NAME		: return;
	case LSL_IDENTIFIER	: return;
	case LSL_FLOAT		: printf("%f", exp->value.floatValue);		break;
#endif
	case LSL_INTEGER	: printf("%d", exp->value.integerValue);	break;
#ifdef LUASL_USE_ENUM
	case LSL_STRING		: return;
	case LSL_KEY		: return;
	case LSL_VECTOR		: return;
	case LSL_ROTATION	: return;
	case LSL_LIST		: return;
	case LSL_LABEL		: return;
#endif
	case LSL_EXPRESSION	:
	    outputExpression(exp->left);
#ifdef LUASL_USE_ENUM
	    printf(" %s ", LSL_Tokens[exp->expression - LSL_COMMA].token);
#else
	    printf(" # ");
#endif
	    outputExpression(exp->right);
	    break;
#ifdef LUASL_USE_ENUM
	case LSL_DO		: return;
	case LSL_FOR		: return;
	case LSL_IF		: return;
	case LSL_ELSE		: return;
	case LSL_ELSEIF		: return;
	case LSL_JUMP		: return;
	case LSL_STATE_CHANGE	: return;
	case LSL_WHILE		: return;
	case LSL_RETURN		: return;
	case LSL_STATEMENT	: return;
	case LSL_BLOCK		: return;
	case LSL_PARAMETER	: return;
	case LSL_FUNCTION	: return;
	case LSL_STATE		: return;
	case LSL_SCRIPT		: return;
#endif
    }
}

void convertExpression2Lua(LSL_Expression *exp)
{
#ifdef LUASL_USE_ENUM
    switch(exp->type)
    {
	case LSL_COMMENT	: return;
	case LSL_TYPE		: return;
	case LSL_NAME		: return;
	case LSL_IDENTIFIER	: return;
	case LSL_FLOAT		: return;
	case LSL_INTEGER	: return;
	case LSL_STRING		: return;
	case LSL_KEY		: return;
	case LSL_VECTOR		: return;
	case LSL_ROTATION	: return;
	case LSL_LIST		: return;
	case LSL_LABEL		: return;
	case LSL_EXPRESSION	: return;
	case LSL_DO		: return;
	case LSL_FOR		: return;
	case LSL_IF		: return;
	case LSL_ELSE		: return;
	case LSL_ELSEIF		: return;
	case LSL_JUMP		: return;
	case LSL_STATE_CHANGE	: return;
	case LSL_WHILE		: return;
	case LSL_RETURN		: return;
	case LSL_STATEMENT	: return;
	case LSL_BLOCK		: return;
	case LSL_PARAMETER	: return;
	case LSL_FUNCTION	: return;
	case LSL_STATE		: return;
	case LSL_SCRIPT		: return;
    }
#endif
}

int yyerror(const char *msg)
{
    fprintf(stderr, "Parser error: %s\n", msg);
    return 0;
}

int main(void)
{
    const char test[] = " 4 + 2 * 10 + 3 * ( 5 + 1 )";
    LSL_Expression *exp;

    if ((exp = newTree(test)))
    {
	int result = evaluateExpression(exp, 0);

#ifdef LUASL_DEBUG
	printf("\n");
#endif
	printf("Result of '%s' is %d\n", test, result);
	outputExpression(exp);
	printf("\n");
	burnLSLExpression(exp);
    }

    return 0;
}

