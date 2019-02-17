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
	    
	    /**
	     * \brief Execute an affectation at compile time 
	     * \brief Affectation is not a compile time instruction, so this function will just throw an ExternalError
	     */
	    generator::Generator executeAffect (const generator::Affect & gen);

	    /**
	     * \brief Will execute an array access
	     * \brief It works only if the array is compile time
	     */
	    generator::Generator executeArrayAccess (const generator::ArrayAccess & acc);

	    /**
	     * \brief Execute a binary int expression
	     * \brief Two operands needs to be compile time known
	     */
	    generator::Generator executeBinaryInt (const generator::BinaryInt & binInt);
	    
	    /**
	     * \brief Execute a binary float expression
	     * \brief Two operands needs to be compile time known
	     */
	    generator::Generator executeBinaryFloat (const generator::BinaryFloat & binFloat);

	    /**
	     * \brief Execute a conditional block at compile time
	     */
	    generator::Generator executeConditional (const generator::Conditional & conditional);

	    /**
	     * \brief Execute a set of expression at compile time
	     */
	    generator::Generator executeSet (const generator::Set & set);

	    /**
	     * \brief execute a var ref at compile time
	     */
	    generator::Generator executeVarDecl (const generator::VarDecl & decl);

	    /**
	     * \brief Execute a var reference (return the value of the assiociated vardecl, if it is immutable)
	     */
	    generator::Generator executeVarRef (const generator::VarRef & ref);
	    
	};
	
    }

}
