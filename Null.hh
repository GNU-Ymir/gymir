#pragma once

#include "Expression.hh"

namespace Syntax {

    struct Null : Expression {
	Null (Lexical::TokenPtr token)
	    : Expression (token, AstEnums::NULL_)
	{}
	
	virtual void print (int nb = 0);
    };

};
