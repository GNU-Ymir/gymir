#pragma once

#include "Var.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IOfVar : public IVar {

	Var type;

    public:

	IOfVar (Word ident, Var type) :
	    IVar (ident),
	    type (type)
	{}

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	static const char * id () {
	    return TYPEID (IOfVar);
	}
	
	std::vector <std::string> getIds () {
	    auto ret = IVar::getIds ();
	    ret.push_back (TYPEID (IOfVar));
	    return ret;
	}

	Var typeVar () {
	    return this-> type;
	}
	
	void print (int nb = 0) override {
	    printf ("\n%*c<OfVar> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	    this-> type-> print (nb + 4);
	}

	virtual ~IOfVar () {
	    delete type;
	}
	
    };


    class IVariadicVar : public IVar {
    public:

	IVariadicVar (Word ident) : IVar (ident) {}

	static const char * id () {
	    return TYPEID (IVariadicVar);
	}
	
	std::vector <std::string> getIds () {
	    auto ret = IVar::getIds ();
	    ret.push_back (TYPEID (IVariadicVar));
	    return ret;
	}
	
	void print (int nb = 0) override {
	    printf ("\n%*c<VariadicVar> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	}
    };

    typedef IOfVar* OfVar;
    typedef IVariadicVar* VariadicVar;
        
}
