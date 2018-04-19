#pragma once

#include "../syntax/Word.hh"
#include <ymir/utils/memory.hh>
#include <algorithm>
#include <map>
#include <string>

namespace semantic {
    class ISymbol;
    typedef ISymbol* Symbol;
}

namespace Ymir {
    struct Tree;    
}

#define TYPEID(x) #x

//const char* getTypeid 

namespace syntax {
    
    class IBlock;
    typedef IBlock* Block;
    
    class IExpression;
    typedef IExpression* Expression;

    class IInstruction  {
    protected:

	Block parent;	
	bool isStatic;

    public:

	Word token;
	
	IInstruction (Word token) : token (token) {}

	virtual std::vector <semantic::Symbol> allInnerDecls () = 0;
	
	virtual void print (int nb = 0) = 0;

	Block& father () {
	    return this-> parent;
	}
	
	void setStatic (bool is) {
	    this-> isStatic = is;
	}
	
	virtual std::vector <std::string> getIds () {
	    return {TYPEID (IInstruction)};
	}

	virtual IInstruction* instruction () = 0;	   

	virtual IInstruction* templateReplace (const std::map <std::string, Expression>& tmps) = 0;
	
	virtual std::string prettyPrint ();
	
	virtual Ymir::Tree toGeneric ();
		
	template <typename T>
	bool is () {
	    return this-> to<T> () != NULL;
	}

	template <typename T>
	T* to () {	    
	    auto ids = this-> getIds ();
	    if (std::find (ids.begin (), ids.end (), T::id ()) != ids.end ())
		return (T*) this;
	    return NULL;
	}

	virtual ~IInstruction () {	    
	}	
	
    };


    class INone : public IInstruction {
    public :

	INone (Word token) : IInstruction (token) {}

	IInstruction* instruction  () {
	    return this;
	}

	static const char* id () {
	    return "INone";
	}

	Ymir::Tree toGeneric () override;
	
	IInstruction* templateReplace (const std::map <std::string, Expression>&) override {
	    return this;    
	}
	
	std::vector <semantic::Symbol> allInnerDecls () override;

	std::vector <std::string> getIds () override {
	    auto ret = IInstruction::getIds ();
	    ret.push_back (INone::id ());
	    return ret;
	}
	
	void print (int nb = 0) override {
	    printf ("\n%*c<None> %s", nb, ' ', this-> token.toString ().c_str ());
	}
	
	
    };
    
    typedef IInstruction* Instruction;
    
}
