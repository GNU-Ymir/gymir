#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IDColon : public IExpression {

	Expression left, right;

    public:

	IDColon (Word token, Expression left, Expression right);

	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	void print (int nb = 0) override;

	virtual ~IDColon ();

    };
    
};
