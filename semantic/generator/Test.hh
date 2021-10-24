#pragma once

#include <ymir/semantic/Generator.hh>

namespace semantic {
    namespace generator {

	class Test : public IGenerator {
	private :

	    Generator _value;

	    
	private :

	    friend Generator;
	    
	    Test ();

	    Test (const lexing::Word & location, const std::string & name, const Generator & value);

	public :

	    /**
	     * \brief Create a unit Test 
	     * \param location the location of the test (for debug info)
	     * \param name the name of the test (unmangled, but with location information included)
	     * \param value the test to run
	     */
	    static Generator init (const lexing::Word & location, const std::string & name, const Generator & value);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    virtual Generator clone () const ;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    virtual bool equals (const Generator & other) const ;	    

	    /**
	     * \return the value of the var
	     */
	    const Generator & getValue () const;
	    
	};
	
    }
}
