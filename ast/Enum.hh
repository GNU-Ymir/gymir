#pragma once

#include "Declaration.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    class IEnum : public IDeclaration {
	Word ident;
	Var type;
	std::vector <Word> names;
	std::vector <Expression> values;

    public:

	IEnum (Word ident, Var type, std::vector <Word> names, std::vector <Expression> values) :
	    ident (ident),
	    type (type),
	    names (names),
	    values (values)
	{}

	void declare () override {}
	
	void print (int nb = 0) override {
	    printf ("\n%*c<Enum> %s",
		    nb, ' ',
		    this-> ident.toString ().c_str ()
	    );

	    for (int i = 0 ; i < (int) this-> names.size (); i++) {
		printf ("\n%*c%s ", nb + 4, ' ',
			this-> names [i].toString ().c_str ());
		
		this-> values [i]-> print (nb + 8);
	    }
	}
	
    };

    typedef IEnum* Enum;
    
}
