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
	
	void setStatic (bool is) {
	    this-> isStatic = is;
	}

	static ulong id () {
	    return 0;
	}
	
	virtual ulong getId () {
	    return IInstruction:: id ();
	};
	
	template <typename T>
	bool is () {
	    return this-> getId () == T::id ();
	}
	
    };


    class INone : public IInstruction {
    public :

	INone (Word token) : IInstruction (token) {}

	void print (int nb = 0) override {
	    printf ("\n%*c<None> %s", nb, ' ', this-> token.toString ().c_str ());
	}
    };
    
    typedef IInstruction* Instruction;
    
}
