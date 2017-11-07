#pragma once

#include "Expression.hh"

namespace Syntax {

    struct InsideFor : Expression {
	ExpressionPtr var;
	ExpressionPtr iter;
	ExpressionPtr value;

	InsideFor (Lexical::TokenPtr token, ExpressionPtr var, ExpressionPtr iter, ExpressionPtr value)
	    : Expression (token, AstEnums::INSIDEFOR),
	      var (var),
	      iter (iter),
	      value (value)
	{}
	
	virtual void print (int nb = 0);
    };


};
