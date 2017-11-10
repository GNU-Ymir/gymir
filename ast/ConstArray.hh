#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IConstArray : public IExpression {
	std::vector <Expression> params;
	//std::vector <InfoType> casters;

    public:

	IConstArray (Word token, std::vector <Expression> params) :
	    IExpression (token),
	    params (params)
	{}

	void print (int nb = 0) override {
	    printf ("\n%*c<ConstArray> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	}	
    };
    
    typedef IConstArray* ConstArray;

}
