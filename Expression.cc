#include "Expression.hh"

namespace Syntax {
    
    void Expression::print (int nb) {
	printf("%*c<Expression> ", nb, ' ');
	token -> print ();
	printf ("\n");
    }

};
