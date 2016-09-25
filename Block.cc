#include "Block.hh"
#include "Error.hh"
#include "Table.hh"

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
    }
    
};
