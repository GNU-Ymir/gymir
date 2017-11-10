#include "ast/Block.hh"
#include "ast/Declaration.hh"

namespace syntax {

    void IBlock::print (int nb) {
	printf ("\n%*c<Block> : %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	
	for (auto it : this-> decls)
	    it-> print (nb + 4);
	
	for (auto it : this-> insts)
	    it-> print (nb + 4);	
    }
    
}
