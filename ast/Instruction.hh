#pragma once

#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <gc/gc_cpp.h>

namespace syntax {
    
    class IBlock;
    typedef IBlock* Block;
    
    class IInstruction : public gc {
    protected:
	Word token;
	Block parent;	
	bool isStatic;

    public:

	IInstruction (Word token) : token (token) {}

	virtual void print (int nb = 0) = 0;
	
    };

    typedef IInstruction* Instruction;
    
}
