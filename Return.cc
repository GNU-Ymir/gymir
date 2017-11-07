#include "Return.hh"

namespace Syntax {

    void Return::print (int nb) {
	printf("%*c<Return> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (ret != NULL)
	    ret -> print (nb + 4);
    }

};
