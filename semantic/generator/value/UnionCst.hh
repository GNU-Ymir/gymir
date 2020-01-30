#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * \str UnionCst
	 * Access a specific index inside an array
	 */
	class UnionCst : public Value {

	    Generator _str;

	    std::string _fieldName;
	    
	    Generator _typeCst;
	    
	    Generator _param;

	private :

	    friend Generator;
	    
	    UnionCst ();

	    UnionCst (const lexing::Word & loc, const Generator & type, const Generator & str, const std::string & name, const Generator & typeCst, const Generator & param);

	public :

	    /**
	     * \brief Generate a new affectation
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & str, const std::string & name, const Generator & typeCst, const Generator & param);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;

	    /**
	     * \brief Mandatory function used inside proxy design pattern for dynamic casting
	     */
	    bool isOf (const IGenerator * type) const override;	    
	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    /**
	     * \return the generator of the str that will be created
	     */
	    const Generator & getStr () const;

	    /**
	     * \return the field name
	     */
	    const std::string & getFieldName () const;
	    
	    /**
	     * \return the type casters
	     */
	    const Generator & getTypeCst () const;
	    
	    /**
	     * \return the parameters
	     */
	    const Generator & getParameter () const;

	    
	    std::string prettyString () const override;
	    
	};
	
    }

}
