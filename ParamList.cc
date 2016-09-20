#include "ParamList.hh"

namespace Syntax {
    
    void ParamList::print (int nb) {
	printf("%*c<ParamList> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	for (int i = 0; i < (int)params.size (); i++) {
	    if (params[i] != NULL)
		params[i]->print(nb + 4);
	}
    }

};
