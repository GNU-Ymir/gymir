#include "Int.hh"
#include <stdio.h>
#include "IntInfo.hh"

namespace Syntax {
    
    void Int::print (int nb) {
	printf("%*c<Int> ", nb, ' ');
	token -> print ();
	printf ("\n");	
    }

    ExpressionPtr Int::expression () {
	auto aux = new Int (this->token);
	aux-> info = new Semantic::IntInfo ();
	return aux;
    }    
};
