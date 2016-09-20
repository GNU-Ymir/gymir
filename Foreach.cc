#include "Foreach.hh"

namespace Syntax {

    void Foreach::print (int nb) {
	printf("%*c<Foreach> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if(var != NULL) var -> print (nb + 4);
	if (iter != NULL) iter->print (nb + 4);
	if (block != NULL) block -> print (nb + 4);
    }

};
