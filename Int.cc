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
	aux-> sym = Semantic::SymbolPtr (new Semantic::Symbol (aux-> token, new Semantic::IntInfo ()));
	return aux;
    }    
};
