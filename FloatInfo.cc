#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/utils/FloatUtils.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/value/_.hh>
#include <cfloat>
#include <limits>
#include <math.h>

namespace semantic {

    using namespace syntax;

	
    IFloatInfo::IFloatInfo (bool isConst, FloatConst type) :
	IInfoType (isConst),
	_type (type)
    {}

    bool IFloatInfo::isSame (InfoType other) {
	if (auto ot = other-> to<IFloatInfo> ()) {
	    if (ot-> _type == this-> _type) return true;
	}
	return false;
    }
    
    InfoType IFloatInfo::BinaryOp (Word op, syntax::Expression right) {
	if (op == Token::EQUAL) return Affect (right);
	if (op == Token::DIV_AFF) return opAff (op, right);
	if (op == Token::STAR_EQUAL) return opAff (op, right);
	if (op == Token::MINUS_AFF) return opAff (op, right);
	if (op == Token::PLUS_AFF) return opAff (op, right);
	if (op == Token::INF) return opTest (op, right);
	if (op == Token::SUP) return opTest (op, right);
	if (op == Token::INF_EQUAL) return opTest (op, right);
	if (op == Token::SUP_EQUAL) return opTest (op, right);
	if (op == Token::NOT_EQUAL) return opTest (op, right);
	if (op == Token::NOT_INF_EQUAL) return opTest (op, right);
	if (op == Token::NOT_SUP_EQUAL) return opTest (op, right);
	if (op == Token::DEQUAL) return opTest (op, right);
	if (op == Token::PLUS) return opNorm (op, right);
	if (op == Token::MINUS) return opNorm (op, right);
	if (op == Token::DIV) return opNorm (op, right);
	if (op == Token::STAR) return opNorm (op, right);
	return NULL;
    } 
	
    InfoType IFloatInfo::BinaryOpRight (Word op, syntax::Expression left) {
	if (op == Token::EQUAL) return AffectRight (left);
	if (op == Token::INF) return opTestRight (op, left);
	if (op == Token::SUP) return opTestRight (op, left);
	if (op == Token::INF_EQUAL) return opTestRight (op, left);
	if (op == Token::SUP_EQUAL) return opTestRight (op, left);
	if (op == Token::NOT_EQUAL) return opTestRight (op, left);
	if (op == Token::DEQUAL) return opTestRight (op, left);
	if (op == Token::PLUS) return opNormRight (op, left);
	if (op == Token::MINUS) return opNormRight (op, left);
	if (op == Token::DIV) return opNormRight (op, left);
	if (op == Token::STAR) return opNormRight (op, left);
	return NULL;	
    }

    InfoType IFloatInfo::UnaryOp (Word op) {
	if (op == Token::MINUS) return Inv (op);
	else if (op == Token::AND && this-> isLvalue ()) return toPtr (op);
	return NULL;
    }

    InfoType IFloatInfo::toPtr (Word &) {
	auto ret = new (Z0)  IPtrInfo (this-> isConst (), this-> clone ());
	ret-> binopFoo = &FloatUtils::InstAddr;
	return ret;
    }
    
