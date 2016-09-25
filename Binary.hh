#pragma once

#include "Expression.hh"

namespace Syntax {
    
    struct Binary : Expression {

	ExpressionPtr left;
	ExpressionPtr right;

	Binary (Lexical::TokenPtr token, ExpressionPtr left, ExpressionPtr right)
	    : Expression (token, AstEnums::BINARY),
	      left (left),
	      right (right),
	      isRight (false)
	{}
	
	virtual void print (int nb = 0);
	
	ExpressionPtr expression ();

    private:

	ExpressionPtr affect ();
	
	ExpressionPtr normal ();

	ExpressionPtr  opUndef (Binary*);
	ExpressionPtr varUndef (ExpressionPtr);

    private:

	bool isRight;
	
    };

    
};
