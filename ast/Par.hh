#pragma once

#include "Expression.hh"
#include "ParamList.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IPar : public IExpression {
	Word end;
	ParamList params;
	Expression left;
	//DotCall dotCall;
	bool opCall = false;
	// ApplicationScore score;

    public :

	IPar (Word word, Word end, Expression left, ParamList params, bool fromOpCall = false) :
	    IExpression (word),
	    end (end),
	    params (params),
	    left (left),
	    opCall (fromOpCall)
	{
	    this-> left-> inside = this;
	    this-> params-> inside = this;	    	    
	}
	
	void print (int nb = 0) override {
	    printf ("\n%*c<Par> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );

	    this-> left-> print (nb + 4);
	    this-> params-> print (nb + 4);
	}
	
    };

    typedef IPar* Par;    
}
