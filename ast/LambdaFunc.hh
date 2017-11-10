#pragma once

#include "Expression.hh"
#include "Var.hh"
#include "Block.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class ILambdaFunc : public IExpression {

	std::vector <Var> params;
	Var ret;
	Block block;
	Expression expr;

	
    public:

	ILambdaFunc (Word begin, std::vector <Var> params, Var type, Block block) : 
	    IExpression (begin),
	    params (params),
	    ret (type),
	    block (block)
	{}

	ILambdaFunc (Word begin, std::vector <Var> params, Expression ret) :
	    IExpression (begin),
	    params (params),
	    ret (NULL),
	    block (NULL),
	    expr (ret)
	{}

	void print (int nb = 0) override {
	    printf ("\n%*c<LambdaFunc> %s (",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	    
	    for (auto it : this-> params) {
		it-> print (nb + 4);
	    }
	    printf ("\n%*c ) %s ",
		    nb, ' ',
		    this-> ret ? "->" : "=>"
	    );

	    if (this-> ret) {
		this-> ret-> print (nb + 4);
		this-> block-> print (nb + 8);
	    } else {
		this-> expr-> print (nb + 8);
	    }	    		    
	}
	
    };

    typedef ILambdaFunc* LambdaFunc;
    
}
