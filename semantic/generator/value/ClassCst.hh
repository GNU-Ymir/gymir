#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct Call
	 * Access a specific index inside an array
	 */
	class ClassCst : public Value {

	    Generator _frame;

	    Generator _self;
	    
	    std::vector <Generator> _types;
	    
	    std::vector <Generator> _params;	    
	    
	private :

	    friend Generator;
	    
	    ClassCst ();

	    ClassCst (const lexing::Word & loc, const Generator & type, const Generator & frame, const std::vector<Generator> & types, const std::vector <Generator> & params);

	public :

	    /**
	     * \brief Generate a new affectation
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & frame, const std::vector<Generator> & types, const std::vector <Generator> & params);

	    /**
	       
	     */
	    static Generator init (const ClassCst & other, const Generator & self);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;
	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    /**
	     * \return the generator of the frame that will be called
	     */
	    const Generator & getFrame () const;

	    /**
	     * \return the type casters
	     */
	    const std::vector <Generator> & getTypes () const;
	    
	    /**
	     * \return the parameters
	     */
	    const std::vector <Generator> & getParameters () const;

	    /**
	     * \return the self element (if any, if not it will be constructed at generation time);
	     */
	    const Generator & getSelf () const;
	    
	    std::string prettyString () const override;
	    
	};
	
    }

}
