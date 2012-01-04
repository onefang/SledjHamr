/*
 * ParserParam.h
 * Definitions of the parameters for the reentrant functions
 * of flex (yylex) and bison (yyparse)
 */

// Since in this sample we use the reentrant version of both flex and yacc we are forced to provide parameters for the yylex function, when called from yyparse.
 
#ifndef __PARSERPARAM_H__
#define __PARSERPARAM_H__
 
#ifndef YY_NO_UNISTD_H
#define YY_NO_UNISTD_H 1
#endif // YY_NO_UNISTD_H
 
#include "LuaSL_type_parser.h"
#include "LuaSL_lexer.h"
#include "LuaSL_LSL_tree.h"
 
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
 
#endif // __PARSERPARAM_H__


