#include "While.hh"

namespace Syntax {

    void While::print (int nb) {
	printf("%*c<While> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (test != NULL)
	    test -> print (nb + 4);
	if (block != NULL)
	    block -> print (nb + 4);
    }

};
