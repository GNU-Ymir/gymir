#pragma once

#include "Expression.hh"

namespace Syntax {

    struct Int : Expression {
	Int (Lexical::TokenPtr token)
	    : Expression (token, AstEnums::INT)
	{}

	ExpressionPtr expression ();
	
	virtual void print (int nb = 0);

	Ymir::Tree treeExpr () override;
	
    };
    
};
