#pragma once

#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IReturn : public IInstruction {

	Expression elem;
	semantic::InfoType caster;
	bool _isUseless = false;
	
    public:

	IReturn (Word ident);
	
	IReturn (Word ident, Expression elem);

	Expression getExpr ();
	
	Instruction instruction () override;

	Instruction templateReplace (const std::map <std::string, Expression>&) override;

	std::vector <semantic::Symbol> allInnerDecls () override;
	
	semantic::InfoType& getCaster ();
	
	Ymir::Tree toGeneric () override;

	bool& isUseless ();
	
	void print (int nb = 0) override;

	static std::string id () {
	    return TYPEID (IReturn);
	}
	
	virtual std::vector <std::string> getIds () {
	    auto ret = IInstruction::getIds ();
	    ret.push_back (TYPEID (IReturn));
	    return ret;
	}
	
	virtual ~IReturn ();
	
    };
    
}
