#pragma once

#include "Expression.hh"
#include "Var.hh"

namespace Syntax {
        
    struct Dot : Expression {	
	ExpressionPtr left;
	VarPtr right;
	Dot (Lexical::TokenPtr token, ExpressionPtr left, VarPtr right)
	    : Expression (token, AstEnums::DOT),
	      left (left),
	      right (right)
	{}
	
	virtual void print (int nb = 0);
    };

};
