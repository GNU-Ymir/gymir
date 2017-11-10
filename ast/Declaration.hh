#pragma once

#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <gc/gc_cpp.h>

namespace syntax {

    class IDeclaration : public gc {
    protected :
	
	bool isPublic;

    public:

	bool is_public () {
	    return this-> isPublic;
	}

	void is_public (bool isPublic) {
	    this-> isPublic = isPublic;
	}
	
	virtual void print (int nb = 0) = 0;	
	
    };
        
    typedef IDeclaration* Declaration;

}
