#include "New.hh"

namespace Syntax {

    void New::print (int nb) {
	printf("%*c<New> ", nb, ' ');
	token -> print ();
	printf ("\n");
	if (type != NULL)
	    type->print (nb + 4);
	if (size != NULL)
	    size->print (nb + 4);
    }

};
