#pragma once

#include <ymir/semantic/generator/Type.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct A slice is an array with a dynamic size
	 * It can borrow data from static array, are allocated on the heap
	 */
	class Closure : public Type {

	    /** The name of the enclosed types */
	    std::vector <std::string> _names;

	    /* This is the most ugly thing in this compiler, but ... I
	       don't see any other way to do that, without rethinking
	       a bunch of stuff,

	       But I will assume that, a frame (foreign) cannot be
	       exited without all the inner element validated, right?
	    */
	    uint32_t _index; 
	    
	public :
	    
	    static std::string ARITY_NAME;

	    static std::string INIT_NAME;
	    
	private : 

	    friend Generator;

	    Closure ();

	    Closure (const lexing::Word & loc, const std::vector <Generator> & innerType, const std::vector<std::string> & names, uint32_t position);

	public : 


	    static Generator init (const lexing::Word & loc, const std::vector <Generator> & innerType, const std::vector<std::string> & names, uint32_t position);
	    
	    Generator clone () const override;

	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;
	    
	    /** 
	     * \return the name of the type formatted
	     */
	    std::string typeName () const override;

	    /**
	     * \return the type of the field named name, or empty
	     */
	    const Generator& getField (const std::string & name) const;

	    /**
	     * \return the names of the fields
	     */
	    const std::vector <std::string> & getNames () const;

	    /**
	     * \return the index of the enclosed symbols
	     */
	    uint32_t getIndex () const;
	    
	    /**
	     * \brief Tell if the tuple type if fake (generated by template)
	     */
	    bool isFake () const;


	    Generator toMutable () const override;

	    Generator toDeeplyMutable () const override;
	    
	protected:

	    /**
	     * Does not change the mutability , nor ref of the params
	     */
	    Generator createMutable (bool is) const override;	    
	    
	};

    }
    
}
