#pragma once

#include "Expression.hh"

namespace Syntax {
    
    struct InsideIf : Expression {
	ExpressionPtr test;
	ExpressionPtr if_bl;
	ExpressionPtr else_bl;

	InsideIf (Lexical::TokenPtr token, ExpressionPtr test, ExpressionPtr if_bl, ExpressionPtr else_bl)
	    : Expression (token, AstEnums::INSIDEIF),
	      test (test),
	      if_bl (if_bl),
	      else_bl (else_bl)
	{}
	
	virtual void print (int nb = 0);
    };
    
}
