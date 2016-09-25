#pragma once

#include "Token.hh"
#include <list>
#include <cstdlib>

namespace Syntax {

    namespace AstEnums {
	enum AstEnum {
	    AST,
	    INSTRUCTION,
	    EXPRESSION,
	    DECLARATION,
	    PROGRAM,
	    VAR,
	    TYPEDVAR,
	    TYPE,
	    BINARY,
	    AFUNARY,
	    BEFUNARY,
	    CAST,
	    NEW,
	    INSIDEIF,
	    INSIDEFOR,
	    INT,
	    FLOAT,
	    BOOL,
	    NULL_,
	    CHAR,
	    STRING,
	    PARAMLIST,
	    PAR,
	    ACCESS,
	    DOT,
	    BLOCK,
	    IF,
	    ELSEIF,
	    ELSE,
	    RETURN,
	    FOR,
	    FOREACH,
	    WHILE,
	    BREAK,
	    DELETE,
	    FUNCTION,	
	};
    };
         
};

