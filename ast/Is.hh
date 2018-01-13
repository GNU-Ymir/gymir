#pragma once

#include <ymir/utils/Array.hh>
#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IIs : public IExpression {
	Expression left, type;
	Word expType;

    public:

	IIs (Word begin, Expression expr, Expression type) :
	    IExpression (begin),
	    left (expr),
	    type (type),
	    expType (Word::eof ())
	{
	    this-> left-> inside = this;
	    this-> type-> inside = this;
	}

	IIs (Word begin, Expression expr, Word type) :
	    IExpression (begin),
	    left (expr),
	    type (NULL),
	    expType (type)
	{
	    this-> left-> inside = this;
	}

	Expression templateExpReplace (std::map <std::string, Expression>) override;
	
	void print (int nb = 0) override {
	    printf ("\n%*c<Is> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	    this-> left-> print (nb + 4);
	    if (this-> type) this-> type-> print (nb + 4);
	    else
		printf (": %s", this-> expType.toString ().c_str ());	    
	}
	
    };

    typedef IIs* Is;
   
}
