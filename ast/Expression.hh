#pragma once

#include "Instruction.hh"

#include "../errors/_.hh"
#include "../syntax/Word.hh"
#include <map>

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
	    Ymir::Error::assert ((std::string ("TODO") + this-> getIds ().back ()).c_str ());
	    return NULL;
	}
	
	IInstruction* templateReplace (const std::map <std::string, IExpression*>& elems) final {
	    return this-> templateExpReplace (elems);
	}

	virtual IExpression* templateExpReplace (const std::map <std::string, IExpression*>&);
	
	IExpression* clone ();
	
	virtual IExpression* onClone () {
	    this-> print (0);
	    Ymir::Error::assert ((std::string ("TODO") + this-> getIds ().back ()).c_str ());
	    return NULL;	    
	};
	
	virtual std::vector <std::string> getIds () override {
	    auto ids = IInstruction::getIds ();
	    ids.push_back (TYPEID (IExpression));
	    return ids;
	}
	
	virtual void print (int) override;

	virtual std::string prettyPrint ();
	
    };

    typedef IExpression* Expression;
    
}
