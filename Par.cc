#include "Par.hh"

namespace Syntax {

    void Par::print (int nb) {
	printf("%*c<Par> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (left != NULL)
	    left -> print (nb + 4);
	if (right != NULL)
	    right -> print (nb + 4);
    }

};
