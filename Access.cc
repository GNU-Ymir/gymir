#include "Access.hh"

namespace Syntax {    
    void Access::print (int nb) {
	printf("%*c<Access> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (left != NULL)
	    left -> print (nb + 4);
	if (right != NULL)
	    right -> print (nb + 4);
    }
}
