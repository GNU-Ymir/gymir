#pragma once

#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/generator/value/_.hh>

namespace semantic {

    namespace validator {
	
	class CompileTime {
	    
	    enum CompileConstante {
		LIMIT_RECURSE_CALL = 300
	    };
	    
	    Visitor & _context;

	    std::vector <std::pair <generator::Generator, generator::Generator> > _knownValues;
	    
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
	     * \brief Execute a binary int expression
	     * \brief Two operands needs to be compile time known
	     */
	    generator::Generator executeBinaryChar (const generator::BinaryChar & binChar);

	    /**
	     * \brief Execute an unary int expression
	     */
	    generator::Generator executeUnaryInt (const generator::UnaryInt & unaInt);
	    
	    /**
	     * \brief Execute an unary bool expression
	     */
	    generator::Generator executeUnaryBool (const generator::UnaryBool & unaBool);
	    
	    /**
	     * \brief Execute a binary float expression
	     * \brief Two operands needs to be compile time known
	     */
	    generator::Generator executeBinaryFloat (const generator::BinaryFloat & binFloat);

	    /**
	     * \brief Execute a binary bool expression
	     * \brief Two operands needs to be compile time known
	     */
	    generator::Generator executeBinaryBool (const generator::BinaryBool & binFloat);

	    /**
	     * \brief Execute a conditional block at compile time
	     */
	    generator::Generator executeConditional (const generator::Conditional & conditional);

	    /**
	     * \brief Execute a set of expression at compile time
	     */
	    generator::Generator executeSet (const generator::Set & set);

	    /**
	     * \brief Execute a block of expression at compile time
	     */
	    generator::Generator executeBlock (const generator::Block & bl);

	    /**
	     * \brief execute a var ref at compile time
	     */
	    generator::Generator executeVarDecl (const generator::VarDecl & decl);

	    /**
	     * \brief Execute a var reference (return the value of the assiociated vardecl, if it is immutable)
	     */
	    generator::Generator executeVarRef (const generator::VarRef & ref);

	    /**
	     * \brief Execute a frame call at compile time
	     */
	    generator::Generator executeCall (const generator::Call & call);

	    /**
	     * \brief Execute a frame at compile time (used in executeCall)
	     */
	    generator::Generator executeFrame (const generator::FrameProto & fr);

	    /**
	     * \brief Execute an addresser at compile time
	     * \brief If the addresser is a funcPtr to a FrameProto we know it at compile time
	     */
	    generator::Generator executeAddresser (const generator::Generator & addr);

	    /**
	     * \brief Execute an alias at compile time 
	     * \brief it can be an alias of a something known at compile time such as StringValue or ArrayValue
	     */
	    generator::Generator executeAlias (const generator::Generator & alias);
	    
	    /**
	     * \brief Execute lambda proto
	     * \brief it is always known at compile time
	     */
	    generator::Generator executeLamdaProto (const generator::Generator & addr);

	    /**
	     * \brief Execute template ref 
	     * \brief it is known iif it refer to a function
	     */
	    generator::Generator executeTemplateRef (const generator::Generator & addr);

	    /**
	     * \brief Execute a mult sym, a mult sym can be known at compile time, if it refers to elements that are known at compile time
	     */
	    generator::Generator executeMultSym (const generator::Generator & addr);

	    /**
	     * \brief Execute a sizeof 
	     */
	    generator::Generator executeSizeOf (const generator::Generator & sof);

	    /**
	     * \brief Return a funcptr on the prototype
	     */
	    generator::Generator executeFrameProto (const generator::Generator & proto);

	    /**
	     * \brief execute a range value (validate the content of the range value)
	     */
	    generator::Generator executeRangeValue (const generator::RangeValue & proto);

	    /**
	     * \brief execute a slice concat value (validate the content of the slice value)
	     */	    
	    generator::Generator executeSliceConcat (const generator::SliceConcat & slc);

	    /**
	     * \brief execute a slice compare value
	     */	    
	    generator::Generator executeSliceCompare (const generator::SliceCompare & slc);
	    
	};
	
    }

}
