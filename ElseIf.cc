#include "ElseIf.hh"

namespace Syntax {

    void ElseIf::print (int nb) {
	printf("%*c<Else> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (if_bl != NULL)
	    if_bl -> print (nb + 4);
    }

};
