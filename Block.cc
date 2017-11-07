#include "Block.hh"
#include "Error.hh"
#include "Table.hh"
#include <iostream>

namespace Syntax {

    void Block::print (int nb) {
	printf("%*c<Block> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	for (auto & it : decls) {
	    if (it != NULL)
		it -> print (nb + 4);
	}
	for (auto & it : instructions) {
	    if (it != NULL)
		it -> print (nb + 4);
	}	
    }    

    Instruction * Block::instruction () {	
	for (auto it : this->decls) {
	    it->declare ();
	}

	std::vector<InstructionPtr> insts;
	for (auto it : this->instructions) {	    
	    if (Semantic::Table::instance ().alreadyReturned ())
		Ymir::Error::append (it->token->getLocus(),
			     "Instruction non-atteignable");
	    else 
		insts.push_back (it->instruction ());				
	}
	return new Block (this-> token, {}, insts);
    }

    Ymir::Tree Block::toGeneric () {
	Ymir::enterBlock ();
	for (auto it : this-> instructions) {
	    Ymir::getStackStmtList ().back ().append (it-> statement ());
	}
	auto treeBlock = Ymir::leaveBlock ();
	return treeBlock.block;
    }
    
};
