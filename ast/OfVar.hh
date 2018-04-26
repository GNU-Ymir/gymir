#pragma once

#include "Var.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IOfVar : public IVar {

	Expression type;

    public:

	IOfVar (Word ident, Expression type) :
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

	Expression typeVar () {
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

	bool _isValue = false;

    public:

	IVariadicVar (Word ident, bool isValue = false) :
	    IVar (ident),
	    _isValue (isValue)
	{}

	static const char * id () {
	    return TYPEID (IVariadicVar);
	}

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	bool& isValue () {
	    return this-> _isValue;
	}
	
	std::vector <std::string> getIds () {
	    auto ret = IVar::getIds ();
	    ret.push_back (TYPEID (IVariadicVar));
	    return ret;
	}

	std::string prettyPrint () {
	    return IVar::prettyPrint () + "...";
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
