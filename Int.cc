#include "Int.hh"

namespace Syntax {
    
    void Int::print (int nb) {
	printf("%*c<Int> ", nb, ' ');
	token -> print ();
	printf ("\n");	
    }

};
