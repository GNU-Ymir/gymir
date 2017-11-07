#include "String.hh"

namespace Syntax {
    
    void String::print (int nb) {
	printf("%*c<String> %s", nb, ' ', content.c_str());
	token -> print ();
	printf ("\n");	
    }

};
