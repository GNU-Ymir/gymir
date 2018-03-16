#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IDot : public IExpression {
	
	Expression left, right;

    public:

	IDot (Word word, Expression left, Expression right);

	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	Expression getLeft ();

	Ymir::Tree toGeneric () override;

	std::string prettyPrint () override;
	
	static const char * id () {
	    return TYPEID (IDot);
	}
	
	std::vector <std::string> getIds () {
	    auto ret = IExpression::getIds ();
	    ret.push_back (TYPEID (IDot));
	    return ret;
	}
	
	
	void print (int nb = 0) override;

	virtual ~IDot ();
	
    };


    class IDotCall : public IExpression {
    private:

	Expression _call;
	Expression _firstPar;
	bool dyn;
	
    public:

	IDotCall (Instruction inside, Word token, Expression call, Expression firstPar);

	Expression expression () override;
	
	Expression& call ();

	Expression& firstPar ();

	static const char * id () {
	    return TYPEID (IDotCall);
	}
	
	std::vector <std::string> getIds ();
	
	void print (int nb = 0) override;

	virtual ~IDotCall ();
	
    };

    
    typedef IDot* Dot;
}
