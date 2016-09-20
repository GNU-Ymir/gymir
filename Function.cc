#include "Function.hh"

namespace Syntax {

    void Function::print (int nb) {
	printf ("%*c", nb, ' ');
	printf("<Function> ");
	token -> print ();	
	if (type != NULL)
	    type -> print (-1);
	printf ("\n");
	for (auto it : params)
	    if (it != NULL) it -> print (nb + 4);
	if (block != NULL)
	    block -> print (nb + 4);
    }

};
