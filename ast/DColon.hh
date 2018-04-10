#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IDColon : public IExpression {

	Expression left, right;

    public:

	IDColon (Word token, Expression left, Expression right);

	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	Expression getLeft ();

	Expression getRight ();
	
	Ymir::Tree toGeneric () override;

	static const char * id () {
	    return TYPEID (IDColon);
	}
	
	std::vector <std::string> getIds () {
	    auto ret = IExpression::getIds ();
	    ret.push_back (TYPEID (IDColon));
	    return ret;
	}
	
	void print (int nb = 0) override;

	std::string prettyPrint () override; 
	
	virtual ~IDColon ();

    };
    
    typedef IDColon* DColon;
};
