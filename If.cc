#include "If.hh"

namespace Syntax {
    
    void If::print (int nb) {
	printf("%*c<If> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (test != NULL)
	    test->print (nb + 4);
	if (block != NULL)
	    block -> print (nb + 4);
	if (else_bl != NULL)
	    else_bl -> print (nb + 4);
    }

};
