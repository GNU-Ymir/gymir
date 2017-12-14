#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IDot : public IExpression {
	
	Expression left, right;

    public:

	IDot (Word word, Expression left, Expression right);

	Expression expression () override;

	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;
		
    };


    class IDotCall : public IExpression {
    private:

	Expression _call;
	Expression _firstPar;
	bool dyn;
	
    public:

	IDotCall (Instruction inside, Word token, Expression call, Expression firstPar);

	Expression expression () override;

	Expression& call ();

	Expression& firstPar ();
	
	void print (int nb = 0) override;
	
    };

    
    typedef IDot* Dot;
}
