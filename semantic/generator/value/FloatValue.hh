#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * A block possess a list of expression 
	 * it has also a type, (as any value)
	 */
	class FloatValue : public Value {	    
	private :
	    
	    std::string _value;

	    float _valFloat = 0;

	    double _valDouble = 0;

	    bool _isStr;
	    
	protected : 

	    friend Generator;

	    FloatValue ();

	    FloatValue (const lexing::Word & loc, const Generator & type, const std::string & value);

	    FloatValue (const lexing::Word & loc, const Generator & type, float valFloat);

	    FloatValue (const lexing::Word & loc, const Generator & type, double valDouble);
	    
	public :

	    static Generator init (const lexing::Word & loc, const Generator & type, const std::string & value);

	    static Generator init (const lexing::Word & loc, const Generator & type, float valFloat);
	    
	    static Generator init (const lexing::Word & loc, const Generator & type, double valDouble);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    /**
	     * \return the value of the float const 
	     */
	    const std::string & getValue () const;

	    /**
	     * \return the value is encoded in a string
	     */
	    bool isStr () const;

	    /**
	     * \return the value encoded in a float
	     */
	    float getValueFloat () const;

	    /**
	     * \return the value encoded in a double
	     */
	    double getValueDouble () const;

	    std::string prettyString () const override;
	};	
	
    }
       
}

