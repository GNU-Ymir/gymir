#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class ICast : public IExpression {

	Expression type;
	Expression expr;
	//Token deco;

    public:

	ICast (Word begin, Expression type, Expression expr);

	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;

	virtual ~ICast ();
	
    };

    typedef ICast* Cast;    
}
