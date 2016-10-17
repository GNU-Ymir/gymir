#pragma once

#include "Token.hh"
#include <list>
#include <cstdlib>
	
#define AST_ENUM_LIST				\
    AST_ENUM (AST, "Ast")			\
    AST_ENUM (INSTRUCTION, "Instruction")	\
    AST_ENUM (EXPRESSION, "Expression")		\
    AST_ENUM (DECLARATION, "Declaration")	\
    AST_ENUM (PROGRAM, "Program")		\
    AST_ENUM (VAR, "Var")			\
    AST_ENUM (TYPEDVAR, "TypedVar")		\
    AST_ENUM (TYPE, "Type")			\
    AST_ENUM (BINARY, "Binary")			\
    AST_ENUM (AFUNARY, "AfUnary")		\
    AST_ENUM (BEFUNARY, "BefUnary")		\
    AST_ENUM (CAST, "Cast")			\
    AST_ENUM (NEW, "New")			\
    AST_ENUM (INSIDEIF, "InsideIf")		\
    AST_ENUM (INSIDEFOR, "InsideFor")		\
    AST_ENUM (INT, "Int")			\
    AST_ENUM (FLOAT, "Float")			\
    AST_ENUM (BOOL, "Bool")			\
    AST_ENUM (NULL_, "Null")			\
    AST_ENUM (CHAR, "Char")			\
    AST_ENUM (STRING, "String")			\
    AST_ENUM (PARAMLIST, "ParamList")		\
    AST_ENUM (PAR, "Par")			\
    AST_ENUM (ACCESS, "Access")			\
    AST_ENUM (DOT, "Dot")			\
    AST_ENUM (BLOCK, "Block")			\
    AST_ENUM (IF, "If")				\
    AST_ENUM (ELSEIF, "ElseIf")			\
    AST_ENUM (ELSE, "Else")			\
    AST_ENUM (RETURN, "Return")			\
    AST_ENUM (FOR, "For")			\
    AST_ENUM (FOREACH, "ForEach")		\
    AST_ENUM (WHILE, "While")			\
    AST_ENUM (BREAK, "Break")			\
    AST_ENUM (DELETE, "Delete")			\
    AST_ENUM (FUNCTION, "Function")		\
    AST_ENUM (VAR_DECL, "VarDecl")			



namespace Syntax {

    namespace AstEnums {
	
	enum AstEnum {
#define AST_ENUM(name, _) name,
#define AST_ENUM_KEYWORD(x, y) AST_ENUM(x, y)
	    AST_ENUM_LIST
#undef AST_ENUM
#undef AST_ENUM_KEYWORD
	};
	
	const char * toString (AstEnum elem);	
	
    };
    
};

