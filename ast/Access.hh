#pragma once

#include "Expression.hh"
#include "ParamList.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <vector>

namespace syntax {

    class IAccess : public IExpression {
	
	Word end;

	ParamList params;

	Expression left;

    public:

	IAccess (Word word, Word end, Expression left, ParamList params) :
	    IExpression (word),
	    end (end),
	    params (params),
	    left (left)	    
	{
	    this-> left-> inside = this;
	    this-> params-> inside = this;
	}
	
	IAccess (Word word, Word end) :
	    IExpression (word),
	    end (end)
	{
	}

	Expression expression () override;


	
	// Expression templateExpReplace (std::map<std::string, Expression> values)
	//     override {
	//     auto params = this-> params.templateExpReplace (values);
	//     auto left = this-> left-> templateExpReplace (values);
	//     return new IAccess (this-> token, this-> end, left, params);
	// }
	    
	Expression getLeft () {
	    return this-> left;
	}

	std::vector <Expression> getParams () {
	    return this-> params-> getParams ();
	}
	    
	void print (int nb = 0) override {
	    printf ("\n%*c<Access> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	    this-> left-> print (nb + 4);
	    this-> params-> print (nb + 4);
	}
	    
    protected:
	
	// Expression onClone () override {
	//     return new Access (this-> token,
	// 		       this-> end,
	// 		       this-> left-> clone (),
	// 		       (ParamList) this-> params-> clone ()
	//     );
	// }	
	
    private:
	
	Expression findOpAccess ();
	
    };

    typedef IAccess* Access;

}
