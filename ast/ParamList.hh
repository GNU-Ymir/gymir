#pragma once

#include "Expression.hh"
#include <vector>
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    class IParamList : public IExpression {

	std::vector <Expression> params;

    public :
	IParamList (Word ident, std::vector <Expression> params) :
	    IExpression (ident),
	    params (params)
	{}


	std::vector <Expression> getParams () {
	    return this-> params;
	}
	
	void print (int nb = 0) override {
	    printf ("\n%*c<ParamList> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );

	    for (auto it : this-> params) {
		it-> print (nb + 4);
	    }	    
	}
    };

    typedef IParamList* ParamList;
    
}
