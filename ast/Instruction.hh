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

	Block parent;	
	bool isStatic;

    public:

	Word token;
	
	IInstruction (Word token) : token (token) {}

	virtual void print (int nb = 0) = 0;
	
	void setStatic (bool is) {
	    this-> isStatic = is;
	}

	static const char* id () {
	    return "IInstruction";
	}
	
	virtual const char* getId () {
	    return IInstruction:: id ();
	};

	template <typename T>
	bool is () {
	    return strcmp (this-> getId (), T::id ()) == 0;
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
