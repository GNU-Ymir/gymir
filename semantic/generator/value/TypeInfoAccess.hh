#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct Call
	 * Access a specific index inside an array
	 */
	class TypeInfoAccess : public Value {

	    Generator _classRef;	   
	    
	private :

	    friend Generator;
	    
	    TypeInfoAccess ();

	    TypeInfoAccess (const lexing::Word & loc, const Generator & type, const Generator & classRef);

	public :

	    /**
	     * \brief Generate a new affectation
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & classRef);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;
	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    /**
	     * \return the self element (if any, if not it will be constructed at generation time);
	     */
	    const Generator & getClassRef () const;
	    
	    std::string prettyString () const override;
	    
	};
	
    }

}
