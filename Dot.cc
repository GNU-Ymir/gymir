#include "Dot.hh"

namespace Syntax {

    void Dot::print (int nb) {
	printf("%*c<Dot> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (left != NULL)
	    left -> print (nb + 4);
	if (right != NULL)
	    right -> print (nb + 4);
    }
    
};
