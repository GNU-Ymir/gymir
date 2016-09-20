#include "For.hh"

namespace Syntax {
    
    void For::print (int nb) {
	printf("%*c<For> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	for (auto & it : inits) {
	    if (it != NULL)
		it -> print (nb + 4);
	}
	if (test != NULL)
	    test -> print (nb + 4);
	for (auto & it : iter)
	    if (it != NULL) it -> print (nb + 4);
	if (block != NULL)
	    block -> print(nb + 4);
    }

};
