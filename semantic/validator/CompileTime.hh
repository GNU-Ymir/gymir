#pragma once

#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/generator/value/_.hh>

namespace semantic {

    namespace validator {

	class CompileTime {

	    Visitor & _context;

	private :

	    /**
	     * \brief As for any no proxied class, it does nothing special
	     * \brief We just wan't all class to be initialized the same way
	     */
	    CompileTime (Visitor & context);
	    
	public :

	    /**
	     * \brief Create a new compile time executor
	     * \param context the context of the compile time
	     */
	    static CompileTime init (Visitor & context);

	    /**
	     * \brief Execute a generator
	     */
	    generator::Generator execute (const generator::Generator & gen);	    
	    
	    
	};
	
    }

}
