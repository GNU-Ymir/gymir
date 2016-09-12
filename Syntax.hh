#pragma once

#include "Token.hh"
#include <list>

namespace Syntax {

    
    struct Ast {	
	Lexical::TokenPtr token;
	Ast (Lexical::TokenPtr token);
    };

    struct AstGC {

    public:
	static AstGC & instance () {
	    if (inst == NULL) inst = new AstGC ();
	    return *inst;
	}

	void addInfo (Ast*);
	void empty ();
	
    private:

	AstGC () {}
	AstGC (const AstGC &);
	AstGC & operator=(const AstGC &);
	
    private:

	std::list <Ast*> table;
	static AstGC * inst;
	
    };
    
    typedef Ast* AstPtr;	           

}
