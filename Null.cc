#include "Null.hh"

namespace Syntax {
    
    void Null::print (int nb) {
	printf("%*c<Null> ", nb, ' ');
	token -> print ();
	printf ("\n");	
    }

};
