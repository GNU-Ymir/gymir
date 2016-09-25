#include "Expression.hh"
#include "config.h"
#include "coretypes.h"
#include "input.h"
#include "diagnostic.h"


namespace Syntax {
    
    void Expression::print (int nb) {
	printf("%*c<Expression> ", nb, ' ');
	token -> print ();
	printf ("\n");
    }

    ExpressionPtr Expression::expression() {
	fatal_error (this->token->getLocus (),
		     "Erreur interne");
    }
    
};
