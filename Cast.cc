#include "Cast.hh"

namespace Syntax {
    
    void Cast::print (int nb) {
	printf("%*c<Cast> ", nb, ' ');
	token -> print ();
	printf ("\n");
	if (type != NULL)
	    type->print (nb + 4);
	if (inside != NULL)
	    inside->print (nb + 4);
    }

};
