#include "Break.hh"

namespace Syntax {
    
    void Break::print (int nb) {
	printf("%*c<Break> ", nb, ' ');
	token -> print ();
	printf ("\n");	
    }

};
