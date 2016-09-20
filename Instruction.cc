#include "Instruction.hh"

namespace Syntax {
    
    void Instruction::print(int nb) {
	printf("%*c<Instruction> ", nb, ' ');
	token->print();
	printf ("\n");
    }
    
};
