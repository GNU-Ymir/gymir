#pragma once

#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IFor : public IInstruction {

	Word id;
	std::vector <Var> var;
	Expression iter;
	Block block;
	// InfoType ret;
	
    public:

	IFor (Word token, Word id, std::vector <Var> var, Expression iter, Block bl) :
	    IInstruction (token),
	    id (id),
	    var (var),
	    iter (iter),
	    block (bl)
	{
	    this-> iter-> inside = this;
	}

	void print (int nb = 0) override {
	    printf ("\n%*c<For> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	    for (auto it : this-> var) {
		it-> print (nb + 4);
	    }

	    this-> iter-> print (nb + 5);
	    this-> block-> print (nb + 4);
	}
		
    };

    typedef IFor* For;
    
}
