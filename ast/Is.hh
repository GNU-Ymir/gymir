#pragma once

#include <ymir/utils/Array.hh>
#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IIs : public IExpression {
	Expression left, type;
	Word expType;

    public:

	IIs (Word begin, Expression expr, Expression type);

	IIs (Word begin, Expression expr, Word type);
	
	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;

	virtual std::vector <std::string> getIds () override {
	    auto ids = IExpression::getIds ();
	    ids.push_back (TYPEID (IIs));
	    return ids;
	}
	
	std::string prettyPrint () override;
	
	virtual ~IIs ();
	
    };

    typedef IIs* Is;
   
}
