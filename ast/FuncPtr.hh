#pragma once

#include "Expression.hh"
#include <vector>

#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IFuncPtr : public IExpression {

	std::vector <Var> params;
	Var ret;
	Expression expr;
	
    public:

	IFuncPtr (Word begin, std::vector <Var> params, Var type, Expression expr = NULL) :
	    IExpression (begin),
	    params (params),
	    ret (type),
	    expr (expr)
	{
	    this-> ret-> inside = this;
	    if (this-> expr)
		this-> expr-> inside = this;
	}

	void print (int nb = 0) override {
	    printf ("\n%*c<FuncPtr> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	    
	    for (auto it : this-> params) {
		it-> print (nb + 4);
	    }
	    this-> ret-> print (nb + 5);
	    if (this-> expr)
		this->expr-> print (nb + 5);
	}

    };

    typedef IFuncPtr* FuncPtr;

}