    InfoType IFloatInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	return NULL;
    }

    InfoType IFloatInfo::DColonOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "init") return Init ();
	if (var-> token == "max") return Max ();
	if (var-> token == "min") return Min ();
	if (var-> token == "nan") return Nan ();
	if (var-> token == "dig") return Dig ();
	if (var-> token == "infinity") return Infinity ();
	if (var-> token == "epsilon") return Epsilon ();
	if (var-> token == "mant_dig") return MantDig ();
	if (var-> token == "max_10_exp") return Max10Exp ();
	if (var-> token == "max_exp") return MaxExp ();
	if (var-> token == "min_10_exp") return Min10Exp ();
	if (var-> token == "min_exp") return MinExp ();
	if (var-> token == "sizeof") return SizeOf ();
	if (var-> token == "typeid") return StringOf ();
	return NULL;
    }
    
    InfoType IFloatInfo::CastOp (InfoType other) {
	if (auto ot = other-> to<IFloatInfo> ()) {
	    if (ot-> _type == this-> _type) return this;
	    else {
		auto aux = other-> clone ();
		aux-> isConst (this-> isConst ());
		aux-> binopFoo = FloatUtils::InstCast;
		return aux;
	    }
	} else if (auto ot = other-> to<IFixedInfo> ()) {
	    if (this-> _type == FloatConst::DOUBLE) {
		if (ot-> type () != FixedConst::LONG &&
		    ot-> type () != FixedConst::ULONG) return NULL;
	    } else if (ot-> type () < FixedConst::INT) return NULL;
	    auto aux = ot-> clone ();
	    aux-> isConst (this-> isConst ());
	    aux-> binopFoo = FloatUtils::InstCast;
	    return aux;
	}
	return NULL;
    }

    InfoType IFloatInfo::CompOp (InfoType other) {
	if (other-> is<IUndefInfo> ()) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = FloatUtils::InstCast;
	    return ret;
	} else if (auto ot = other-> to<IFloatInfo> ()) {
	    if (ot-> _type >= this-> _type) {
		auto ret = ot-> clone ();
		ret-> binopFoo = FloatUtils::InstCast;
		return ret;
	    }
	} else if (other-> is<IRefInfo> () && this-> isLvalue ()) {
	    auto aux = new (Z0)  IRefInfo (this-> isConst (), this-> clone ());
	    aux-> binopFoo = FloatUtils::InstAddr;
	    return aux;
	} else if (auto en = other-> to<IEnumInfo> ()) {
	    return this-> CompOp (en-> content ());
	}
	return NULL;
    }
	
    std::string IFloatInfo::innerTypeString () {
	if (this-> _type == FloatConst::FLOAT) return "f32";
	else return "f64";
    }

    std::string IFloatInfo::innerSimpleTypeString () {
	if (this-> _type == FloatConst::FLOAT) return "f";
	else return "d";
    }

    InfoType IFloatInfo::onClone () {
	auto ret = new (Z0)  IFloatInfo (this-> isConst (), this-> _type);
	if (this-> value ())
	    ret-> value () = this-> value ()-> clone ();
	return ret;
    }

    const char* IFloatInfo::getId () {
	return IFloatInfo::id ();
    }
	
    InfoType IFloatInfo::Affect (syntax::Expression right) {
	if (auto ot = right-> info-> type ()-> to<IFloatInfo> ()) {
	    if (this-> _type >= ot-> _type) {
		auto f = new (Z0)  IFloatInfo (false, this-> _type);
		f-> binopFoo = &FloatUtils::InstAffect;
		return f;
	    } else if (right-> info-> type ()-> is<IFixedInfo> ()) {
		auto f = new (Z0)  IFloatInfo (false, this-> _type);
		f-> binopFoo = &FloatUtils::InstAffect;
		return f;
	    }
	}
	return NULL;
    }

    InfoType IFloatInfo::AffectRight (syntax::Expression left) {
	if (left-> info-> type ()-> is<IUndefInfo> ()) {
	    auto fl = new (Z0)  IFloatInfo (false, this-> _type);
	    fl-> binopFoo = &FloatUtils::InstAffect;
	    return fl;
	}
	return NULL;
    }

    InfoType IFloatInfo::Inv (const Word& op) {
	auto ret = new (Z0) IFloatInfo (true, this-> _type);
	ret-> unopFoo = &FloatUtils::UnaryMinus;
	if (this-> value ())
	    ret-> value () = this-> value ()-> UnaryOp (op);	
	return ret;
    }

    InfoType IFloatInfo::Init () {
	auto ret = this-> cloneConst ();
	ret-> value () = new (Z0) IFloatValue (this-> _type, NAN, NAN);
	return ret;
    }

    InfoType IFloatInfo::Max () {
	auto ret = this-> cloneConst ();
	if (this-> _type == FloatConst::FLOAT) {
	    ret-> value () = new (Z0) IFloatValue (this-> _type, FLT_MAX, 0.0);
	} else ret-> value () = new (Z0) IFloatValue (this-> _type, 0.0f, DBL_MAX);
	return ret;
    }

    InfoType IFloatInfo::Infinity () {
	auto ret = this-> cloneConst ();
	if (this-> _type == FloatConst::FLOAT) {
	    ret-> value () = new (Z0) IFloatValue (this-> _type, std::numeric_limits<float>::infinity (), 0.0);
	} else ret-> value () = new (Z0) IFloatValue (this-> _type, 0.0f, std::numeric_limits<double>::infinity ());
	return ret;
    }
    
    InfoType IFloatInfo::Min () {
	auto ret = this-> cloneConst ();
	if (this-> _type == FloatConst::FLOAT) {
	    ret-> value () = new (Z0) IFloatValue (this-> _type, FLT_MIN, 0.0);
	} else ret-> value () = new (Z0) IFloatValue (this-> _type, 0.0f, DBL_MIN);
	return ret;
    }

    InfoType IFloatInfo::Nan () {
	auto ret = this-> cloneConst ();
	if (this-> _type == FloatConst::FLOAT) {
	    ret-> value () = new (Z0) IFloatValue (this-> _type, NAN, 0.0);
	} else ret-> value () = new (Z0) IFloatValue (this-> _type, 0.0, NAN);
	return ret;
    }

    InfoType IFloatInfo::Dig () {
	auto ret = new (Z0) IFixedInfo (true, FixedConst::INT);
	if (this-> _type == FloatConst::FLOAT) {
	    ret-> value () = new (Z0) IFixedValue (FixedConst::INT, FLT_DIG, FLT_DIG);
	} else ret-> value () = new (Z0) IFixedValue (FixedConst::INT, DBL_DIG, DBL_DIG);
	return ret;
    }

    InfoType IFloatInfo::Epsilon () {
	auto ret = this-> cloneConst ();
	if (this-> _type == FloatConst::FLOAT) {
	    ret-> value () = new (Z0) IFloatValue (this-> _type, FLT_EPSILON, 0.0);
	} else ret-> value () = new (Z0) IFloatValue (this-> _type, 0.0f, DBL_EPSILON);
	return ret;
    }

    InfoType IFloatInfo::MantDig () {
	auto ret = new (Z0) IFixedInfo (true, FixedConst::INT);
	if (this-> _type == FloatConst::FLOAT) {
	    ret-> value () = new (Z0) IFixedValue (FixedConst::INT, FLT_MANT_DIG, 0);
	} else ret-> value () = new (Z0) IFixedValue (FixedConst::INT, 0, DBL_MANT_DIG);
	return ret;
    }
	
    InfoType IFloatInfo::Max10Exp () {
	auto ret = new (Z0) IFixedInfo (true, FixedConst::INT);
	if (this-> _type == FloatConst::FLOAT) {
	    ret-> value () = new (Z0) IFixedValue (FixedConst::INT, FLT_MAX_10_EXP, 0);
	} else ret-> value () = new (Z0) IFixedValue (FixedConst::INT, 0, DBL_MAX_10_EXP);
	return ret;
    }

    InfoType IFloatInfo::MaxExp () {
	auto ret = new (Z0) IFixedInfo (true, FixedConst::INT);
	if (this-> _type == FloatConst::FLOAT) {
	    ret-> value () = new (Z0) IFixedValue (FixedConst::INT, FLT_MAX_EXP, 0);
	} else ret-> value () = new (Z0) IFixedValue (FixedConst::INT, 0, DBL_MAX_EXP);
	return ret;
    }

    InfoType IFloatInfo::MinExp () {
	auto ret = new (Z0) IFixedInfo (true, FixedConst::INT);
	if (this-> _type == FloatConst::FLOAT) {
	    ret-> value () = new (Z0) IFixedValue (FixedConst::INT, FLT_MIN_EXP, 0);
	} else ret-> value () = new (Z0) IFixedValue (FixedConst::INT, 0, DBL_MIN_EXP);
	return ret;
    }

    InfoType IFloatInfo::Min10Exp () {
	auto ret = new (Z0) IFixedInfo (true, FixedConst::INT);
	if (this-> _type == FloatConst::FLOAT) {
	    ret-> value () = new (Z0) IFixedValue (FixedConst::INT, FLT_MIN_10_EXP, 0);
	} else ret-> value () = new (Z0) IFixedValue (FixedConst::INT, 0, DBL_MIN_10_EXP);
	return ret;
    }

    InfoType IFloatInfo::Inf () {
	auto ret = this-> cloneConst ();
	if (this-> _type == FloatConst::FLOAT) {
	    ret-> value () = new (Z0) IFloatValue (this-> _type, INFINITY, 0.0);
	} else ret-> value () = new (Z0) IFloatValue (this-> _type, 0.0f, INFINITY);
	return ret;
    }
    
    InfoType IFloatInfo::Sqrt () {
	return NULL;
    }

    InfoType IFloatInfo::SizeOf () {
	auto ret = new (Z0)  IFixedInfo (true, FixedConst::UINT);
	ret-> unopFoo = FloatUtils::InstSizeOf;
	return ret;	
    }
    
    InfoType IFloatInfo::opAff (Word, syntax::Expression right) {
	if (auto ot = right-> info-> type ()-> to<IFloatInfo> ()) {
	    if (ot-> _type <= this-> _type) {
		auto ret = this-> clone ();
		ret-> binopFoo = &FloatUtils::InstReaff;
		return ret;
	    }
	}
	return NULL;
    }
    
    InfoType IFloatInfo::opNorm (Word op, syntax::Expression right) {
	if (auto ot = right-> info-> type ()-> to<IFloatInfo> ()) {
	    if (this-> _type >= ot->_type) {
		auto ret = this-> cloneConst ();
		ret-> binopFoo = &FloatUtils::InstNormal;
		if (this-> value ())
		    ret-> value () = this-> value ()-> BinaryOp (op, right-> info-> type ()-> value ());

		return ret;
	    } else {
		auto ret = ot-> cloneConst ();
		ret-> binopFoo = &FloatUtils::InstNormalRight;
		if (this-> value ())
		    ret-> value () = this-> value ()-> BinaryOp (op, right-> info-> type ()-> value ());

		return ret;
	    }
	} else if (right-> info-> type ()-> is<IFixedInfo> ()) {
	    auto fl = this-> cloneConst ();
	    fl-> binopFoo = &FloatUtils::InstNormal;
	    if (this-> value ())
		fl-> value () = this-> value ()-> BinaryOp (op, right-> info-> type ()-> value ());

	    return fl;
	} 
	return NULL;
    }

    InfoType IFloatInfo::opTest (Word op, syntax::Expression right) {
	if (auto ot = right-> info-> type ()-> to<IFloatInfo> ()) {
	    if (this-> _type >= ot-> _type) {
		auto ret = new (Z0)  IBoolInfo (true);
		ret-> binopFoo = FloatUtils::InstTest;
		if (this-> value ())
		    ret-> value () = this-> value ()-> BinaryOp (op, right-> info-> type ()-> value ());

		return ret;
	    } else {
		auto ret = new (Z0)  IBoolInfo (true);
		if (this-> value ())
		    ret-> value () = this-> value ()-> BinaryOp (op, right-> info-> type ()-> value ());

		ret-> binopFoo = FloatUtils::InstTestRight;
		return ret;
	    }
	} else if (right-> info-> type ()-> is<IFixedInfo> ()) {
	    auto ret = new (Z0)  IBoolInfo (true);
	    ret-> binopFoo = FloatUtils::InstTest;
	    if (this-> value ())
		ret-> value () = this-> value ()-> BinaryOp (op, right-> info-> type ()-> value ());

	    return ret;
	}
	return NULL;
    }

    InfoType IFloatInfo::opNormRight (Word, syntax::Expression right) {
	if (right-> info-> type ()-> is<IFixedInfo> ()) {
	    auto fl = this-> cloneConst ();
	    fl-> binopFoo = FloatUtils::InstNormalRight;
	    return fl;
	}
	return NULL;
    }

    InfoType IFloatInfo::opTestRight (Word, syntax::Expression right) {
	if (right-> info-> type ()-> is<IFixedInfo> ()) {
	    auto ret = new (Z0)  IBoolInfo (true);
	    ret-> binopFoo = FloatUtils::InstTestRight;
	    return ret;
	}
	return NULL;
    }
       
    FloatConst IFloatInfo::type () {
	return this-> _type;
    }

    Ymir::Tree IFloatInfo::toGeneric () {
	if (this-> _type == FloatConst::FLOAT) return float_type_node;
	else return double_type_node;
    }
   
    namespace FloatUtils {

	using namespace Ymir;
	using namespace syntax;

	Tree InstAffect (Word locus, InfoType, Expression left, Expression right) {
	    auto ltree = left-> toGeneric ();
	    Ymir::Tree rtree = convert (ltree.getType ().getTree (), right-> toGeneric ().getTree ());
	    
	    return Ymir::buildTree (
		MODIFY_EXPR, locus.getLocus (), ltree.getType (), ltree, rtree
	    );
	}
	
	Tree InstReaff (Word locus, InfoType, Expression left, Expression right) {
	    auto ltree = left-> toGeneric ();
	    Ymir::Tree rtree = convert (ltree.getType ().getTree (), right-> toGeneric ().getTree ());
	    tree_code code = OperatorUtils::toGenericReal (locus);
	    return Ymir::buildTree (
		MODIFY_EXPR, locus.getLocus (), ltree.getType (), ltree,
		Ymir::buildTree (
		    code, locus.getLocus (), ltree.getType (), ltree, rtree
		)
	    );		
	}
	
	Ymir::Tree InstNormal (Word locus, InfoType, Expression left, Expression right) {
	    auto ltree = left-> toGeneric ();
	    Ymir::Tree rtree = convert (ltree.getType ().getTree (), right-> toGeneric ().getTree ());
	    tree_code code = OperatorUtils::toGenericReal (locus);
	    return Ymir::buildTree (
		code, locus.getLocus (), ltree.getType (), ltree, rtree
	    );
	}

	Ymir::Tree InstNormalRight (Word locus, InfoType, Expression left, Expression right) {
	    auto rtree = right-> toGeneric ();
	    Ymir::Tree ltree = convert (rtree.getType ().getTree (), left-> toGeneric ().getTree ());

	    tree_code code = OperatorUtils::toGenericReal (locus);
	    return Ymir::buildTree (
		code, locus.getLocus (), ltree.getType (), ltree, rtree
	    );
	}

	Ymir::Tree InstTest (Word locus, InfoType, Expression left, Expression right) {
	    auto ltree = left-> toGeneric ();
	    Ymir::Tree rtree = convert (ltree.getType ().getTree (), right-> toGeneric ().getTree ());
	    tree_code code = OperatorUtils::toGenericReal (locus);
	    return Ymir::buildTree (
		code, locus.getLocus (), boolean_type_node, ltree, rtree
	    );
	}

	Ymir::Tree InstTestRight (Word locus, InfoType, Expression left, Expression right) {
	    auto rtree = right-> toGeneric ();
	    Ymir::Tree ltree = convert (rtree.getType ().getTree (), left-> toGeneric ().getTree ());

	    tree_code code = OperatorUtils::toGenericReal (locus);
	    return Ymir::buildTree (
		code, locus.getLocus (), boolean_type_node, ltree, rtree
	    );
	}
	
	Ymir::Tree UnaryMinus (Word locus, InfoType, Expression elem) {
	    auto lexp = elem-> toGeneric ();
	    return Ymir::buildTree (
		NEGATE_EXPR, locus.getLocus (), lexp.getType (), lexp
	    );
	}

	Ymir::Tree InstSSub (Word locus, InfoType, Expression elem) {
	    auto lexp = elem-> toGeneric ();
	    return Ymir::buildTree (
		PREDECREMENT_EXPR, locus.getLocus (), lexp.getType (), lexp
	    );
	}
	
	Ymir::Tree InstPPlus (Word locus, InfoType, Expression elem) {
	    auto lexp = elem-> toGeneric ();
	    return Ymir::buildTree (
		PREINCREMENT_EXPR, locus.getLocus (), lexp.getType (), lexp
	    );
	}

	Ymir::Tree InstCast (Word, InfoType typei, Expression elem, Expression) {
	    auto type = typei-> toGeneric ();
	    auto lexp = elem-> toGeneric ();
	    return convert (type.getTree (), lexp.getTree ());	    
	}
	
	Ymir::Tree InstAddr (Word locus, InfoType, Expression elem, Expression) {
	    return Ymir::getAddr (locus.getLocus (), elem-> toGeneric ());
	}

	Ymir::Tree InstSizeOf (Word, InfoType, Expression elem) {	    
	    return TYPE_SIZE_UNIT (elem-> info-> type ()-> toGeneric ().getTree ());
	}
	
    }

    IFloatValue::IFloatValue (FloatConst type, float f, double d) {
	this-> type = type;
	if (this-> type == FloatConst::FLOAT) {
	    this-> value.f = f;	    
	} else this-> value.d = d;
    }
    
    double& IFloatValue::getDouble () {
	return this-> value.d;
    }
    
    float& IFloatValue::getFloat () {
	return this-> value.f;
    }

    Value IFloatValue::BinaryOp (Word op, Value val) {
	if (val == NULL) return NULL;
	if (op == Token::PLUS) return this-> add (val);
	if (op == Token::MINUS) return this-> sub (val);
	if (op == Token::DIV) return this-> div (val);
	if (op == Token::STAR) return this-> mul (val);
	if (op == Token::INF) return this-> inf (val);
	if (op == Token::SUP) return this-> sup (val);
	if (op == Token::INF_EQUAL) return this-> infeq (val);
	if (op == Token::SUP_EQUAL) return this-> supeq (val);
	if (op == Token::NOT_EQUAL) return this-> neq (val);
	if (op == Token::DEQUAL) return this-> eq (val);
	return NULL;
    }
    
    const char * IFloatValue::getId () {
	return IFloatValue::id ();
    }

    std::string IFloatValue::toString () {
	if (this-> type == FloatConst::FLOAT)
	    return Ymir::OutBuffer (this-> value.d).str ();
	else
	    return Ymir::OutBuffer (this-> value.f).str ();
    }

    syntax::Expression IFloatValue::toYmir (Symbol sym) {
	auto ret = new (Z0)  IFloat (sym-> sym, this-> type);
	if (this-> type == FloatConst::FLOAT)
	    ret-> setValue (this-> value.f);
	else
	    ret-> setValue (this-> value.d);
	ret-> info = sym;
	return ret;
    }


    Value IFloatValue::add (Value other) {
	if (auto ot = other-> to<IFloatValue> ()) {
	    if (this-> type == FloatConst::DOUBLE)
		return new (Z0)  IFloatValue (this-> type, 0,
					      this-> value.d + ot-> value.d);
	    return new (Z0)  IFloatValue (this-> type,
					  this-> value.f + ot-> value.f,
					  0
	    );
	}
	return NULL;
    }
    
    Value IFloatValue::sub (Value other) {
	if (auto ot = other-> to<IFloatValue> ()) {
	    if (this-> type == FloatConst::DOUBLE)
		return new (Z0)  IFloatValue (this-> type, 0,
					      this-> value.d - ot-> value.d);
	    return new (Z0)  IFloatValue (this-> type,
					  this-> value.f - ot-> value.f,
					  0
	    );
	}
	return NULL;
    }        


    Value IFloatValue::mul (Value other) {
	if (auto ot = other-> to<IFloatValue> ()) {
	    if (this-> type == FloatConst::DOUBLE)
		return new (Z0)  IFloatValue (this-> type, 0,
					      this-> value.d * ot-> value.d);
	    return new (Z0)  IFloatValue (this-> type,
					  this-> value.f * ot-> value.f,
					  0
	    );
	}
	return NULL;
    }
    
    Value IFloatValue::div (Value other) {
	if (auto ot = other-> to<IFloatValue> ()) {
	    if (this-> type == FloatConst::DOUBLE)
		return new (Z0)  IFloatValue (this-> type, 0,
					      this-> value.d / ot-> value.d);
	    return new (Z0)  IFloatValue (this-> type,
					  this-> value.f / ot-> value.f,
					  0
	    );
	}
	return NULL;
    }        

    
    Value IFloatValue::inf (Value other) {
    	if (auto ot = other-> to<IFloatValue> ()) {
	    if (this-> type == FloatConst::DOUBLE)
		return new (Z0)  IBoolValue (this-> value.d < ot-> value.d);
	    return new (Z0)  IBoolValue (this-> value.f < ot-> value.f);
	}
	return NULL;
    }
    
    Value IFloatValue::sup (Value other) {
    	if (auto ot = other-> to<IFloatValue> ()) {
	    if (this-> type == FloatConst::DOUBLE)
		return new (Z0)  IBoolValue (this-> value.d > ot-> value.d);
	    return new (Z0)  IBoolValue (this-> value.f > ot-> value.f);
	}
	return NULL;
    }
    
    Value IFloatValue::infeq (Value other) {
    	if (auto ot = other-> to<IFloatValue> ()) {
	    if (this-> type == FloatConst::DOUBLE)
		return new (Z0)  IBoolValue (this-> value.d <= ot-> value.d);
	    return new (Z0)  IBoolValue (this-> value.f <= ot-> value.f);
	}
	return NULL;
    }
    
    Value IFloatValue::supeq (Value other) {
    	if (auto ot = other-> to<IFloatValue> ()) {
	    if (this-> type == FloatConst::DOUBLE)
		return new (Z0)  IBoolValue (this-> value.d >= ot-> value.d);
	    return new (Z0)  IBoolValue (this-> value.f >= ot-> value.f);
	}
	return NULL;
    }
    
    Value IFloatValue::neq (Value other) {
    	if (auto ot = other-> to<IFloatValue> ()) {
	    if (this-> type == FloatConst::DOUBLE)
		return new (Z0)  IBoolValue (this-> value.d != ot-> value.d);
	    return new (Z0)  IBoolValue (this-> value.f != ot-> value.f);
	}
	return NULL;
    }
    
    Value IFloatValue::eq (Value other) {
    	if (auto ot = other-> to<IFloatValue> ()) {
	    if (this-> type == FloatConst::DOUBLE)
		return new (Z0)  IBoolValue (this-> value.d == ot-> value.d);
	    return new (Z0)  IBoolValue (this-> value.f == ot-> value.f);
	}
	return NULL;
    }

    Value IFloatValue::clone () {
	return new (Z0)  IFloatValue (this-> type, this-> value.f, this-> value.d);
    }    

    bool IFloatValue::equals (Value other) {
	if (auto ot = other-> to <IFloatValue> ()) {
	    return this-> type == ot-> type &&
		this-> value.f == ot-> value.f &&
		this-> value.d == ot-> value.d;
	}
	return false;
    }

    
}
