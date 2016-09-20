#pragma once

#include "Expression.hh"
#include "Var.hh"

namespace Syntax {
    
    struct New : Expression {
	VarPtr type;
	ExpressionPtr size;
	New (Lexical::TokenPtr token, VarPtr type, ExpressionPtr inside)
	    : Expression (token, AstEnums::NEW),
	      type (type),
	      size (inside)
	{}
	
	virtual void print (int nb = 0);
    };    

};
