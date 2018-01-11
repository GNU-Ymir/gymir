#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IMixin : public IExpression {
	Expression inside;

    public:
	
	IMixin (Word ident, Expression inside)  :
	    IExpression (ident),
	    inside (inside)
	{}

	Expression templateExpReplace (std::map <std::string, Expression>) override;
	
	void print (int nb = 0) override {
	    printf ("\n%*c<Mixin> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	    this-> inside-> print (nb + 4);
	}
	
    };

    typedef IMixin* Mixin;
}
