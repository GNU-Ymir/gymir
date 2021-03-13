#pragma once

#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/validator/type/_.hh>

namespace semantic {

    namespace validator {

	class ClassVisitor {

	    Visitor & _context;

	private :

	    ClassVisitor (Visitor & context);

	public :

	    /**
	     * Create a new visitor
	     * This class is not proxied
	     */
	    static ClassVisitor init (Visitor & content);

	    /**
	     * Validate the class symbol
	     */
	    generator::Generator validate (const semantic::Class & cls, bool inModule = false);


	private :
	    
	    generator::Generator validateAncestor (const semantic::Class & cls, const syntax::Expression & expression);
	    
	};
	
    }
    
}
