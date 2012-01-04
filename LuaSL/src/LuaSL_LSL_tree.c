/*
 * Implementation of functions used to build the abstract syntax tree.
 */

#include "LuaSL_parser_param.h"
#include "LuaSL_yaccer.tab.h"
#include <stdlib.h>

/**
 * @brief Allocates space for expression
 * @return The expression or NULL if not enough memory
 */
static SExpression* newExpression(EOperationType type, SExpression *left, SExpression *right, int value)
{
    SExpression* b = malloc(sizeof *b);

    if (b == NULL) return NULL;

    b->type = type;
    b->value = value;
    b->left = left;
    b->right = right;

    return b;
}

SExpression* createNumber(int value)
{
    return newExpression(eVALUE, NULL, NULL, value);
}

SExpression *createOperation(EOperationType type, SExpression *left, SExpression *right)
{
    return newExpression(type, left, right, 0);
}

int evaluate(SExpression *e)
{
    switch(e->type)
    {
        case eVALUE:
            return e->value;
        case eMULTIPLY:
            return evaluate(e->left) * evaluate(e->right);
        case ePLUS:
            return evaluate(e->left) + evaluate(e->right);
        default:
            // shouldn't be here
            return 0;
    }
}

void deleteExpression(SExpression *b)
{
    if (b == NULL) return;

    deleteExpression(b->left);
    deleteExpression(b->right);

    free(b);
}

SExpression *getAST(const char *expr)
{
    SParserParam p;
    YY_BUFFER_STATE state;

    p.expression = NULL;
    if (yylex_init(&(p.scanner)))
        return NULL;

    state = yy_scan_string(expr, p.scanner);
    if (yyparse(&p))
        return NULL;

    yy_delete_buffer(state, p.scanner);
    yylex_destroy(p.scanner);
    return p.expression;
}

int yyerror(const char *msg)
{
    fprintf(stderr,"Error:%s\n",msg); return 0;
}

