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
	std::vector <semantic::InfoType> casters;
	
    public:

	IConstTuple (Word word, Word end, std::vector <Expression> params);

	Expression expression () override;

	Ymir::Tree toGeneric () override;
       
	static const char * id () {
	    return TYPEID (IConstTuple);
	}
	
	std::vector <std::string> getIds () override;
	
	void print (int nb = 0) override;
	
    };

    typedef IConstTuple* ConstTuple;
    
}
