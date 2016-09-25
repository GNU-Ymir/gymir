#include "Expression.hh"
#include "Error.hh"

namespace Syntax {
    
    void Expression::print (int nb) {
	printf("%*c<Expression> ", nb, ' ');
	token -> print ();
	printf ("\n");
    }

    InstructionPtr Expression::instruction () {
	return this-> expression ();
    }
    
    ExpressionPtr Expression::expression() {
	Ymir::Error::append (this->token->getLocus (),
		     "Erreur interne");
    }
    
};
