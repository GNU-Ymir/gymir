#pragma once

#include "Var.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    /**
     * \struct IOfVar
     * The syntaxic node representation of an of var (template of specialisation)
     * \verbatim
     of_var := Identifier ('of' | 'trait') expression
     \endverbatim
     */
    class IOfVar : public IVar {
	
	/** The type (right operand) */
	Expression _type;

	/** This ofvar is a trait test, or just a normal test ? */
	bool _isTrait = false;
	
    public:

	/**
	 * \param ident the location and name of this ofvar
	 * \param type the type of the expression
	 * \param is this ofvar a trait test ?
	 */
	IOfVar (Word ident, Expression type, bool isTrait = false) :
	    IVar (ident),
	    _type (type),
	    _isTrait (isTrait)
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

	std::string prettyPrint () {
	    return IVar::prettyPrint () + std::string (" of ") + this-> _type-> prettyPrint ();
	}
	
	/**
	 * \return the type of the ofVar
	 */
	Expression getTypeVar () {
	    return this-> _type;
	}

	/**
	 * \return is this of var a trait test ?
	 */
	bool isTrait () {
	    return this-> _isTrait;
	}
	
	void print (int nb = 0) override {
	    printf ("\n%*c<OfVar> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	    this-> _type-> print (nb + 4);
	}

	virtual ~IOfVar () {
	    delete this-> _type;
	}
	
    };

    /**
     * \struct IVariadicVar
     * The syntaxic node representation of a variadic var 
     * \verbatim
     variadic_var := Identifier '...' | Identifier ':' '...'
     \endverbatim
     */
    class IVariadicVar : public IVar {

	/** Is this variadic is used as a cte value ? */
	bool _isValue = false;

    public:

	/**
	 * \param ident the location and ident of this variadic var
	 * \param isValue this variadic var is a variadic cte value ?
	 */
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
