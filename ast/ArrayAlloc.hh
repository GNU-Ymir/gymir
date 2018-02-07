#pragma once

#include "Expression.hh"
#include "../semantic/types/InfoType.hh"
#include "../syntax/Word.hh"

namespace semantic {
    class IArrayInfo;
    typedef IArrayInfo* ArrayInfo;
}

namespace syntax {

    class IArrayAlloc : public IExpression {

	Expression type;
	
	Expression size;
	
	semantic::InfoType cster;

	bool isImmutable;
	
    public:

	IArrayAlloc (Word token, Expression type, Expression size, bool isImmutable = false);
    
	Expression expression () override;

	Expression staticArray ();
	
	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	Expression onClone () override;
	
	Expression getType ();

	Expression getSize ();
	
	Ymir::Tree toGeneric () override;

	static const char * id () {
	    return TYPEID (IArrayAlloc);
	}
	
	virtual std::vector <std::string> getIds ();
	
	std::string prettyPrint () override;

	void print (int nb = 0) override;

	virtual ~IArrayAlloc ();
	
    private :

	Ymir::Tree staticGeneric (semantic::ArrayInfo info, Ymir::Tree inner, Ymir::Tree array);
	
	Ymir::Tree dynamicGeneric (semantic::ArrayInfo info, Ymir::Tree inner, Ymir::Tree array);

	
    };

    typedef IArrayAlloc* ArrayAlloc;
}
