#pragma once

#include "ast.hh"
#include <vector>

namespace Syntax {
    struct AstGC {

    public:
	static AstGC & instance () {
	    return inst;
	}

	unsigned long addInfo (Ast*);

	void free (unsigned long id);
	
	void clean ();
	
    private:

	AstGC () {}
	AstGC (const AstGC &);
	AstGC & operator=(const AstGC &);
	
    private:

	std::vector <Ast*> table;
	static AstGC inst;
	
    };
};
