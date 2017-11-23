#pragma once

#include "Var.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class ITypedVar : public IVar {

	Var type;
	Expression expType;

    public:

	ITypedVar (Word ident, Var type) :
	    IVar (ident),
	    type (type)
	{}

	ITypedVar (Word ident, Var type, Word deco) :
	    IVar (ident),
	    type (type)
	{
	    this-> deco = deco;
	}

	ITypedVar (Word ident, Expression type) :
	    IVar (ident),
	    expType (type)
	{}

	ITypedVar (Word ident, Expression type, Word deco) :
	    IVar (ident),
	    expType (type)
	{
	    this-> deco = deco;
	}
	
	static const char * id () {
	    return "ITypedVar";
	}

	const char * getId () override {
	    return ITypedVar::id ();
	}

	void print (int nb = 0) override {
	    printf ("\n%*c<TypedVar> %s%s",
		    nb, ' ',
		    this-> deco.isEof () ? "" : this-> deco.getStr ().c_str (),
		    this-> token.toString ().c_str ()
	    );
	    if (this-> type) 
		this-> type-> print (nb + 4);
	    else this-> expType-> print (nb + 4);
	}
	
    };

    typedef ITypedVar* TypedVar;
    
}
