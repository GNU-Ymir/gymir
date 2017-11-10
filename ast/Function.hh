#pragma once

#include "Declaration.hh"
#include "Var.hh"
#include "Expression.hh"
#include "Block.hh"
#include <vector>
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IFunction : public IDeclaration {
    protected :
	
	Word ident;
	Var type;
	std::vector <Var> params;
	std::vector <Expression> tmps;
	Block block;
	Expression test;

    public:
	
	IFunction (Word ident, std::vector <Var> params, std::vector <Expression> tmps, Expression test, Block block) :
	    ident (ident),
	    type (NULL),
	    params (params),
	    tmps (tmps),
	    block (block),
	    test (test)	    
	{}

	
	IFunction (Word ident, Var type, std::vector <Var> params, std::vector <Expression> tmps, Expression test, Block block) :
	    ident (ident),
	    type (type),
	    params (params),
	    tmps (tmps),
	    block (block),
	    test (test)	    
	{}

	void print (int nb = 0) override {
	    printf ("\n%*c<Function> %s ",
		    nb, ' ',
		    this-> ident.toString ().c_str ()
	    );
	    if (this-> type) {
		printf ("\n%*c ->", nb, ' ');
		this-> type-> print (nb + 4);
	    }

	    if (this-> test) {
		printf ("\n%*c if (", nb + 2, ' ');
		this-> test-> print (nb + 4);
		printf ("\n%*c )", nb + 2, ' ');
	    }
	    
	    printf ("\n%*c !(", nb + 2, ' ');
	    for (auto it : this-> tmps) {
		it-> print (nb + 4);
	    }
	    printf ("\n%*c )", nb + 2, ' ');

	    printf ("\n%*c (", nb + 2, ' ');
	    for (auto it : this-> params) {
		it-> print (nb + 4);
	    }
	    printf ("\n%*c )", nb + 2, ' ');

	    this-> block-> print (nb + 6);	    	    
	}
	
    };

    typedef IFunction* Function;
    
}
