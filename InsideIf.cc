#include "InsideIf.hh"

namespace Syntax {
    
    void InsideIf::print (int nb) {
	printf("%*c<InsideIf> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (test != NULL)
	    test->print (nb + 4);
	if (if_bl != NULL)
	    if_bl -> print(nb + 4);
	if (else_bl != NULL)
	    else_bl->print (nb + 4);
    }


};
