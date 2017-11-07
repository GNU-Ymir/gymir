#include "Delete.hh"

namespace Syntax {
    
    void Delete::print (int nb) {
	printf("%*c<Delete> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (elem != NULL)
	    elem -> print (nb + 4);
    }    

};
