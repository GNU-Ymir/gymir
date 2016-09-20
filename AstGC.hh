#pragma once

#include "ast.hh"

namespace Syntax {
    struct AstGC {

    public:
	static AstGC & instance () {
	    return inst;
	}

	void addInfo (Ast*);
	void clean ();
	
    private:

	AstGC () {}
	AstGC (const AstGC &);
	AstGC & operator=(const AstGC &);
	
    private:

	std::list <Ast*> table;
	static AstGC inst;
	
    };
};
