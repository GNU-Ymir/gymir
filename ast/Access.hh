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

	std::vector <semantic::InfoType> treats;	
	
    public:

	IAccess (Word word, Word end, Expression left, ParamList params);
	
	IAccess (Word word, Word end);

	Expression expression () override;
		    
	Expression getLeft ();

	std::vector <Expression> getParams ();

	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;
	    	
    private:
	
	Expression findOpAccess ();
	
    };

    typedef IAccess* Access;

}
