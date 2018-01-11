#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    typedef unsigned char ubyte;
    
    class IConstRange : public IExpression {
	Expression left, right;
	//Semantic::InfoType content;
	ubyte lorr = 0;
	//Semantic::InfoType caster = NULL;

    public:
	
	IConstRange (Word token, Expression left, Expression right);

	Expression expression () override;

	Expression templateExpReplace (std::map <std::string, Expression>) override;
	
	void print (int nb = 0) override;
	
    private:

	Expression findOpRange (IConstRange*);

    };

    typedef IConstRange* ConstRange;
}
