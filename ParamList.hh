#pragma once

#include "Expression.hh"

namespace Syntax {
    
    struct ParamList : Expression {	
	std::vector <ExpressionPtr> params;
	ParamList (Lexical::TokenPtr token, std::vector<ExpressionPtr> params)
	    : Expression (token, AstEnums::PARAMLIST),
	      params (params)
	{}
	
	virtual void print (int nb = 0);
    };


};
