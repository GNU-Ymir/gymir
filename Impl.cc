#include "ast/Impl.hh"
#include "ast/Constructor.hh"

namespace syntax {

    void IImpl::declare () {}
    
    void IImpl::print (int nb) {
	printf ("\n%*c<Impl> %s : %s",
		nb, ' ',
		this-> what.toString ().c_str (),
		this-> who.isEof () ? "object" : this-> who.toString ().c_str ()
	);

	for (auto it : this-> csts) {
	    it-> print (nb + 4);
	}
	    
	for (auto it : this-> methods) {
	    it-> print (nb + 4);
	}	    
    }
    
    
}
