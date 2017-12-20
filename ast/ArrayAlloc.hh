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

	Ymir::Tree toGeneric () override;

	static const char * id ();

	const char * getId () override;
	
	void print (int nb = 0) override;
    };

    typedef IArrayAlloc* ArrayAlloc;
}
