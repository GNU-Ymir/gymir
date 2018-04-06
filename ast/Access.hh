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

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	Expression getLeft ();

	std::vector <Expression> getParams ();

	Ymir::Tree toGeneric () override;

	bool isLvalue () override;
	
	void print (int nb = 0) override;

	std::string prettyPrint () override;
	
	static const char * id () {
	    return TYPEID (IAccess);
	}
	
	std::vector <std::string> getIds () override {
	    auto ret = IExpression::getIds ();
	    ret.push_back (TYPEID (IAccess));
	    return ret;
	}

	virtual ~IAccess ();
	
    private:
	
	Expression findOpAccess ();
	
    };

    typedef IAccess* Access;

}
