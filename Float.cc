#include "Float.hh"

namespace Syntax {
    
    void Float::print (int nb) {
	printf("%*c<Float> ", nb, ' ');
	token -> print ();	
	if (suite == "") {
	    printf("0.%s", token->getCstr ());
	    printf ("\n");
	} else {
	    printf ("%s.%s", token->getCstr (), suite.c_str());
	    printf ("\n");
	}
    }

};
