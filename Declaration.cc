#include "Declaration.hh"

namespace Syntax {

    void Declaration::print (int nb) {
	printf("%*c<Declaration> ", nb, ' ');
	token -> print ();
	printf ("\n");
    }

};
