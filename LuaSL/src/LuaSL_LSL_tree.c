
#define LSL_Keywords_define
#define LSL_Tokens_define
#include "LuaSL_LSL_tree.h"
#include <stdlib.h>
#include <stdio.h>


static LSL_AST *newAST(LSL_Type type, LSL_AST *left, LSL_AST *right)
{
    LSL_AST *ast = malloc(sizeof(LSL_AST));

    if (ast == NULL) return NULL;

    ast->type = type;
    ast->left = left;
    ast->right = right;
    ast->line = -1;
    ast->character = -1;

    return ast;
}

void burnAST(LSL_AST *ast)
{
    if (ast == NULL) return;

    burnAST(ast->left);
    burnAST(ast->right);
    free(ast);
}

LSL_AST *addExpression(LSL_Expression *exp)
{
    LSL_AST *ast = newAST(LSL_EXPRESSION, NULL, NULL);

    if (ast)
	ast->content.expressionValue = exp;

    return ast;
}

static LSL_Expression *newLSLExpression(LSL_Type type, LSL_Expression *left, LSL_Expression *right)
{
    LSL_Expression *exp = malloc(sizeof(LSL_Expression));

    if (exp == NULL) return NULL;

    exp->type = type;
    exp->left = left;
    exp->right = right;

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
	exp->content.integerValue = value;

    return exp;
}

LSL_Expression *addOperation(LSL_Operation type, LSL_Expression *left, LSL_Expression *right)
{
    LSL_Expression *exp = newLSLExpression(LSL_EXPRESSION, left, right);

    if (exp)
	exp->content.operationValue = type;

    return exp;
}

int evaluateExpression(LSL_Expression *exp, int old)
{
    if (NULL == exp)
	return old;
#ifdef LUASL_DEBUG
    #ifdef LUASL_USE_ENUM
	printf(" %s ", LSL_Tokens[exp->content.operationValue - LSL_COMMA].token);
    #else
	printf(" # ");
    #endif
#endif

    if (LSL_INTEGER == exp->type)
    {
#ifdef LUASL_DEBUG
	printf("%d", exp->content.integerValue);
#endif
	return exp->content.integerValue;
    }

    switch (exp->content.operationValue)
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
	    break;
#endif
	case LSL_BIT_NOT		: return ~ evaluateExpression(exp->right, old);
	case LSL_BOOL_NOT		: return ! evaluateExpression(exp->right, old);
	case LSL_NEGATION		: return 0 - evaluateExpression(exp->right, old);
	case LSL_DIVIDE			: return evaluateExpression(exp->left, old) /  evaluateExpression(exp->right, old);
#ifdef LUASL_USE_ENUM
	case LSL_MODULO			: return evaluateExpression(exp->left, old) %  evaluateExpression(exp->right, old);
#endif
	case LSL_MULTIPLY		: return evaluateExpression(exp->left, old) *  evaluateExpression(exp->right, old);
#ifdef LUASL_USE_ENUM
	case LSL_DOT_PRODUCT		: break;
	case LSL_CROSS_PRODUCT		: break;
#endif
	case LSL_SUBTRACT		: return evaluateExpression(exp->left, old) -  evaluateExpression(exp->right, old);
	case LSL_ADD			: return evaluateExpression(exp->left, old) +  evaluateExpression(exp->right, old);
#ifdef LUASL_USE_ENUM
	case LSL_CONCATENATE		: break;
#endif
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
    }

    return old;
}

int evaluateAST(LSL_AST *ast, int old)
{
    if (NULL == ast)
	return old;
    switch(ast->type)
    {
#ifdef LUASL_USE_ENUM
	case LSL_COMMENT	:
	case LSL_TYPE		:
	case LSL_NAME		:
	case LSL_IDENTIFIER	:
	    break;
	case LSL_FLOAT		: return (int) ast->content.floatValue;
#endif
	case LSL_INTEGER	:
#ifdef LUASL_DEBUG
	    printf("%d", ast->content.integerValue);
#endif
	    return ast->content.integerValue;
#ifdef LUASL_USE_ENUM
	case LSL_STRING		:
	case LSL_KEY		:
	case LSL_VECTOR		:
	case LSL_ROTATION	:
	case LSL_LIST		:
	case LSL_LABEL		:
	    break;
#endif
	case LSL_EXPRESSION	: return evaluateExpression(ast->content.expressionValue, old);
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
    if (NULL == exp)
	return;

    if (LSL_INTEGER == exp->type)
    {
	printf("%d", exp->content.integerValue);
    }
    else
    {
	outputExpression(exp->left);
#ifdef LUASL_USE_ENUM
	printf(" %s ", LSL_Tokens[exp->content.operationValue - LSL_COMMA].token);
#else
	printf(" # ");
#endif
	outputExpression(exp->right);
    }
}

void outputAST(LSL_AST *ast)
{
    if (NULL == ast)
	return;
    switch(ast->type)
    {
#ifdef LUASL_USE_ENUM
	case LSL_COMMENT	: return;
	case LSL_TYPE		: return;
	case LSL_NAME		: return;
	case LSL_IDENTIFIER	: return;
	case LSL_FLOAT		: printf("%f", ast->content.floatValue);		break;
	case LSL_INTEGER	: printf("%d", ast->content.integerValue);		break;
	case LSL_STRING		: return;
	case LSL_KEY		: return;
	case LSL_VECTOR		: return;
	case LSL_ROTATION	: return;
	case LSL_LIST		: return;
	case LSL_LABEL		: return;
#endif
	case LSL_EXPRESSION	: outputExpression(ast->content.expressionValue);	break;
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

void convertAST2Lua(LSL_AST *ast)
{
#ifdef LUASL_USE_ENUM
    if (NULL == ast)
	return;
    switch(ast->type)
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

LSL_AST *newTree(const char *expr)
{
    LuaSL_yyparseParam param;
    YY_BUFFER_STATE state;

#ifdef LUASL_DEBUG
    yydebug= 5;
#endif

    param.ast = NULL;
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

    return param.ast;
}

int main(void)
{
    const char test[] = " 4 + 2 * 10 + 3 * ( 5 + 1 )";
    LSL_AST *ast;

    if ((ast = newTree(test)))
    {
	int result = evaluateAST(ast, 0);

#ifdef LUASL_DEBUG
	printf("\n");
#endif
	printf("Result of '%s' is %d\n", test, result);
	outputAST(ast);
	printf("\n");
	burnAST(ast);
    }

    return 0;
}

