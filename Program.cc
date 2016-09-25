#include "Program.hh"
#include "PureFrames.hh"

namespace Syntax {
    
    void Program::print (int nb) {
	printf("%*c<Program> ", nb, ' ');
	token -> print ();
	printf ("\n");
	for (auto & it : elems) {
	    if (it != NULL)
		it -> print (nb + 4);
	}
    }

    void Program::semantic () {
	for (auto it : elems) {
	    it -> declare ();
	}

	for (auto & it : Semantic::PureFrames::allPure ()) {
	    it.validate ();
	}	
    }

    
};
