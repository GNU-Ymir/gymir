#include "ast/VarDecl.hh"
#include "ast/Var.hh"

namespace syntax {

    void IVarDecl::print (int nb) {
	printf ("\n%*c<VarDecl> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);

	for (auto it : this-> decls) {
	    it-> print (nb + 4);
	}	    
    }    
    
}
