#pragma once

#include "Var.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {


    /**
     * \struct INamedExpression
     * The syntaxic node representation of an named var 
     * \verbatim
     name_expr := Identifier '->' expression
     \endverbatim
     */
    class INamedExpression : public IExpression {

	Expression _value;

    public :

	INamedExpression (Word ident, Expression value) :
	    IExpression (ident),
	    _value (value)
	{}

	Expression getValue () {
	    return this-> _value;
	}

	static const char * id () {
	    return TYPEID (INamedExpression);
	}
	
	std::vector <std::string> getIds () {
	    auto ret = IExpression::getIds ();
	    ret.push_back (TYPEID (INamedExpression));
	    return ret;
	}
	
    };


    typedef INamedExpression* NamedExpression;

}

