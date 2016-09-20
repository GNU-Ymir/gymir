#include "Binary.hh"

namespace Syntax {

    void Binary::print (int nb) {
	printf("%*c<Binary> ", nb, ' ');
	token -> print ();
	printf ("\n");
	if (left != NULL)
	    left->print (nb + 4);
	if (right != NULL)
	    right->print (nb + 4);
    }
       
};
