#include "ast/VarDecl.hh"
#include "ast/Var.hh"

namespace syntax {

    void IVarDecl::print (int nb) {
	printf ("\n%*c<VarDecl> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);

	for (int it = 0 ; it < this-> decls.size () ; it ++) {
	    
	    this-> decls [it]-> print (nb + 4);
	    if (this-> insts [it]) {
		this-> insts [it]-> print (nb + 6);
	    }
	}	    
    }    
    
}
