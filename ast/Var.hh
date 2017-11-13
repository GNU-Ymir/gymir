#pragma once

#include "Expression.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

#include <vector>

namespace syntax {

    class IVar : public IExpression {
    protected :
	
	std::vector <Expression> templates;

    public :

	Word deco;

	IVar (Word ident) : IExpression (ident) {}
	IVar (Word ident, Word deco) :
	    IExpression (ident),
	    deco (deco)
	{}

	IVar (Word ident, std::vector <Expression> tmps) :
	    IExpression (ident),
	    templates (tmps)
	{}


	static ulong id () {
	    return 2;
	}
	
	ulong getId () override {
	    return IVar::id ();
	}
	
	void print (int nb = 0) override {
	    printf ("\n%*c<Var> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	    
	    for(auto it : this-> templates) {
		it-> print (nb + 4);
	    }
	    
	}	
    };

    class IArrayVar : public IVar {
	Expression content;

    public: 
	IArrayVar (Word token, Expression content) :
	    IVar (token),
	    content (content)
	{}
	
	void print (int nb = 0) override {
	    printf ("\n%*c <ArrayVar> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
		    
	}
	
    };
    
    typedef IVar* Var;    
    typedef IArrayVar* ArrayVar;
}
