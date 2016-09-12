#pragma once

#include "Token.hh"

namespace Syntax {


    struct Ast {	
	Lexical::TokenPtr token;
	Ast (Lexical::TokenPtr token) : token (token) {}				
    };


    struct AstGC {

    public:
	static AstGC & instance () {
	    if (inst == NULL) inst = new AstGC ();
	    return *inst;
	}

    private:

	AstGC () {}
	AstGC (const AstGC &);
	AstGC & operator=(const AstGC &);
	
    private:
	
	static AstGC * inst;
	
    };
    
    typedef Ast* AstPtr;	           

}
