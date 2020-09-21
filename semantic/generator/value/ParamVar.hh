#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {
    namespace generator {

	class ParamVar : public Value {	    

	    bool _isMutable;

	    bool _isSelf;
	    
	private :

	    friend Generator;
	    
	    ParamVar ();

	    ParamVar (const lexing::Word & location, const Generator & type, bool isMutable, bool isSelf);

	public :

	    /**
	     * \brief Create a param var 
	     * \param location the location of the param var (for debug info)
	     * \param type the type of the var
	     */
	    static Generator init (const lexing::Word & location, const Generator & type, bool isMutable, bool isSelf);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    virtual Generator clone () const ;

	    /**
	     * \brief Mandatory function used inside proxy design pattern for dynamic casting
	     */
	    virtual bool isOf (const IGenerator * type) const ;	    

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    virtual bool equals (const Generator & other) const ;	    
	    
	    /**
	     * \return has this var been declared mutable ?
	     */
	    bool isMutable () const;

	    /**
	     * Is defining the self var of a method
	     */
	    bool isSelf () const;

	    std::string prettyString () const override;
	};
	
    }
}
