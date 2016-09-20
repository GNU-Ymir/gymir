#include "Char.hh"

namespace Syntax {

    void Char::print (int nb) {
	printf("%*c<Char> %i", nb, ' ', value);
	token -> print ();
	printf ("\n");	
    }

};
