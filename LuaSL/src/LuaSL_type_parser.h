/*
 * TypeParser.h
 * Definition of the structure used internally by the parser and lexer
 * to exchange data.
 */
 
#ifndef __TYPE_PARSER_H__
#define __TYPE_PARSER_H__
 
#include "LuaSL_LSL_tree.h"
 
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

int yyerror(const char *msg);

 
#endif // __TYPE_PARSER_H__

