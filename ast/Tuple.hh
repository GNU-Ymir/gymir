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

	IConstTuple (Word word, Word end, const std::vector <Expression>& params);

	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;	
	
	Ymir::Tree toGeneric () override;
       
	static const char * id () {
	    return TYPEID (IConstTuple);
	}
	
	std::vector <std::string> getIds () override;
	
	std::vector <Expression> & getExprs ();

	std::vector <semantic::InfoType> & getCasters ();

	std::string prettyPrint () override;
	
	void print (int nb = 0) override;

	virtual ~IConstTuple ();
	
    };

    typedef IConstTuple* ConstTuple;
    
}
