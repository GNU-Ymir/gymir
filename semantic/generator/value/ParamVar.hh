#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {
    namespace generator {

	class ParamVar : public Value {	    

	    bool _isMutable;

	    bool _isSelf;

	    uint _varRefId;
	    
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

	    /**
	     * \brief Override the getuniqid, the ref id of a var decl must be copied, all the varref will refer to it
	     */
	    uint getUniqId () const override;
	    
	    std::string prettyString () const override;
	};
	
    }
}
