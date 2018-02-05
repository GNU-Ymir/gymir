#pragma once

#include "Expression.hh"
#include "../semantic/types/InfoType.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IArrayAlloc : public IExpression {

	Expression type;
	
	Expression size;
	
	semantic::InfoType cster;

    public:

	IArrayAlloc (Word token, Expression type, Expression size);
    
	Expression expression () override;

	Expression staticArray ();
	
	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	Expression getType ();

	Expression getSize ();
	
	Ymir::Tree toGeneric () override;

	static const char * id () {
	    return TYPEID (IArrayAlloc);
	}
	
	virtual std::vector <std::string> getIds ();
	
	void print (int nb = 0) override;


	virtual ~IArrayAlloc ();
	
    };

    typedef IArrayAlloc* ArrayAlloc;
}
