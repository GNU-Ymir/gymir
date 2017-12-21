#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IExpand : public IExpression {
	Expression expr;

    public :

	IExpand (Word begin, Expression expr);

	Expression expression () override;

	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;		
    };

    typedef IExpand* Expand;
    
}
