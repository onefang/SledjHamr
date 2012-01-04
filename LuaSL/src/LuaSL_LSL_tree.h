/*
 * Definition of the structure used to build the abstract syntax tree.
 */
#ifndef __EXPRESSION_H__
#define __EXPRESSION_H__

/**
 * @brief The operation type
 */
typedef enum tagEOperationType
{
    eVALUE,
    eMULTIPLY,
    ePLUS
} EOperationType;

/**
 * @brief The expression structure
 */
typedef struct tagSExpression
{
    EOperationType type;///< type of operation

    int value;///< valid only when type is eVALUE
    struct tagSExpression* left; ///< left side of the tree
    struct tagSExpression* right;///< right side of the tree
} SExpression;

/**
 * @brief It creates an identifier
 * @param value The number value
 * @return The expression or NULL in case of no memory
 */
SExpression* createNumber(int value);

/**
 * @brief It creates an operation
 * @param type The operation type
 * @param left The left operand
 * @param right The right operand
 * @return The expression or NULL in case of no memory
 */
SExpression* createOperation(EOperationType type, SExpression *left, SExpression *right);

/**
 * @brief Deletes a expression
 * @param b The expression
 */
void deleteExpression(SExpression *b);

SExpression *getAST(const char *expr);

int evaluate(SExpression *e);

int yyerror(const char *msg);
int yyparse(void *param);

#endif // __EXPRESSION_H__

