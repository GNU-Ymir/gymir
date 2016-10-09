#include "Instruction.hh"
#include "Error.hh"

namespace Syntax {
    
    void Instruction::print(int nb) {
	printf("%*c<Instruction> ", nb, ' ');
	token->print();
	printf ("\n");
    }

    Instruction * Instruction::instruction () {
	Ymir::Error::fatal (this->token->getLocus (),
			    "Erreur interne ~> %s.instruction ()",
			     AstEnums::toString(this->type));
	return NULL;
    }

    Ymir::Tree Instruction::statement () {
	Ymir::Error::fatal (this->token->getLocus (),
			    "Erreur interne ~> %s.statement ()",
			    AstEnums::toString(this->type));
	return Ymir::Tree ();
    }
    
};
