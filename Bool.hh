#pragma once

#include "Expression.hh"

namespace Syntax {
    
    struct Bool : Expression {
	Bool (Lexical::TokenPtr token)
	    : Expression (token, AstEnums::BOOL)
	{}
	
	virtual void print (int nb = 0);
    };

};
