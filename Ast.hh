#pragma once

#include "Lexer.hh"
#include "Syntax.hh"

#include <gc/gc_cpp.h>

namespace Syntax {
    
    struct Ast : public gc {    
	Lexical::TokenPtr token;
	AstEnums::AstEnum type;
	
	Ast (Lexical::TokenPtr token, AstEnums::AstEnum type);
	virtual void print (int nb = 0) = 0;	
	virtual void semantic () {}	
	virtual ~Ast() {}

    private:
	
	unsigned long id;	
	
    };
    
    typedef Ast* AstPtr;
};
