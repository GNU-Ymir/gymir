#pragma once

#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/generator/type/_.hh>

namespace semantic {

    namespace validator {


	/**
	 * This visitor validate the expression __pragma, that begun to be heavy to manage 
	 */
	class PragmaVisitor {
	public :

	    static const std::string COMPILE;
	    static const std::string MANGLE;
	    static const std::string OPERATOR;
	    
	    static const std::string FIELD_NAMES;
	    static const std::string FIELD_OFFSETS;
	    static const std::string FIELD_TYPE;
	    
	    static const std::string TUPLEOF;
	    static const std::string LOCAL_TUPLEOF;
	    static const std::string HAS_DEFAULT;
	    static const std::string DEFAULT_VALUE;
	    static const std::string TRUSTED;

	    Visitor & _context;

	private :

	    PragmaVisitor (Visitor & context);

	public:


	    /**
	     * Create a new visitor
	     * This class is not proxied
	     */
	    static PragmaVisitor init (Visitor & context);

	    /**
	     * Validate the expression and return a generator
	     */
	    generator::Generator validate (const syntax::Pragma & expression);
	    
	    generator::Generator validateCompile (const syntax::Pragma & prg);

	    generator::Generator validateTrusted (const syntax::Pragma & prg);
	    
	    generator::Generator validateMangle (const syntax::Pragma & prg);

	    generator::Generator validateOperator (const syntax::Pragma & prg);

	    generator::Generator validateFieldNames (const syntax::Pragma & prg);

	    generator::Generator validateEnumFieldNames (const syntax::Pragma & prg, const generator::Enum & cl);
	    
	    generator::Generator validateStructFieldNames (const syntax::Pragma & prg, const generator::Struct & cl);	    

	    generator::Generator validateClassFieldNames (const syntax::Pragma & prg, const generator::Class & cl);


	    generator::Generator validateFieldAddress (const syntax::Pragma & prg);
	    
	    generator::Generator validateStructFieldAddress (const syntax::Pragma & prg, const generator::Generator & cl);
	    
	    generator::Generator validateClassFieldAddress (const syntax::Pragma & prg, const generator::Generator & cl);

	    generator::Generator validateTupleOf (const syntax::Pragma & prg);

	    generator::Generator validateArrayTupleOf (const syntax::Pragma & prg, const generator::Generator & cl);
	    
	    generator::Generator validateStructTupleOf (const syntax::Pragma & prg, const generator::Generator & cl);
	    
	    generator::Generator validateClassTupleOf (const syntax::Pragma & prg, const generator::Generator & cl);

	    generator::Generator validateLocalTupleOf (const syntax::Pragma & prg);
	    
	    generator::Generator validateClassLocalTupleOf (const syntax::Pragma & prg, const generator::Generator & cl);

	    generator::Generator validateHasDefault (const syntax::Pragma & prg);

	    generator::Generator validateStructHasDefault (const syntax::Pragma & prg, const generator::Struct & str);

	    generator::Generator validateDefaultValue (const syntax::Pragma & prg);

	    generator::Generator validateStructDefaultValue (const syntax::Pragma & prg, const generator::Struct & str, const generator::Generator & name);

	    generator::Generator validateFieldType (const syntax::Pragma & prg);

	    generator::Generator validateStructFieldType (const syntax::Pragma & prg, const generator::Struct & str, const generator::Generator & name);

	    generator::Generator validateFieldOffsets (const syntax::Pragma & prg);

	    generator::Generator validateStructFieldOffsets (const syntax::Pragma & prg, const generator::Struct & str);	    

	};

	
    }
    
}
