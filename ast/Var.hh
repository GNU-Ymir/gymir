#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <vector>

namespace syntax {

    class IVar : public IExpression {
    protected :
	
	std::vector <Expression> templates;
	Word deco;

    public :

	IVar (Word ident) : IExpression (ident) {}
	IVar (Word ident, Word deco) :
	    IExpression (ident),
	    deco (deco)
	{}

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
