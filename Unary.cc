#include "Unary.hh"

namespace Syntax {

    void AfUnary::print (int nb) {
	printf("%*c<AfUnary> ", nb, ' ');
	token -> print ();
	printf ("\n");
	if (elem != NULL)
	    elem->print (nb + 4);
    }

    void BefUnary::print (int nb) {
	printf("%*c<BefUnary> ", nb, ' ');
	token -> print ();
	printf ("\n");
	if (elem != NULL)
	    elem->print (nb + 4);
    }

};
