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
	Ymir::Error::fatal (this->token->getLocus (),
			     "Erreur interne ~> %s.expression ()",
			     AstEnums::toString(this->type));
	return NULL;
    }

    Ymir::Tree Expression::statement () {
	Ymir::Error::append (this-> token-> getLocus (),
			    "n'est pas une instruction");
	return Ymir::Tree ();
    }

    Ymir::Tree Expression::treeExpr () {
	Ymir::Error::fatal (this->token->getLocus (),
			    "Erreur interne ~> %s.treeExpr ()",
			    AstEnums::toString (this->type));
	return Ymir::Tree ();
    }
    
};
