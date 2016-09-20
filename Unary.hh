#pragma once

#include "Expression.hh"

namespace Syntax {
    
    struct AfUnary : Expression {
	ExpressionPtr elem;
	AfUnary (Lexical::TokenPtr token, ExpressionPtr elem)
	    : Expression (token, AstEnums::AFUNARY),
	      elem (elem)
	{}
	
	virtual void print (int nb = 0);
    };

    struct BefUnary : Expression {
	ExpressionPtr elem;
	BefUnary (Lexical::TokenPtr token, ExpressionPtr elem)
	    : Expression (token, AstEnums::BEFUNARY),
	      elem (elem)
	{}
	
	virtual void print (int nb = 0);
    };

}
