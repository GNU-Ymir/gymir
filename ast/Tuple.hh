#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    class IConstTuple : public IExpression {

	Word end;
	std::vector <Expression> params;

    public:

	IConstTuple (Word word, Word end, std::vector <Expression> params) :
	    IExpression (word),
	    end (end),
	    params (params)
	{}

	void print (int nb = 0) override {
	    printf ("\n%*c<ConstTuple> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );

	    for (auto it : this-> params)
		it-> print (nb + 4);	    
	}
	
    };

    typedef IConstTuple* ConstTuple;
    
}
