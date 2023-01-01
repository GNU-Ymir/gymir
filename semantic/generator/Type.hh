#pragma once

#include <ymir/semantic/Generator.hh>
#include <ymir/utils/Lazy.hh>

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

	    /** The value contained in this type is pure */
	    bool _isPure = false;
	    
	    /** The list of inner type (applicable iif _isComplex) */
	    std::vector <Generator> _inners;

	    /** The proxy of the type, used for Enum  */
	    Generator _proxy;

	    Ymir::Lazy <std::string, Type> _typeName;
	    
	protected :

	    friend Generator;

	    Type ();

	    Type (const lexing::Word & loc, const std::string & name);

	    Type (const Type & other);
	    
	public : 

	    static Generator initPure (const lexing::Word & loc, const Type & other);
	    
	    static Generator init (const lexing::Word & loc, const Type & other);
	    
	    static Generator init (const lexing::Word & loc, const Type & other, bool isMutable, bool isRef);
	    
	    static Generator init (const Type & other, bool isMutable, bool isRef);
	    
	    static Generator init (const Type & other, bool isMutable);

	    static Generator initPure (const Type & other);
	    
	    /** 
	     * \brief Set the proxy of the type, used in Enum 
	     * \warning not the same proxy as the design pattern
	     * \param gen, an EnumRef, that proxy the type
	     */
	    static Generator init (const Type & other, const Generator & gen);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const override ;


	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     * \it will not check the ref, and mutability infos
	     */
	    bool completeEquals (const Generator & other) const;
	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     * \it will not check the ref, and mutability infos, but check proxy
	     */
	    bool equals (const Generator & other) const override;
	    
	    /**
	     * \return does the current type can store the value of the type other without casting ?
	     */
	    virtual bool isCompatible (const Generator & other) const;

	    /**
	     * \param isParentMutable if the parent is not mutable dont print the mutability information (which is not accurate)
	     * \param includeRef add the keyword ref if it is a ref ?
	     * \return the typename of the type formatted
	     */
	    std::string computeTypeName (bool isParentMutable = true, bool includeRef = true, bool isParentPure = false) const;

	    /**
	     * @return the typename of the type formatted
	     */
	    const std::string & getTypeName () const;
	    
	    /**
	     * \return is this type a reference ?
	     */
	    bool isRef () const;
	    /**
	     * \return is this type mutable ?
	     */
	    bool isMutable () const;

	    /**
	     * \return is the type pure ?
	     */
	    bool isPure () const;
	    
	    /**
	     * \return is this type and all inner types mutable?
	     */
	    bool isDeeplyMutable () const;
	    
	    /**
	     * \return this type is complex ? (cf this-> _isComplex definition)
	     */
	    bool isComplex () const;	    

	    /**
	     * \return this type is in the heap and need to be explicitly aliases
	     */
	    virtual bool needExplicitAlias () const;

	    /**
	     * \return this type is in the heap and need to be explicitly aliases 
	     */
	    virtual bool containPointers () const;	    
	    
	    /**
	     * \return assuming that isComplex () == true, the content type of this type
	     */
	    const std::vector<Generator> & getInners () const;	    	    
	    
	    /**
	     * \brief Transform the type into the same type but deeply mutable (every sub type are also mutable)
	     * \brief This is used when the keyword dmut is used
	     */
	    virtual Generator toDeeplyMutable () const;
	    
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
	     * \warning not the same proxy as the design pattern
	     * \return the proxy of the type (if it is an enum)
	     */
	    const Generator & getProxy () const;
	    
	    /**
	     * \brief Override it for all type -> return getTypeName ();
	     */
	    std::string prettyString () const override final;
	    
	protected: 

	    /**
	     * \brief Used in toMutable, apply the level - 1 operation
	     */
	    Generator toLevelMinusOne (bool isFatherMut) const;
	    
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

	    /**
	     * Change the value of _isMutable, 
	     * Warning different of isMutable (bool)
	     */
	    virtual void setMutable (bool is);

	    /**
	     * Used in the init function
	     */
	    virtual Generator createMutable (bool is) const;

	private :

	    std::string performComputeTypeName () const;
	   	    
	};	

    }
       
}
