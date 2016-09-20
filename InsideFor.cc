#include "InsideFor.hh"

namespace Syntax {
    
    void InsideFor::print (int nb) {
	printf("%*c<InsideFor> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (var != NULL)
	    var -> print (nb + 4);
	if (iter != NULL)
	    iter -> print (nb + 4);
	if (value != NULL)
	    value -> print (nb + 4);
    }

};
