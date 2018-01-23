#pragma once

#include "Function.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    class IConstructor : public IFunction {
    public:

	IConstructor (Word token, std::vector <Var> params, Block bl) :
	    IFunction (Word (token.getLocus(), token.getStr () + "__cst__"),
		       params,
		       {}, NULL, bl)	    	    
	{
	    this-> params.insert (this-> params.begin (), new (GC) IVar (token));
	}

	void print (int nb = 0) override {
	    printf ("\n%*c<Constructor> %s",
		    nb, ' ',
		    this-> ident.toString ().c_str ()
	    );
	    
	    for (auto it : this-> params) {
		it-> print (nb + 4);
	    }
	    this-> block-> print (nb + 8);
	}
    };

    typedef IConstructor* Constructor;
}
