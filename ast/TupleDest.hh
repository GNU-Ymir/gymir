#pragma once

#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    class ITupleDest : public IInstruction {

	std::vector <Var> decls;
	std::vector <Expression> insts;
	Expression right;
	bool isVariadic;

    public:

	ITupleDest (Word token, bool isVariadic, std::vector <Var> decls, Expression right) :
	    IInstruction (token),
	    decls (decls),
	    right (right),
	    isVariadic (isVariadic)	    
	{}

	ITupleDest (Word token, std::vector <Expression> insts, Expression right) :
	    IInstruction (token),
	    insts (insts),
	    right (right),
	    isVariadic (false)
	{}

	Instruction instruction () override {
	    Ymir::Error::assert ("TODO");
	    return NULL;
	}

	Instruction templateReplace (const std::map <std::string, Expression>&) override;	
	
	void print (int nb = 0) override {
	    printf ("\n%*c<TupleDest> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );

	    for (auto it : this-> decls) {
		it-> print (nb + 4);		
	    }

	    for (auto it : this-> insts) {
		it-> print (nb + 4);
	    }

	    this-> right-> print (nb + 4);	    
	}

	virtual ~ITupleDest () {
	    for (auto it : decls)
		delete it;
	    for (auto it : insts)
		delete it;
	    if (right) delete right;
	}
	
    };

    typedef ITupleDest* TupleDest;    
}
