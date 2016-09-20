#include "Else.hh"

namespace Syntax {

    void Else::print (int nb) {
	printf("%*c<Else> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (block != NULL)
	    block -> print (nb + 4);
    }

};
