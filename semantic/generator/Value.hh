#pragma once

#include <ymir/semantic/Generator.hh>
#include <ymir/semantic/generator/Type.hh>

namespace semantic {

    namespace generator {

	/**
	 * A value is in opposition of type
	 * A value is typed, but a type has no value
	 */
	class Value : public IGenerator {	    	    
	private :

	    Generator _type;

	    /**
	     * this value is set to true if the current generator break the closest loop scope in any case
	     */
	    bool _breaker = false;

	    /**
	     * The location of the breaker inside the expression (if _breaker)
	     */
	    lexing::Word _breakerLoc = lexing::Word::eof ();
	    
	    /**
	     * this value is set to true if the current generator close the frame in any case
	     */
	    bool _returner = false;

	    /**
	     * The location of the returner inside the expression (if _returner)
	     */
	    lexing::Word _returnerLoc = lexing::Word::eof ();
	    
	    /**
	     * This value is set to true, if the current generator can be used as a left value 
	     * (Does not consider const or immutability informations)
	     */
	    bool _isLvalue = false;

	    /**
	     * This value is set to true, if the current generator has been declared and constructed in the frame
	     */
	    bool _isLocal = true;
	    
	protected : 

	    friend Generator;

	    Value ();

	    Value (const lexing::Word & loc, const Generator & type);

	    Value (const lexing::Word & loc, const std::string & name, const Generator & type);
	    
	public :

	    static Generator initBrRet (const Value & other, bool breaker, bool returner, const lexing::Word& brLoc, const lexing::Word & rtLoc);

	    static Generator init (const Value & other, const Generator & type);
	    
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
	     * \return the type of the value
	     */
	    const Generator & getType () const;

	    /**
	     * \return Tell if the generator close the closest loop scope
	     */
	    bool isBreaker () const;

	    /**
	     * \return the location of the breaker
	     */
	    const lexing::Word & getBreakerLocation () const;
	    
	    /**
	     * \return Does this generator close the frame ?
	     */
	    bool isReturner () const;

	    /**
	     * \return the location of the returner loc
	     */
	    const lexing::Word & getReturnerLocation () const;
	    
	    /**
	     * \return Does this generator can be used as a LValue ?
	     * \warning does not check const or imut informations
	     */
	    bool isLvalue () const;

	    /**
	     * \return does this generator has been constructed in the frame ?	     
	     */
	    bool isLocal () const;
	    
	protected:

	    /**
	     * \brief change the lvalue information
	     */
	    void isLvalue (bool is);
	    
	    /**
	     * \brief Change the locality of the value
	     */
	    void isLocal (bool is);

	    /**
	     * A child class can declare itself a returner (e.g. Return)
	     */
	    void setReturner (bool);

	    /**
	     * When a child class declare itself as a returner, 
	     * the returner location is set to the location of the value by default
	     * But it can be modified if needed
	     */
	    void setReturnerLocation (const lexing::Word & loc);
	    
	    /**
	     * A child can declare itself as a breaker (e.g. Break)	       	     
	     */
	    void setBreaker (bool);

	    /**
	     * When a child class declare itself as a breaker, 
	     * the breaker location is set to the location of the value by default
	     * But it can be modified if needed
	     */
	    void setBreakerLocation (const lexing::Word & loc);

	    
	};	

    }
       
}
