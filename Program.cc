#include "Program.hh"

namespace Syntax {
    
    void Program::print (int nb) {
	printf("%*c<Program> ", nb, ' ');
	token -> print ();
	printf ("\n");
	for (auto & it : elems) {
	    if (it != NULL)
		it -> print (nb + 4);
	}
    }

};
