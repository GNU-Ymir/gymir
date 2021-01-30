#include <ymir/semantic/validator/CompileTime.hh>
#include <ymir/global/State.hh>

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
	    static int nb_recurs = 0;
	    nb_recurs += 1;
	    
	    std::list <Ymir::Error::ErrorMsg> errors;
	    try {
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

		of (BoolValue, f ATTRIBUTE_UNUSED,
		    return gen;
		);
			 
		of (None, n ATTRIBUTE_UNUSED,
		    return gen;
		);

		of (StringValue, s ATTRIBUTE_UNUSED,
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

		of (BinaryBool, bin,
		    return executeBinaryBool (bin);
		);

		of (UnaryInt, una,
		    return executeUnaryInt (una);
		);

		of (UnaryBool, ub,
		    return executeUnaryBool (ub);
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

		of (Block, bl,
		    return executeBlock (bl);
		);
			 
		of (generator::VarDecl, vdecl,
		    return executeVarDecl (vdecl);
		);

		of (VarRef, vref,
		    return executeVarRef (vref);
		);
		
		of (Addresser, addr ATTRIBUTE_UNUSED,
		    return executeAddresser (gen);
		);

		of (Aliaser, al ATTRIBUTE_UNUSED,
		    return executeAlias (gen);
		);
		
		of (Call, cll,
		    return executeCall (cll);
		);

		of (LambdaProto, proto ATTRIBUTE_UNUSED,
		    return executeLamdaProto (gen);
		);

		of (TemplateRef, _ref ATTRIBUTE_UNUSED,
		    return executeTemplateRef (gen);
		);

		of (FrameProto, proto ATTRIBUTE_UNUSED,
		    return executeFrameProto (gen);
		);
		
		of (MultSym, mult ATTRIBUTE_UNUSED,
		    return executeMultSym (gen);
		);

		of (UniqValue, val,
		    return execute (val.getValue ());
		);
	    }
	    } catch (Error::ErrorList list) {
		
		errors.insert (errors.begin (), list.errors.begin (), list.errors.end ());
		if (nb_recurs <= 3 || global::State::instance ().isVerboseActive ()) {
		    errors.insert (errors.begin (), Ymir::Error::createNote (gen.getLocation (), ExternalError::get (IN_COMPILE_TIME_EXEC)));		    
		} else {
		    if (nb_recurs == 4) {
			errors.push_back (Ymir::Error::createNoteOneLine ("    : %(B) ", "..."));
			errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (OTHER_CALL)));		    	
		    }
		}
	    } 

	    nb_recurs -= 1;
	    
	    if (errors.size () != 0)
		throw Error::ErrorList {errors};
	    else {
		Ymir::Error::occur (
		    gen.getLocation (),
		    ExternalError::get (COMPILE_TIME_UNKNOWN)
		);
	    }

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
	    if (!index.is <Fixed> ())
		Ymir::Error::occur (
		    index.getLocation (),
		    ExternalError::get (COMPILE_TIME_UNKNOWN)
		); 
	    if (!array.is <ArrayValue> ())
		Ymir::Error::occur (
		    array.getLocation (),
		    ExternalError::get (COMPILE_TIME_UNKNOWN)
		); 
	    
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
	    auto leftEx = this-> execute (binInt.getLeft ());
	    auto rightEx = this-> execute (binInt.getRight ());
	    if (!leftEx.is<Fixed> ())
		Ymir::Error::occur (
		    leftEx.getLocation (),
		    ExternalError::get (COMPILE_TIME_UNKNOWN)
		);
	    
	    if (!rightEx.is<Fixed> ())
		Ymir::Error::occur (
		    rightEx.getLocation (),
		    ExternalError::get (COMPILE_TIME_UNKNOWN)
		);
	    
	    auto left = leftEx.to<Fixed> ().getUI ();
	    auto right = rightEx.to<Fixed> ().getUI ();
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
	    auto elemEx = this-> execute (unaInt.getOperand ());
	    if (!elemEx.is<Fixed> ())
		Ymir::Error::occur (
		    elemEx.getLocation (),
		    ExternalError::get (COMPILE_TIME_UNKNOWN)
		);
	    
	    auto elem = elemEx.to <Fixed> ().getUI ();
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
	    } else {
		Ymir::Error::occur (
		    unaInt.getLocation (),
		    ExternalError::get (COMPILE_TIME_UNKNOWN)
		);
		return Generator::empty ();
	    }
	}

	bool applyUnaBool (Unary::Operator op, bool v) {
	    switch (op) {
	    case Unary::Operator::NOT : return !v;
	    default :
		Ymir::Error::halt ("%(r) - unhandeld case", "Critical");
		return bool ();
	    }
	}

	generator::Generator CompileTime::executeUnaryBool (const generator::UnaryBool & unaBool) { 
	    auto elemEx = this-> execute (unaBool.getOperand ());
	    
	    auto elem = elemEx.to <BoolValue> ();
	    if (unaBool.getType ().is <Bool> ()) {
		std::string type = unaBool.getType ().prettyString ();			       		
		auto result = applyUnaBool (unaBool.getOperator (), elem.getValue ());
				
		return BoolValue::init (unaBool.getLocation (), unaBool.getType (), result);
	    } else {
		Ymir::Error::occur (
		    unaBool.getLocation (),
		    ExternalError::get (COMPILE_TIME_UNKNOWN)
		);
		return Generator::empty ();
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
	    auto leftEx = this-> execute (binFloat.getLeft ());
	    auto rightEx = this-> execute (binFloat.getRight ());
	    if (!leftEx.is<FloatValue> ())
		Ymir::Error::occur (
		    leftEx.getLocation (),
		    ExternalError::get (COMPILE_TIME_UNKNOWN)
		);
	    if (!rightEx.is<FloatValue> ())
		Ymir::Error::occur (
		    rightEx.getLocation (),
		    ExternalError::get (COMPILE_TIME_UNKNOWN)
		);

	    
	    auto left = strtod (leftEx.to<FloatValue> ().getValue ().c_str (), NULL);
	    auto right = strtod (rightEx.to<FloatValue> ().getValue ().c_str (), NULL);
	    
	    if (binFloat.getType ().is<Float> ()) {
		auto res = applyBinFloat (binFloat.getOperator (), left, right);		
		return FloatValue::init (binFloat.getLocation (), binFloat.getType (), format ("%", res));
	    } else {
		auto res = applyBinFloatBool<double> (binFloat.getOperator (), left, right);
		return BoolValue::init (binFloat.getLocation (), Bool::init (binFloat.getLocation ()), res);
	    }
	}

	bool applyBinBool (Binary::Operator op, bool left, bool right) {
	    switch (op) {
	    case Binary::Operator::EQUAL : return left == right;
	    case Binary::Operator::NOT_EQUAL : return left != right;
	    case Binary::Operator::AND : return left && right;
	    case Binary::Operator::OR : return left || right;
	    default :
		Ymir::Error::halt ("%(r) - unhandeld case", "Critical");
		return false;
	    }
	}

	generator::Generator CompileTime::executeBinaryBool (const generator::BinaryBool & binBool) {
	    auto leftEx = this-> execute (binBool.getLeft ());
	    if (!leftEx.is <BoolValue> ())
		Ymir::Error::occur (
		    leftEx.getLocation (),
		    ExternalError::get (COMPILE_TIME_UNKNOWN)
		);

	    if (binBool.getOperator () == Binary::Operator::AND && !leftEx.to <BoolValue> ().getValue ()) {
		return BoolValue::init (binBool.getLocation (), Bool::init (binBool.getLocation ()), false);
	    }
	    
	    auto rightEx = this-> execute (binBool.getRight ());
	    if (!rightEx.is<BoolValue> ())
		Ymir::Error::occur (
		    leftEx.getLocation (),
		    ExternalError::get (COMPILE_TIME_UNKNOWN)
		);
	    
	    auto res = applyBinBool (binBool.getOperator (), leftEx.to <BoolValue> ().getValue (), rightEx.to <BoolValue> ().getValue ());
	    return BoolValue::init (binBool.getLocation (), Bool::init (binBool.getLocation ()), res);
	}

	generator::Generator CompileTime::executeConditional (const generator::Conditional & conditional) {
	    auto test = this-> execute (conditional.getTest ());
	    if (test.isEmpty () || !test.is <BoolValue> ())
		Ymir::Error::occur (
		    conditional.getTest ().getLocation (),
		    ExternalError::get (COMPILE_TIME_UNKNOWN)
		);

	    auto isTrue = test.to <BoolValue> ().getValue ();
	    if (isTrue) {
		return this-> execute (conditional.getContent ());		
	    } else if (!conditional.getElse ().isEmpty ())
		return this-> execute (conditional.getElse ());

	    Ymir::Error::occur (
		conditional.getLocation (),
		ExternalError::get (COMPILE_TIME_UNKNOWN)
	    );
	    return Generator::empty ();
	}

	generator::Generator CompileTime::executeSet (const generator::Set & set) {
	    Ymir::Error::occur (
		set.getLocation (),
		ExternalError::get (COMPILE_TIME_UNKNOWN)
	    );
	    return Generator::empty ();
	}

	generator::Generator CompileTime::executeBlock (const generator::Block & block) {
	    if (block.getType ().is <Void> ()) {
		Ymir::Error::occur (
		    block.getLocation (),
		    ExternalError::get (COMPILE_TIME_UNKNOWN)
		);
		return Generator::empty ();
	    }
	    else return this-> execute (block.getContent ().back ());			       
	}
	
	generator::Generator CompileTime::executeVarDecl (const generator::VarDecl & decl) {
	    Ymir::Error::occur (
		decl.getLocation (),
		ExternalError::get (COMPILE_TIME_UNKNOWN)
	    );
	    return Generator::empty ();
	}

	generator::Generator CompileTime::executeVarRef (const generator::VarRef & ref) {
	    if (ref.getValue ().isEmpty ()) {
		Ymir::Error::occur (
		    ref.getLocation (),
		    ExternalError::get (COMPILE_TIME_UNKNOWN)
		);
		return Generator::empty ();
	    } else return this-> execute (ref.getValue ());
	}

	generator::Generator CompileTime::executeCall (const generator::Call & call) {
	    if (call.getParameters ().size () != 0 || !call.getFrame ().is <FrameProto> ()) {
		Ymir::Error::occur (
		    call.getLocation (),
		    ExternalError::get (COMPILE_TIME_UNKNOWN)
		);
		return Generator::empty ();
	    } else {
		return this-> executeFrame (call.getFrame ().to <generator::FrameProto> ());
	    }
	}

	generator::Generator CompileTime::executeFrame (const generator::FrameProto & fr) {
	    static int nb_recurse = 0;
	    for (auto & it : this-> _knownValues) {
		if (fr.equals (it.first)) return it.second;
	    }
	    
	    auto & frame = this-> _context.retreiveFrameFromProto (fr);
	    if (!frame.isEmpty ()) {
		nb_recurse += 1;
		if (nb_recurse >= CompileConstante::LIMIT_RECURSE_CALL) {
		    Ymir::Error::occur (fr.getLocation (), ExternalError::get (CALL_RECURSION));
		}
		auto ret = this-> execute (frame.to<Frame> ().getContent ());
		this-> _knownValues.push_back (std::pair <Generator, Generator> (fr.clone (), ret));
		nb_recurse -= 1;
		return ret;
	    } else {
		Ymir::Error::occur (
		    fr.getLocation (),
		    ExternalError::get (COMPILE_TIME_UNKNOWN)
		);
		return Generator::empty ();
	    }
	}

	generator::Generator CompileTime::executeAddresser (const generator::Generator & addr) {
	    if (addr.to<Value> ().getType ().is<FuncPtr> () && addr.to<Addresser> ().getWho ().is <FrameProto> ()) {
		return addr;
	    } else return Generator::empty ();
	}


	generator::Generator CompileTime::executeAlias (const generator::Generator & alias) {
	    auto inner = alias.to <Aliaser> ().getWho ();
	    auto ret = this-> execute (inner);
	    if (!ret.isEmpty ())
		return alias;
	    else return Generator::empty ();
	}
	
	generator::Generator CompileTime::executeLamdaProto (const generator::Generator & gen) {
	    if (gen.to <LambdaProto> ().isRefClosure () || gen.to<LambdaProto> ().isMoveClosure ()) {
		Ymir::Error::occur (
		    gen.getLocation (),
		    ExternalError::get (COMPILE_TIME_UNKNOWN)
		);
		return Generator::empty ();
	    }
	    return gen;
	}

	generator::Generator CompileTime::executeTemplateRef (const generator::Generator & gen) {
	    if (!gen.to <TemplateRef> ().getTemplateRef ().to <semantic::Template> ().getDeclaration ().is <syntax::Function> ()) {
		Ymir::Error::occur (
		    gen.getLocation (),
		    ExternalError::get (COMPILE_TIME_UNKNOWN)
		);
		return Generator::empty ();
	    }
	    return gen;
	}


	
	generator::Generator CompileTime::executeMultSym (const generator::Generator & gen) {
	    std::vector <Generator> mult;
	    for (auto & it : gen.to <MultSym> ().getGenerators ()) {
		try {
		    auto ret = this-> execute (it);
		    mult.push_back (ret);
		} catch (Error::ErrorList list) {		    
		} 
	    }

	    if (mult.size () == 0)
		Ymir::Error::occur (
		    gen.getLocation (),
		    ExternalError::get (COMPILE_TIME_UNKNOWN)
		);

	    return MultSym::init (gen.getLocation (), mult);
	}

	generator::Generator CompileTime::executeFrameProto (const generator::Generator & proto) {
	    auto params = proto.to <FrameProto> ().getParameters ();
	    auto ret = proto.to <FrameProto> ().getReturnType ();
	    std::vector <Generator> paramTypes;
	    for (auto & it : params) {
		paramTypes.push_back (it.to <generator::ProtoVar> ().getType ());
	    }
	    
	    auto funcType = FuncPtr::init (proto.getLocation (), ret, paramTypes);
	    return Addresser::init (proto.getLocation (), funcType, proto);
	}
	
    }
   
    
}
