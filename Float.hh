#pragma once

#include "Expression.hh"

namespace Syntax {

    struct Float : Expression {
	std::string suite;
	
	Float (Lexical::TokenPtr token)
	    : Expression (token, AstEnums::FLOAT)
	{}
	
	Float (Lexical::TokenPtr token, std::string suite)
	    : Expression (token, AstEnums::FLOAT),
	      suite (suite)
	{}
	
	virtual void print (int nb = 0);
	
    };
    
};
