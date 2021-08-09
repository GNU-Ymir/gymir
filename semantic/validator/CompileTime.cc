#include <ymir/semantic/validator/CompileTime.hh>
#include <ymir/semantic/generator/Visitor.hh>
#include <ymir/semantic/validator/UtfVisitor.hh>
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
	    std::list <Ymir::Error::ErrorMsg> errors;
	    try {
		match (gen) {
		    s_of_u (Fixed)
			return gen;
		
		    s_of_u (ArrayValue)
			return gen;
		
		    s_of_u (CharValue)
			return gen;
					     
		    s_of_u (FloatValue)
			return gen;
		
		    s_of_u (BoolValue)
			return gen;
		
		    s_of_u (None)
			return gen;
		
		    s_of_u (StringValue)
			return gen;
		
		    s_of (RangeValue, rng)
			return executeRangeValue (rng);

		    s_of (SliceConcat, slc)
			return executeSliceConcat (slc);

		    s_of (SliceCompare, slc)
			return executeSliceCompare (slc);
		    
		    s_of (Affect, aff)
			return executeAffect (aff);
		
		    s_of (ArrayAccess, arr)
			return executeArrayAccess (arr);		

		    s_of (BinaryInt, bin)
			return executeBinaryInt (bin);
		    
		    s_of (BinaryChar, bin)
			return executeBinaryChar (bin);		

		    s_of (BinaryBool, bin)
			return executeBinaryBool (bin);		

		    s_of (UnaryInt, una)
			return executeUnaryInt (una);		

		    s_of (UnaryBool, ub)
			return executeUnaryBool (ub);		
			 
		    s_of (BinaryFloat, fl)
			return executeBinaryFloat (fl);		

		    s_of (Conditional, cd)
			return executeConditional (cd);		

		    s_of (Set, st)
			return executeSet (st);		

		    s_of (Block, bl)
			return executeBlock (bl);		
			 
		    s_of (generator::VarDecl, vdecl)
			return executeVarDecl (vdecl);		

		    s_of (VarRef, vref)
			return executeVarRef (vref);		
		
		    s_of_u (Addresser)
			return executeAddresser (gen);		

		    s_of_u (Aliaser)
			return executeAlias (gen);		
		
		    s_of (Call, cll)
			return executeCall (cll);	       

		    s_of_u (LambdaProto)
			return executeLamdaProto (gen);		

		    s_of_u (TemplateRef)
			return executeTemplateRef (gen);		

		    s_of_u (FrameProto)
			return executeFrameProto (gen);		
		
		    s_of_u (MultSym)
			return executeMultSym (gen);		

		    s_of (UniqValue, val)
			return execute (val.getValue ());

		    s_of_u (SizeOf)
			return executeSizeOf (gen);		    
		}	    
	    } catch (Error::ErrorList list) {
		Ymir::Error::occurAndNote (
		    gen.getLocation (),
		    list.errors, 
		    ExternalError::COMPILE_TIME_UNKNOWN
		    );	
	    }

	    Ymir::Error::occur (
		gen.getLocation (),
		ExternalError::COMPILE_TIME_UNKNOWN
		);	

	    return gen;
	}

	generator::Generator CompileTime::executeRangeValue (const generator::RangeValue & rng) {
	    auto left = execute (rng.getLeft ());
	    auto right = execute (rng.getRight ());
	    auto full = execute (rng.getIsFull ());
	    auto step = rng.getStep ();
	    if (!step.isEmpty ())
		step = execute (step);
	    
	    return RangeValue::init (rng.getLocation (), rng.getType (), left, right, step, full);
	}

	generator::Generator CompileTime::executeAffect (const generator::Affect & gen) {
	    Ymir::Error::occur (
		gen.getLocation (),
		ExternalError::AFFECT_COMPILE_TIME
	    );
	    return Generator::empty ();
	}

	generator::Generator CompileTime::executeSliceConcat (const generator::SliceConcat & slc) {
	    auto left = execute (slc.getLeft ());
	    auto right = execute (slc.getRight ());
	    if (left.is<Aliaser> ()) left = left.to <Aliaser> ().getWho ();
	    if (right.is<Aliaser> ()) right = right.to <Aliaser> ().getWho ();
	    
	    if (left.is <StringValue> () && right.is<StringValue> ()) {
		auto res = left.to <StringValue> ().getValue ();
		for (int i = 0 ; i < slc.getType ().to <Type> ().getInners () [0].to <Char> ().getSize () / 8; i++) {
		    res.pop_back ();
		}
		
		res.insert (res.end (), right.to <StringValue> ().getValue ().begin (), right.to <StringValue> ().getValue ().end ());


		auto inner = slc.getType ().to <Type> ().getInners ()[0];
		uint size = inner.to <Char> ().getSize ();
		auto len = res.size () / (size / 8);

		auto type = Array::init (slc.getLocation (), inner, len);
		type = Type::init (type.to <Type> (), true);

		auto sliceType = Slice::init (slc.getLocation (), inner);
		sliceType = Type::init (sliceType.to <Type> (), true);		
		
		return Aliaser::init (
		    slc.getLocation (),
		    sliceType,
		    StringValue::init (slc.getLocation (), type, res, len)
		    );	       
	    }
	    
	    Ymir::Error::occur (
		slc.getLocation (),
		ExternalError::COMPILE_TIME_UNKNOWN
		); 	    
	    return Generator::empty ();
	}

	
       	
	generator::Generator CompileTime::executeArrayAccess (const generator::ArrayAccess & acc) {
	    auto array = this-> execute (acc.getArray ());
	    auto index = this-> execute (acc.getIndex ());
	    if (!index.is <Fixed> ())
		Ymir::Error::occur (
		    index.getLocation (),
		    ExternalError::COMPILE_TIME_UNKNOWN
		); 
	    if (!array.is <ArrayValue> ())
		Ymir::Error::occur (
		    array.getLocation (),
		    ExternalError::COMPILE_TIME_UNKNOWN
		); 
	    
	    return array.to<ArrayValue> ().getContent ()[index.to<Fixed> ().getUI ().u];
	}

	ulong getMaxU (const Integer & type) {
	    switch (type.getSize ()) {
	    case 8 : return UCHAR_MAX;
	    case 16 : return USHRT_MAX;
	    case 32 : return UINT_MAX;
	    case 64 : return ULONG_MAX;
	    default : return ULONG_MAX;
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
	    default : return LONG_MAX;
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
	    default : return LONG_MIN;
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
		    ExternalError::COMPILE_TIME_UNKNOWN
		);
	    
	    if (!rightEx.is<Fixed> ())
		Ymir::Error::occur (
		    rightEx.getLocation (),
		    ExternalError::COMPILE_TIME_UNKNOWN
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
		    Ymir::Error::occur (binInt.getLocation (), ExternalError::OVERFLOW, type, result.i);
		else if (!isSigned && (result.u > maxU || result.u < minU))
		    Ymir::Error::occur (binInt.getLocation (), ExternalError::OVERFLOW, type, result.u);
		
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
		    ExternalError::COMPILE_TIME_UNKNOWN
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
		    Ymir::Error::occur (unaInt.getLocation (), ExternalError::OVERFLOW, type, result.i);
		else if (!isSigned && (result.u > maxU || result.u < minU))
		    Ymir::Error::occur (unaInt.getLocation (), ExternalError::OVERFLOW, type, result.u);

		return Fixed::init (unaInt.getLocation (), unaInt.getType (), result);
	    } else {
		Ymir::Error::occur (
		    unaInt.getLocation (),
		    ExternalError::COMPILE_TIME_UNKNOWN
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
		    ExternalError::COMPILE_TIME_UNKNOWN
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
		    ExternalError::COMPILE_TIME_UNKNOWN
		);
	    if (!rightEx.is<FloatValue> ())
		Ymir::Error::occur (
		    rightEx.getLocation (),
		    ExternalError::COMPILE_TIME_UNKNOWN
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


	template <typename T>
	T applyBinChar (Binary::Operator op, T left, T right) {
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
	bool applyBinCharBool (Binary::Operator op, T left, T right) {
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
	
	generator::Generator CompileTime::executeBinaryChar (const generator::BinaryChar & binChar) {
	    auto leftEx = this-> execute (binChar.getLeft ());
	    auto rightEx = this-> execute (binChar.getRight ());
	    if (!leftEx.is<CharValue> ())
		Ymir::Error::occur (
		    leftEx.getLocation (),
		    ExternalError::COMPILE_TIME_UNKNOWN
		);
	    if (!rightEx.is<CharValue> ())
		Ymir::Error::occur (
		    rightEx.getLocation (),
		    ExternalError::COMPILE_TIME_UNKNOWN
		);

	    
	    auto left = leftEx.to<CharValue> ().getValue ();
	    auto right = rightEx.to<CharValue> ().getValue ();
	    if (binChar.getType ().is<Char> ()) {
		auto res = applyBinChar (binChar.getOperator (), left, right);		
		return CharValue::init (binChar.getLocation (), binChar.getType (), res);
	    } else {
		auto res = applyBinCharBool (binChar.getOperator (), left, right);
		return BoolValue::init (binChar.getLocation (), Bool::init (binChar.getLocation ()), res);
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
		    ExternalError::COMPILE_TIME_UNKNOWN
		);

	    if (binBool.getOperator () == Binary::Operator::AND && !leftEx.to <BoolValue> ().getValue ()) {
		return BoolValue::init (binBool.getLocation (), Bool::init (binBool.getLocation ()), false);
	    }
	    
	    auto rightEx = this-> execute (binBool.getRight ());
	    if (!rightEx.is<BoolValue> ())
		Ymir::Error::occur (
		    leftEx.getLocation (),
		    ExternalError::COMPILE_TIME_UNKNOWN
		);
	    
	    auto res = applyBinBool (binBool.getOperator (), leftEx.to <BoolValue> ().getValue (), rightEx.to <BoolValue> ().getValue ());
	    return BoolValue::init (binBool.getLocation (), Bool::init (binBool.getLocation ()), res);
	}

	generator::Generator CompileTime::executeConditional (const generator::Conditional & conditional) {
	    auto test = this-> execute (conditional.getTest ());
	    if (test.isEmpty () || !test.is <BoolValue> ())
		Ymir::Error::occur (
		    conditional.getTest ().getLocation (),
		    ExternalError::COMPILE_TIME_UNKNOWN
		);

	    auto isTrue = test.to <BoolValue> ().getValue ();
	    if (isTrue) {
		return this-> execute (conditional.getContent ());		
	    } else if (!conditional.getElse ().isEmpty ())
		return this-> execute (conditional.getElse ());

	    Ymir::Error::occur (
		conditional.getLocation (),
		ExternalError::COMPILE_TIME_UNKNOWN
	    );
	    return Generator::empty ();
	}

	int strcmp (const std::vector <char> & left, const std::vector <char> & right) {	    
	    uint i = 0;
	    while (i < left.size () && i < right.size ()) {
		if (left [i] != right [i]) break;
		else i += 1;
	    }
	    return (int) left [i] - (int) right [i];
	}
	
	generator::Generator CompileTime::executeSliceCompare (const generator::SliceCompare & slc) {
	    auto left = execute (slc.getLeft ());
	    auto right = execute (slc.getRight ());
	    if (left.is<Aliaser> ()) left = left.to <Aliaser> ().getWho ();
	    if (right.is<Aliaser> ()) right = right.to <Aliaser> ().getWho ();
	    
	    if (left.is <StringValue> () && right.is<StringValue> ()) {
		auto size = left.to <StringValue> ().getType ().to <Type> ().getInners ()[0].to <Char> ().getSize ();
		auto v = 0;
		if (size == 32) {
		    auto & lft = left.to <StringValue> ().getValue ();
		    auto & rgt = right.to <StringValue> ().getValue ();		
		    v = strcmp (UtfVisitor::utf32_to_utf8 (lft), UtfVisitor::utf32_to_utf8 (rgt));
		} else {
		    auto & lft = left.to <StringValue> ().getValue ();
		    auto & rgt = right.to <StringValue> ().getValue ();		
		    v = strcmp (lft, rgt);
		}		

		auto b = applyBinIntBool<int> (slc.getOperator (), v, 0);
		return BoolValue::init (slc.getLocation (), Bool::init (slc.getLocation ()), b);
	    }

	    Ymir::Error::occur (
		slc.getLocation (),
		ExternalError::COMPILE_TIME_UNKNOWN
		); 	    
	    return Generator::empty ();
	}

	
	generator::Generator CompileTime::executeSet (const generator::Set & set) {
	    Ymir::Error::occur (
		set.getLocation (),
		ExternalError::COMPILE_TIME_UNKNOWN
	    );
	    return Generator::empty ();
	}

	generator::Generator CompileTime::executeBlock (const generator::Block & block) {
	    if (block.getType ().is <Void> ()) {
		Ymir::Error::occur (
		    block.getLocation (),
		    ExternalError::COMPILE_TIME_UNKNOWN
		);
		return Generator::empty ();
	    }
	    else return this-> execute (block.getContent ().back ());			       
	}
	
	generator::Generator CompileTime::executeVarDecl (const generator::VarDecl & decl) {
	    Ymir::Error::occur (
		decl.getLocation (),
		ExternalError::COMPILE_TIME_UNKNOWN
	    );
	    return Generator::empty ();
	}

	generator::Generator CompileTime::executeVarRef (const generator::VarRef & ref) {
	    if (ref.getValue ().isEmpty ()) {
		Ymir::Error::occur (
		    ref.getLocation (),
		    ExternalError::COMPILE_TIME_UNKNOWN
		);
		return Generator::empty ();
	    } else return this-> execute (ref.getValue ());
	}

	generator::Generator CompileTime::executeCall (const generator::Call & call) {
	    if (call.getParameters ().size () != 0 || !call.getFrame ().is <FrameProto> ()) {
		Ymir::Error::occur (
		    call.getLocation (),
		    ExternalError::COMPILE_TIME_UNKNOWN
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
		    Ymir::Error::occur (fr.getLocation (), ExternalError::CALL_RECURSION);
		}
		auto ret = this-> execute (frame.to<Frame> ().getContent ());
		this-> _knownValues.push_back (std::pair <Generator, Generator> (fr.clone (), ret));
		nb_recurse -= 1;
		return ret;
	    } else {
		Ymir::Error::occur (
		    fr.getLocation (),
		    ExternalError::COMPILE_TIME_UNKNOWN
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
		    ExternalError::COMPILE_TIME_UNKNOWN
		);
		return Generator::empty ();
	    }
	    return gen;
	}

	generator::Generator CompileTime::executeTemplateRef (const generator::Generator & gen) {
	    if (!gen.to <TemplateRef> ().getTemplateRef ().to <semantic::Template> ().getDeclaration ().is <syntax::Function> ()) {
		Ymir::Error::occur (
		    gen.getLocation (),
		    ExternalError::COMPILE_TIME_UNKNOWN
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
		    ExternalError::COMPILE_TIME_UNKNOWN
		);

	    return MultSym::init (gen.getLocation (), mult);
	}

	generator::Generator CompileTime::executeSizeOf (const generator::Generator & sof) {
	    auto type = sof.to <SizeOf> ().getWho ();
	    auto genVisitor = generator::Visitor::init ();	    
	    auto treeType = genVisitor.generateType (type);
	    Fixed::UI result;
	    result.u = treeType.getSize ();
	    auto itype = Integer::init (sof.getLocation (), 0, false);
	    return Fixed::init (sof.getLocation (), itype, result);
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
