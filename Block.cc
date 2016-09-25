#include "Block.hh"

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
	
    }

    
};
