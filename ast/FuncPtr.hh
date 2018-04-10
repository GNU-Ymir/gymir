#pragma once

#include "Expression.hh"
#include <vector>

#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    class IFuncPtr : public IExpression {

	std::vector <Var> params;
	Var ret;
	Expression expr;
	
    public:

	IFuncPtr (Word begin, std::vector <Var> params, Var type, Expression expr = NULL);

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	Expression expression () override;

	std::vector <Var> & getParams ();

	Var getRet ();

	Expression body ();
	
	Ymir::Tree toGeneric () override;
	
	static const char * id ();

	std::vector <std::string> getIds () override;

	std::string prettyPrint () override;
	
	void print (int nb = 0) override;
	
	virtual ~IFuncPtr ();
	
    };

    typedef IFuncPtr* FuncPtr;

}
