/*
 * Definition of the structure used to build the abstract syntax tree.
 */
#ifndef __EXPRESSION_H__
#define __EXPRESSION_H__

#ifndef YY_NO_UNISTD_H
#define YY_NO_UNISTD_H 1
#endif // YY_NO_UNISTD_H


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
 * @brief The structure used by flex and bison
 */
typedef union tagTypeParser
{
        SExpression			*expression;
        int				value;
	int				ival;
	float				fval;
	char				*sval;
//	class LLScriptType		*type;
//	class LLScriptConstant		*constant;
//	class LLScriptIdentifier	*identifier;
//	class LLScriptSimpleAssignable	*assignable;
//	class LLScriptGlobalVariable	*global;
//	class LLScriptEvent		*event;
//	class LLScriptEventHandler	*handler;
//	class LLScriptExpression	*expression;
//	class LLScriptStatement		*statement;
//	class LLScriptGlobalFunctions	*global_funcs;
//	class LLScriptFunctionDec	*global_decl;
//	class LLScriptState		*state;
//	class LLScritpGlobalStorage	*global_store;
//	class LLScriptScript		*script;
}STypeParser;
 
// define the type for flex and bison
#define YYSTYPE STypeParser


#ifndef excludeLexer
    #include "LuaSL_lexer.h"
#endif


/**
 * @brief structure given as argument to the reentrant 'yyparse' function.
 */
typedef struct tagSParserParam
{
        yyscan_t scanner;
        SExpression *expression;
}SParserParam;
 
// the parameter name (of the reentrant 'yyparse' function)
// data is a pointer to a 'SParserParam' structure
#define YYPARSE_PARAM data
 
// the argument for the 'yylex' function
#define YYLEX_PARAM   ((SParserParam*)data)->scanner

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

#include "LuaSL_yaccer.tab.h"


#endif // __EXPRESSION_H__

