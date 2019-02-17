#pragma once

#include <ymir/semantic/Generator.hh>

namespace semantic {

    namespace generator {

	/**
	 * A type is in opposition of value
	 * A value is typed, but a type has no value
	 */
	class Type : public IGenerator {	    	    	    
	private :

	    /** This type is a reference to another value of the same type ? */
	    bool _isRef = false;

	    /** This type is a complex one 
	     * - simple affectation result in a implicit referencing, as pointer are contained inside the type
	     * - This is not allowed in the language, so we have to prevent it
	     * - For the moment those types are : 
	     * - - Array (special case, in fact array are simple types, but are treated as complex one)
	     * - - Slice
	     */
	    bool _isComplex = false;

	    /**
	     * If the type _isComplex, it has a locality
	     * The assertion is that a local data cannot be passed to a global one
	     * Here is the graph of data locality : 
	     * \verbatim
	     ---------          ----------
	     |       |          |        |
	     | local | <------- | global |
	     |       |          |        |
	     ---------          ----------
	      |    ^              |    ^
	      |    |              |    |
	      ------              ------
	     \endverbatim
	     We wan't to ensure that no local data can get out of a frame
	     */
	    bool _isLocal = false;

	    /** This type contains a value, and is mutable ? */
	    bool _isMutable = false;

	    /** The list of inner type (applicable iif _isComplex) */
	    std::vector <Generator> _inners;
	    
	protected :

	    friend Generator;

	    Type ();

	    Type (const lexing::Word & loc, const std::string & name);
	    
	public : 

	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const override ;

	    /**
	     * \brief Mandatory function used inside proxy design pattern for dynamic casting
	     */
	    bool isOf (const IGenerator * type) const override;	    

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;
	    
	    /**
	     * \return does the current type can store the value of the type other without casting ?
	     */
	    virtual bool isCompatible (const Generator & other) const;

	    /**
	     * \param isParentMutable if the parent is not mutable dont print the mutability information (which is not accurate)
	     * \return the typename of the type formatted
	     */
	    std::string getTypeName (bool isParentMutable = true) const;
	    
	    /**
	     * \return is this type a reference ?
	     */
	    bool isRef () const;
	    /**
	     * \return is this type mutable ?
	     */
	    bool isMutable () const;

	    /**
	     * \return this type is complex ? (cf this-> _isComplex definition)
	     */
	    bool isComplex () const;	    
	    
	    /**
	     * \return assuming that isComplex () == true, the content type of this type
	     */
	    const std::vector<Generator> & getInners () const;	    
	    
	    /**
	     * \brief Set if the type is a reference
	     */
	    void isRef (bool is);
	   	    
	    /**
	     * \brief if the type is mutable
	     */
	    virtual void isMutable (bool is);

	    /**
	     * \brief The rule is simple, const is transitive : 
	     * \brief There are two different kind of type : 
	     * \brief - Simple : X -> mut X
	     * \brief - Complex : X (Y) -> mut X (Y with const level - 1) 
	     * \brief level - 1 on a type, means : 
	     * \brief - If Y is mutable -> Y
	     * \brief - Else if Complex Y (Z) -> mut Y (const Z)
	     * \brief - Else Y -> mut Y
	     * \brief example : 
	     * \verbatim
	     * [[i32]] -> mut [mut [i32]]
	     * [[mut i32]] -> mut [mut [i32]]
	     * [mut [i32]] -> mut [mut [i32]]
	     \endverbatim
	     * \return a mutable version of this type, ensuring there are no legal escapes for const discard
	     */
	    virtual Generator toMutable () const;

	    /**
	     * \brief the mutability level is used to compare two types of the same kind
	     * example : 
	     * \verbatim
	     mut [mut i32] -> 2
	     mut [i32] -> 1
	     [i32] -> 0 
	     mut [mut [mut i32]] -> 0
	     mut [[32]] -> 1
	     mut [mut [i32]] -> 2
	     mut [mut [mut i32]] -> 3
	     \endverbatim
	     * \return the mutability level of the type
	     */
	    virtual int mutabilityLevel (int level = 0) const;
	    
	    /**
	     * \return does this type refer to local data ? (simple type never does)
	     */
	    virtual bool isLocal () const;

	    /**
	     * \warning applicable iif this-> isComplex ()
	     * \brief Change the locality of the type 
	     * \brief Set to true, means the type is refering to local datas
	     */
	    void isLocal (bool local);

	protected: 

	    /**
	     * \brief Used in toMutable, apply the level - 1 operation
	     */
	    virtual Generator toLevelMinusOne () const;
	    
	    /**
	     * \brief if (is) the type is complex 
	     */
	    void isComplex (bool is);

	    /**
	     * \return the name of this type formatted
	     */
	    virtual std::string typeName () const ;

	    /**
	     * \brief Change the inner type
	     */
	    void setInners (const std::vector<Generator> & gen);
	    
	};	

    }
       
}
