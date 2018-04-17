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

	bool isLvalue () override;
	
	Ymir::Tree toGeneric () override;

	std::string prettyPrint () override;
	
	void print (int nb = 0) override;

	virtual ~IUnary ();
	
    };

    typedef IUnary* Unary;

}
