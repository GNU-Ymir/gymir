#pragma once

#include "Expression.hh"

namespace Syntax {
    
    struct Char : Expression {
	short value;
	Char (Lexical::TokenPtr token, short value)
	    : Expression (token, AstEnums::CHAR),
	      value (value)
	{}
	
	virtual void print (int nb = 0);
    };
   
};
