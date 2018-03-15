#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IStringOf : public IExpression {
	Expression expr;

    public:

	IStringOf (Word begin, Expression expr);
	
	Expression templateExpReplace (const std::map <std::string, Expression>&) override;	
	
	Expression expression () override;
		
	virtual ~IStringOf ();

    };

    typedef IStringOf* StringOf;
    
}
