#pragma once

#include "Expression.hh"

namespace Syntax {

    struct Access : Expression {
	ExpressionPtr left;
	ExpressionPtr right;
	Access (Lexical::TokenPtr token, ExpressionPtr left, ExpressionPtr right)
	    : Expression (token, AstEnums::ACCESS),
	      left (left),
	      right (right)
	{}
	
	virtual void print (int nb = 0);
    };    


};
