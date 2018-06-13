#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class ITypeOf : public IExpression {
	Expression expr;
	bool _mut;
	
    public:

	ITypeOf (Word begin, Expression expr, bool mut = false);
	
	Expression templateExpReplace (const std::map <std::string, Expression>&) override;	
	
	Expression expression () override;

	virtual std::vector <std::string> getIds () override {
	    auto ids = IExpression::getIds ();
	    ids.push_back (TYPEID (ITypeOf));
	    return ids;
	}
	
	std::string prettyPrint () override;
	
	void print (int nb = 0) override;
	
	virtual ~ITypeOf ();

    };

    typedef ITypeOf* TypeOf;
    
}
