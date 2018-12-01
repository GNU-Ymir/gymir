#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace semantic {
    class IInfoType;
    typedef IInfoType* InfoType;
}

namespace syntax {

    class IUnary : public IExpression {
	Expression elem;
	semantic::InfoType type;
	
    public:

	IUnary (Word word, Expression elem);

	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	Expression getElem ();
	
	bool isLvalue () override;

	virtual std::vector <std::string> getIds () override {
	    auto ids = IExpression::getIds ();
	    ids.push_back (TYPEID (IUnary));
	    return ids;
	}
	
	Ymir::Tree toGeneric () override;

	std::string prettyPrint () override;
	
	void print (int nb = 0) override;

	virtual ~IUnary ();
	
    private :

	Expression findOpUnary (Expression elem);

	bool canOverOpUnary (Expression elem);
	
    };

    typedef IUnary* Unary;

}
