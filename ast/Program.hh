#pragma once

#include "Declaration.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IProgram : public gc {

	std::vector <Declaration> decls;
	Word locus;

    public:

	IProgram (Word token, std::vector <Declaration> decls) :
	    decls (decls),
	    locus (token)
	{}

	void print (int nb = 0) {
	    printf ("\n%*c<Program> %s",
		    nb, ' ',
		    this-> locus.toString ().c_str ()
	    );
	    
	    for (auto it : this-> decls)
		it-> print (nb + 4);
	    printf ("\n");
	}	
    };

    typedef IProgram* Program;
}
