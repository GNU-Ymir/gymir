#pragma once

#include "Expression.hh"

namespace Syntax {
    
    struct String : Expression {
	std::string content;
	String (Lexical::TokenPtr token, std::string content)
	    : Expression (token, AstEnums::STRING),
	      content (content)
	{}

	virtual void print (int nb = 0);
    };
    
};
