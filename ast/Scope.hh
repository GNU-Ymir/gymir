#pragma once

#include <ymir/ast/Block.hh>
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IScope : public IInstruction {

	Block block;
	
    public:
	
	IScope (Word token, Block block);

	Instruction instruction () override;

	Instruction templateReplace (const std::map <std::string, Expression> &) override;
	std::vector <semantic::Symbol> allInnerDecls () override;
	
	void print (int) override {}

	static const char * id () {
	    return TYPEID (IScope);
	}
	
	virtual std::vector <std::string> getIds () override {
	    auto ids = IInstruction::getIds ();
	    ids.push_back (TYPEID (IScope));
	    return ids;
	}
	
    };

    typedef IScope* Scope;
}
