#include "Instruction.hh"
#include "Error.hh"

namespace Syntax {
    
    void Instruction::print(int nb) {
	printf("%*c<Instruction> ", nb, ' ');
	token->print();
	printf ("\n");
    }

    Instruction * Instruction::instruction () {
	Ymir::Error::append (this->token->getLocus (),
			     "Erreur interne, pas implemente");
    }
    
};
