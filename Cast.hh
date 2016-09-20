#pragma once

#include "Expression.hh"
#include "Var.hh"

namespace Syntax {
            
    struct Cast : Expression {
	VarPtr type;
	ExpressionPtr inside;
	Cast (Lexical::TokenPtr token, VarPtr type, ExpressionPtr inside)
	    : Expression (token, AstEnums::CAST),
	      type (type),
	      inside (inside)
	{}
	
	virtual void print (int nb = 0);
    };

};
