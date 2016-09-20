#pragma once

#include "Expression.hh"

namespace Syntax {

    struct Par : Expression {
	ExpressionPtr left;
	ExpressionPtr right;
	Par (Lexical::TokenPtr token, ExpressionPtr left, ExpressionPtr right)
	    : Expression (token, AstEnums::PAR),
	      left (left),
	      right (right)
	{}
	
	virtual void print (int nb = 0);
    };

};
