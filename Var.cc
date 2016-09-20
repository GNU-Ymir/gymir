#include "Var.hh"

namespace Syntax {
    
    void Var::print (int nb) {
	printf("%*c<Var> ", nb, ' ');
	token -> print ();
	if (templates.size () > 0) {
	    printf ("!(");
	    for (int i = 0; i < (int)templates.size (); i++) {
		if (templates[i] != NULL)
		    templates[i]->print (-1);
		if (i < (int)templates.size () - 1) printf(", ");
	    }
	    printf (")");
	}
	if (nb != -1)
	    printf ("\n");
    }

    void TypedVar::print (int nb) {
	printf("%*c<TypedVar> ", nb, ' ');
	token -> print ();
	if (type != NULL) type->print ();
    }
    

};
