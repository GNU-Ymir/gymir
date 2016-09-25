#pragma once

#include "Lexer.hh"
#include "Syntax.hh"

namespace Syntax {
    
    struct Ast {    
	Lexical::TokenPtr token;
	AstEnums::AstEnum type;
	
	Ast (Lexical::TokenPtr token, AstEnums::AstEnum type);
	virtual void print (int nb = 0) = 0;	
	virtual void semantic () {}

	void free ();
	
	virtual ~Ast() {}

    private:
	
	unsigned long id;	
	
    };
    
    typedef Ast* AstPtr;
};
