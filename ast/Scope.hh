#pragma once

#include <ymir/ast/Block.hh>
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    /**
     * \struct IScope
     * A scope exit instruction
     * \verbatim
     scope_exit := 'on' '(' Identifier ')' '=>' block
     \endverbatim
     */
    class IScope : public IInstruction {

	/** The block to execute when the event arrive */
	Block _block;
	
    public:

	/**
	 * \param token the location and type of event
	 * \param block the block to execute when the event arrives
	 */
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
