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

namespace syntax {
    
    class IBlock;
    typedef IBlock* Block;
    
    class IExpression;
    typedef IExpression* Expression;

    /**
     * \struct IInstruction
     * Parent of all kind of statement
     */
    class IInstruction  {
    protected:

	/** The block of instruction where this instruction can be found */
	Block _parent;

	/** Is this instruction a cte one ?*/
	bool _isStatic;

    public:

	/** The location of the instruction */
	Word token;
	
	/**
	 * \param token the location of the instruction
	 */
	IInstruction (Word token) : token (token) {}

	/**
	 * \return the symbol of all the declared variables inside this instruction
	 */
	virtual std::vector <semantic::Symbol> allInnerDecls () = 0;

	/**
	 * \brief print the tree of this instruction to the console
	 * \param nb the current tabulation
	 * \deprecated only used for debugging purposes
	 */
	virtual void print (int nb = 0) = 0;

	/**
	 * \return get or set the parent block of this instruction
	 */
	Block& father () {
	    return this-> _parent;
	}

	/**
	 * \param is is this instruction a cte one ?
	 */
	void setStatic (bool is) {
	    this-> _isStatic = is;
	}

	/**
	 * \brief Used for dynamic cast, as typeid and dynamic_cast<T> are not allowed in GCC
	 * \return the list of the TYPEID this instruction can be cast into (its typeid and of its ancestors)
	 */
	virtual std::vector <std::string> getIds () {
	    return {TYPEID (IInstruction)};
	}

	/**
	 * \brief Semantic analyse
	 * \return a semantic analysed instruction (with symbol declared and typed), or NULL if (language) error occurs and have been thrown
	 */
	virtual IInstruction* instruction () = 0;	   

	/**
	 * \brief Replace the different occurence of the variable by template expression
	 * \brief Generally called after, template resolution
	 * \param values an associative array, string => Expression representing the template association
	 * \return an untyped instruction where all occurence found in values are replaced	
	 */	
	virtual IInstruction* templateReplace (const std::map <std::string, Expression>& tmps) = 0;

	/**
	 * \brief format the instruction into a string using the language syntax
	 * \return the formatted string
	 * \warning if this method is not overriden, an assert error occurs (internal error)
	 */
	virtual std::string prettyPrint ();

	/**
	 * \brief Called at lint_time, this method produce the final code that will be send to GCC internals
	 * \return a valid GCC gimple tree 
	 */
	virtual Ymir::Tree toGeneric ();

	/**
	 * \return true if and only if this can be casted into T
	 */
	template <typename T>
	bool is () {
	    return this-> to<T> () != NULL;
	}
	
	/**
	 * \return a element of type T if and only if this can be casted into T, NULL otherwise
	 */
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

    /**
     * \struct INone
     * Empty instruction, produce nothing at lint_time
     */
    class INone : public IInstruction {
    public :

	/**
	 * \param token the location of this instruction
	 */
	INone (Word token) : IInstruction (token) {}

	/**
	 * \return this
	 */
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
