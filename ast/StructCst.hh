#pragma once

#include "Expression.hh"
#include "ParamList.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <vector>

namespace semantic {
    class IApplicationScore;
    typedef IApplicationScore* ApplicationScore;
}

namespace syntax {

    class IStructCst : public IExpression {
	
	Word end;

	ParamList params;

	Expression left;

	std::vector <semantic::InfoType> treats;	

	semantic::ApplicationScore score;
	
    public:

	IStructCst (Word word, Word end, Expression left, ParamList params);

	IStructCst (Word word, Word end);
	
	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	Ymir::Tree toGeneric () override;

	std::string prettyPrint () override;
	
	static const char * id () {
	    return TYPEID (IStructCst);
	}
	
	std::vector <std::string> getIds () override {
	    auto ret = IExpression::getIds ();
	    ret.push_back (TYPEID (IStructCst));
	    return ret;
	}

	const std::vector <Expression> & getExprs ();

	Expression getLeft ();
	
    };

    typedef IStructCst* StructCst;

}
