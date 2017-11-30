#pragma once

#include "Instruction.hh"

#include "../errors/_.hh"
#include "../syntax/Word.hh"


namespace semantic {
    class ISymbol;
    typedef ISymbol* Symbol;

    class IInfoType;
    typedef IInfoType* InfoType;
}

namespace syntax {

    class IExpression : public IInstruction {
    public:
	
	::semantic::Symbol info;
	
	Instruction inside;

	IExpression (Word word) :
	    IInstruction (word)
	{}

	Instruction instruction () {
	    return this-> expression ();
	}
	
	virtual IExpression* expression () {
	    this-> print (0);
	    Ymir::Error::assert ((std::string ("TODO") + this-> getId ()).c_str ());
	}
	
	virtual void print (int) override {}
	
    };

    typedef IExpression* Expression;
    
}
