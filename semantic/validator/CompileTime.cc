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
		)
		    
		else of (ArrayValue, v ATTRIBUTE_UNUSED,
		    return gen;
		)
			 
		else of (CharValue, c ATTRIBUTE_UNUSED,
		    return gen;
		)
			     
		else of (FloatValue, f ATTRIBUTE_UNUSED,
		    return gen;
		)

		else of (BoolValue, f ATTRIBUTE_UNUSED,
		    return gen;
		)
			 
		else of (None, n ATTRIBUTE_UNUSED,
		    return gen;
		)

		else of (Affect, aff,
		    return executeAffect (aff);
		)

		else of (ArrayAccess, arr,
		    return executeArrayAccess (arr);
		)

		else of (BinaryInt, bin,
		    return executeBinaryInt (bin);
		)

		else of (UnaryInt, una,
		    return executeUnaryInt (una);
		)
			 
		else of (BinaryFloat, fl,
		    return executeBinaryFloat (fl);
		)

		else of (Conditional, cd,
		    return executeConditional (cd);
		)

		else of (Set, st,
		    return executeSet (st);
		) 			 

                else of (Block, bl,
		     return executeBlock (bl);
		)
			 
		else of (generator::VarDecl, vdecl,
		    return executeVarDecl (vdecl);
		)

		else of (VarRef, vref,
		    return executeVarRef (vref);
		)

		else of (Call, cll,
		    return executeCall (cll);
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
	
	ulong getMinU (const Integer &) {
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
		return T ();
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
		return false;
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
	T applyUnaInt (Unary::Operator op, T elem) {
	    switch (op) {
	    case Unary::Operator::MINUS : return -elem;
	    default :
		Ymir::Error::halt ("%(r) - unhandeld case", "Critical");
		return T ();
	    }	    
	}
	
	generator::Generator CompileTime::executeUnaryInt (const generator::UnaryInt & unaInt) {
	    auto elem = this-> execute (unaInt.getOperand ()).to <Fixed> ().getUI ();
	    if (unaInt.getType ().is <Integer> ()) {
		bool isSigned = unaInt.getType ().to <Integer> ().isSigned ();
		std::string type = unaInt.getType ().to <Integer> ().typeName ();
		
		long maxI = getMaxS (unaInt.getType ().to <Integer> ());
		ulong maxU = getMaxU (unaInt.getType ().to <Integer> ());

		long minI = getMinS (unaInt.getType ().to <Integer> ());
		ulong minU = getMinU (unaInt.getType ().to <Integer> ());
		
		Fixed::UI result;
		if (isSigned) {
		    result.i = applyUnaInt <long> (unaInt.getOperator (), elem.i);
		} else
		    result.u = applyUnaInt <ulong> (unaInt.getOperator (), elem.u);

				
		if (isSigned && (result.i > maxI || result.i < minI))
		    Ymir::Error::occur (unaInt.getLocation (), ExternalError::get (OVERFLOW), type, result.i);
		else if (!isSigned && (result.u > maxU || result.u < minU))
		    Ymir::Error::occur (unaInt.getLocation (), ExternalError::get (OVERFLOW), type, result.u);

		return Fixed::init (unaInt.getLocation (), unaInt.getType (), result);
	    } else
		return Generator::empty ();
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
		return T ();
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
		return false;
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
	    auto test = this-> execute (conditional.getTest ());
	    if (test.isEmpty () || !test.is <BoolValue> ())
		return Generator::empty ();

	    auto isTrue = test.to <BoolValue> ().getValue ();
	    if (isTrue) {
		return this-> execute (conditional.getContent ());		
	    } else if (!conditional.getElse ().isEmpty ())
		return this-> execute (conditional.getElse ());
	    
	    return Generator::empty ();
	}

	generator::Generator CompileTime::executeSet (const generator::Set &) {
	    return Generator::empty ();
	}

	generator::Generator CompileTime::executeBlock (const generator::Block & block) {
	    if (block.getType ().is <Void> ()) 
		return Generator::empty ();
	    else return this-> execute (block.getContent ().back ());			       
	}
	
	generator::Generator CompileTime::executeVarDecl (const generator::VarDecl &) {
	    return Generator::empty ();
	}

	generator::Generator CompileTime::executeVarRef (const generator::VarRef & ref) {
	    if (ref.getValue ().isEmpty ())
		return Generator::empty ();
	    else return this-> execute (ref.getValue ());
	}

	generator::Generator CompileTime::executeCall (const generator::Call & call) {
	    if (call.getParameters ().size () != 0) {
		return Generator::empty ();
	    } else {
		return this-> executeFrame (call.getFrame ().to <generator::FrameProto> ());
	    }
	}

	generator::Generator CompileTime::executeFrame (const generator::FrameProto & fr) {
	    auto & frame = this-> _context.retreiveFrameFromProto (fr);
	    if (!frame.isEmpty ()) 
		return this-> execute (frame.to<Frame> ().getContent ());
	    else return Generator::empty ();
	}
	
    }
    
}
