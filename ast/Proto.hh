#pragma once

#include "Declaration.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IProto : IDeclaration {

	Word ident;
	Var type;
	std::vector <Var> params;
	std::string space;
	bool isVariadic;
	
    public:
	
	IProto (Word ident, std::vector <Var> params, bool isVariadic) :
	    ident (ident),
	    type (NULL),
	    params (params),
	    space (""),
	    isVariadic (isVariadic)
	{}

	IProto (Word ident, Var type, std::vector <Var> params, std::string space, bool isVariadic) :
	    ident (ident),
	    type (type),
	    params (params),
	    space (space),
	    isVariadic (isVariadic)
	{}

	void print (int nb = 0) override {
	    printf ("\n%*c<Proto> %s%s",
		    nb, ' ',
		    this-> space.c_str (),
		    this-> ident.toString ().c_str ()
	    );
	    
	    for (auto it : this-> params) {
		it-> print (nb + 4);
	    }

	    if (this-> type)
		this-> type-> print (nb + 6);
	}
	
    };

    typedef IProto* Proto;
}
