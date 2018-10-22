#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    typedef unsigned char ubyte;
    
    class IConstRange : public IExpression {
	Expression left, right;
	bool _inner = false;
	//Semantic::InfoType content;
	ubyte lorr = 0;
	//Semantic::InfoType caster = NULL;

    public:
	
	IConstRange (Word token, Expression left, Expression right, bool inner = false);

	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	void print (int nb = 0) override;

	virtual std::vector <std::string> getIds () override {
	    auto ids = IExpression::getIds ();
	    ids.push_back (TYPEID (IConstRange));
	    return ids;
	}
	
	virtual ~IConstRange ();
	
    private:

	Expression findOpRange (IConstRange*);

    };

    typedef IConstRange* ConstRange;
}
