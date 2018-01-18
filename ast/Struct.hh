#pragma once

#include "Declaration.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    class IStruct : public IDeclaration {
	Word ident;
	std::vector<Var> params;
	std::vector<Expression> tmps;

    public:

	IStruct (Word ident, std::vector <Expression> tmps, std::vector <Var> params) :
	    ident (ident),
	    params (params),
	    tmps (tmps)	      
	{
	    this-> isPublic = true;
	}

	void declare () override {}

	void declareAsExtern (semantic::Module) override {}
	
	void print (int nb = 0) override {
	    printf ("\n%*c<Struct> %s",
		    nb, ' ',
		    this-> ident.toString ().c_str ()
	    );
	    printf ("\n%*c!(", nb, ' ');
	    for (auto it : this-> tmps)
		it-> print (nb + 4);
	    printf ("\n%*c)", nb, ' ');

	    printf ("\n%*c{", nb, ' ');
	    for (auto it : this-> params)
		it-> print (nb + 4);
	    printf ("\n%*c}", nb, ' ');	    
	}
	
    };

    typedef IStruct* Struct;

}
