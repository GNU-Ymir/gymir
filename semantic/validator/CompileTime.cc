#include <ymir/semantic/validator/CompileTime.hh>

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;

	
	CompileTime::CompileTime (Visitor & context) :
	    _context (context)
	{}

	CompileTime CompileTime::init (Visitor & context) {
	    return CompileTime (context);
	}

	Generator CompileTime::execute (const Generator & gen) {
	    match (gen) {
		of (Fixed, f ATTRIBUTE_UNUSED,
		    return gen;
		);

		of (ArrayValue, v ATTRIBUTE_UNUSED,
		    return gen;
		);

		of (CharValue, c ATTRIBUTE_UNUSED,
		    return gen;
		);

		of (FloatValue, f ATTRIBUTE_UNUSED,
		    return gen;
		);

		of (None, n ATTRIBUTE_UNUSED,
		    return gen;
		);

		of (Affect, aff,
		    return executeAffect (aff);
		);

		of (ArrayAccess, arr,
		    return executeArrayAccess (arr);
		);

		of (BinaryInt, bin,
		    return executeBinaryInt (bin);
		);

		of (BinaryFloat, fl,
		    return executeBinaryFloat (fl);
		);

		of (Conditional, cd,
		    return executeConditional (cd);
		);

		of (Set, st,
		    return executeSet (st);
		);

		// of (VarDecl, vdecl,
		//     return executeVarDecl (vdecl);
		// );

		of (VarRef, vref,
		    return executeVarRef (vref);
		);
		
	    }
	    
	    Ymir::Error::occur (
		gen.getLocation (),
		ExternalError::get (COMPILE_TIME_UNKNOWN)
	    );
	    return gen;
	}

	generator::Generator CompileTime::executeAffect (const generator::Affect & gen) {
	    Ymir::Error::occur (
		gen.getLocation (),
		ExternalError::get (AFFECT_COMPILE_TIME)
	    );
	    return Generator::empty ();
	}

	generator::Generator CompileTime::executeArrayAccess (const generator::ArrayAccess & acc) {
	    auto array = this-> execute (acc.getArray ());
	    auto index = this-> execute (acc.getIndex ());

	    // everything has alreagy been validated, we assume that index is a Fixed, and array an array
	    return array.to<ArrayValue> ().getContent ()[index.to<Fixed> ().getUI ().u];
	}

	ulong getMaxU (const Integer & type) {
	    switch (type.getSize ()) {
	    case 8 : return UCHAR_MAX;
	    case 16 : return USHRT_MAX;
	    case 32 : return UINT_MAX;
	    case 64 : return ULONG_MAX;
	    }
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return 0;
	}
		
	ulong getMaxS (const Integer & type) {
	    switch (type.getSize ()) {
	    case 8 : return SCHAR_MAX;
	    case 16 : return SHRT_MAX;
	    case 32 : return INT_MAX;
	    case 64 : return LONG_MAX;
	    }
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return 0;
	}
	
	ulong getMinU (const Integer & type) {
	    return 0;
	}
		
	ulong getMinS (const Integer & type) {
	    switch (type.getSize ()) {
	    case 8 : return SCHAR_MIN;
	    case 16 : return SHRT_MIN;
	    case 32 : return INT_MIN;
	    case 64 : return LONG_MIN;
	    }
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return 0;
	}

	template <typename T>
	T applyBinInt (Binary::Operator op, T left, T right) {
	    switch (op) {
	    case Binary::Operator::LEFT_SHIFT : return left << right;
	    case Binary::Operator::RIGHT_SHIFT : return left >> right;
	    case Binary::Operator::BIT_OR : return left | right;
	    case Binary::Operator::BIT_AND : return left & right; 
	    case Binary::Operator::BIT_XOR : return left ^ right;
	    case Binary::Operator::ADD : return left + right;
	    case Binary::Operator::SUB : return left - right;
	    case Binary::Operator::MUL : return left * right;
	    case Binary::Operator::DIV : return left / right;
	    case Binary::Operator::MODULO : return left % right;
	    default :
		Ymir::Error::halt ("%(r) - unhandeld case", "Critical");
	    }	    
	}
	
	template <typename T>
	bool applyBinIntBool (Binary::Operator op, T left, T right) {
	    switch (op) {
	    case Binary::Operator::INF : return left < right; 
	    case Binary::Operator::SUP : return left > right; 
	    case Binary::Operator::INF_EQUAL : return left <= right; 
	    case Binary::Operator::SUP_EQUAL : return left >= right; 
	    case Binary::Operator::EQUAL : return left == right; 
	    case Binary::Operator::NOT_EQUAL : return left != right; 
	    default :
		Ymir::Error::halt ("%(r) - unhandeld case", "Critical");
	    }
	}
	
	generator::Generator CompileTime::executeBinaryInt (const generator::BinaryInt & binInt) {
	    auto left = this-> execute (binInt.getLeft ()).to<Fixed> ().getUI ();
	    auto right = this-> execute (binInt.getRight ()).to<Fixed> ().getUI ();
	    if (binInt.getType ().is<Integer> ()) {
		bool isSigned = binInt.getType ().to <Integer> ().isSigned ();
		std::string type = binInt.getType ().to <Integer> ().typeName ();
		
		long maxI = getMaxS (binInt.getType ().to <Integer> ());
		ulong maxU = getMaxU (binInt.getType ().to <Integer> ());
		
		long minI = getMinS (binInt.getType ().to <Integer> ());
		ulong minU = getMinU (binInt.getType ().to <Integer> ());
		
		Fixed::UI result;
		
		if (isSigned) {
		    result.i = applyBinInt<long> (binInt.getOperator (), left.i, right.i);
		} else
		    result.u = applyBinInt<ulong> (binInt.getOperator (), left.u, right.u);
		
		if (isSigned && (result.i > maxI || result.i < minI))
		    Ymir::Error::occur (binInt.getLocation (), ExternalError::get (OVERFLOW), type, result.i);
		else if (!isSigned && (result.u > maxU || result.u < minU))
		    Ymir::Error::occur (binInt.getLocation (), ExternalError::get (OVERFLOW), type, result.u);
		
		return Fixed::init (binInt.getLocation (), binInt.getType (), result);
	    } else {
		bool isSigned = binInt.getLeft ().to <Value> ().getType ().to<Integer> ().isSigned ();
		bool res;
		if (isSigned) {
		    res = applyBinIntBool<long> (binInt.getOperator (), left.i, right.i);
		} else
		    res = applyBinIntBool<ulong> (binInt.getOperator (), left.u, right.u);

		return BoolValue::init (binInt.getLocation (), Bool::init (binInt.getLocation ()), res);
	    }
	}
	
	template <typename T>
	T applyBinFloat (Binary::Operator op, T left, T right) {
	    switch (op) {
	    case Binary::Operator::ADD : return left + right;
	    case Binary::Operator::SUB : return left - right;
	    case Binary::Operator::MUL : return left * right;
	    case Binary::Operator::DIV : return left / right;
	    default :
		Ymir::Error::halt ("%(r) - unhandeld case", "Critical");
	    }	    
	}
	
	template <typename T>
	bool applyBinFloatBool (Binary::Operator op, T left, T right) {
	    switch (op) {
	    case Binary::Operator::INF : return left < right; 
	    case Binary::Operator::SUP : return left > right; 
	    case Binary::Operator::INF_EQUAL : return left <= right; 
	    case Binary::Operator::SUP_EQUAL : return left >= right; 
	    case Binary::Operator::EQUAL : return left == right; 
	    case Binary::Operator::NOT_EQUAL : return left != right; 
	    default :
		Ymir::Error::halt ("%(r) - unhandeld case", "Critical");
	    }
	}
	
	generator::Generator CompileTime::executeBinaryFloat (const generator::BinaryFloat & binFloat) {
	    auto left = strtod (this-> execute (binFloat.getLeft ()).to<FloatValue> ().getValue ().c_str (), NULL);
	    auto right = strtod (this-> execute (binFloat.getRight ()).to<FloatValue> ().getValue ().c_str (), NULL);
	    if (binFloat.getType ().is<Float> ()) {
		auto res = applyBinFloat (binFloat.getOperator (), left, right);		
		return FloatValue::init (binFloat.getLocation (), binFloat.getType (), format ("%", res));
	    } else {
		auto res = applyBinFloatBool<double> (binFloat.getOperator (), left, right);
		return BoolValue::init (binFloat.getLocation (), Bool::init (binFloat.getLocation ()), res);
	    }
	}

	generator::Generator CompileTime::executeConditional (const generator::Conditional & conditional) {
	    return Generator::empty ();
	}

	generator::Generator CompileTime::executeSet (const generator::Set & set) {
	    return Generator::empty ();
	}

	generator::Generator CompileTime::executeVarDecl (const generator::VarDecl & decl) {
	    return Generator::empty ();
	}

	generator::Generator CompileTime::executeVarRef (const generator::VarRef & ref) {
	    return Generator::empty ();
	}

    }
    
}
