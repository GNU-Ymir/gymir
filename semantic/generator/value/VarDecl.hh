#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {
    namespace generator {

	class ParamVar;
	
	class VarDecl : public Value {	    

	    std::string _name;

	    Generator _type;

	    Generator _value;

	    bool _isMutable;

	    static uint __lastId__;

	    uint _varRefId;

	    friend ParamVar;
	    
	private :

	    friend Generator;
	    
	    VarDecl ();

	    VarDecl (const lexing::Word & location, const std::string & name, const Generator & type, const Generator & value, bool isMutable);

	public :

	    /**
	     * \brief Create a ref var 
	     * \param location the location of the ref var (for debug info)
	     * \param name the name of the ref var (unmangled, but with location information included)
	     * \param type the type of the var
	     * \param refId the id of the generator that will declare the var
	     */
	    static Generator init (const lexing::Word & location, const std::string & name, const Generator & type, const Generator & value, bool isMutable);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    virtual Generator clone () const ;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    virtual bool equals (const Generator & other) const ;	    

	    /**
	     * \return the type of the var declared
	     * \warning different from getType, which is always Void
	     */
	    const Generator & getVarType () const;

	    /**
	     * \return the initial value of the var
	     */
	    const Generator & getVarValue () const;

	    /**
	     * \return has this var been declared mutable ?
	     */
	    bool isMutable () const;
	    
	    /**
	     * \brief Override the getuniqid, the ref id of a var decl must be copied, all the varref will refer to it
	     */
	    uint getUniqId () const override;
	    
	    std::string prettyString () const override;
	};
       	
    }
}
