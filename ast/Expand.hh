#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IExpand : public IExpression {
	Expression expr;
	ulong it;

	static std::map <syntax::Expression, Ymir::Tree> __values__;
	
    public :

	IExpand (Word begin, Expression expr);

	IExpand (Word begin, Expression expr, ulong it);
	
	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;

	virtual ~IExpand ();
	
    };

    typedef IExpand* Expand;
    
}
