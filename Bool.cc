#include "Bool.hh"

namespace Syntax {
    
    void Bool::print (int nb) {
	printf("%*c<Bool> ", nb, ' ');
	token -> print ();
	printf ("\n");	
    }
    
};
