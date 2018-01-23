#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/tree/Tree.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/utils/FixedUtils.hh>
#include <ymir/semantic/value/_.hh>

namespace semantic {

    using namespace syntax;
    
    IFixedInfo::IFixedInfo (bool isConst, FixedConst type) :
	IInfoType (isConst),
	_type (type)
    {}

    bool IFixedInfo::isSame (InfoType other) {
	auto ot	= other-> to<IFixedInfo> ();
	if (ot && ot-> type () == this-> type ()) {
	    return true;
	}
	return false;
    }
    
    std::string IFixedInfo::innerTypeString () {
	return syntax::name (this-> _type);
    }

    std::string IFixedInfo::innerSimpleTypeString () {
	return syntax::sname (this-> _type);
    }
	
    InfoType IFixedInfo::BinaryOp (Word op, syntax::Expression right) {
	if (op == Token::EQUAL) return Affect (right);
	if (op == Token::DIV_AFF) return opReaff (op, right);
	if (op == Token::AND_AFF) return opReaff (op, right);
	if (op == Token::PIPE_EQUAL) return opReaff (op, right);
	if (op == Token::MINUS_AFF) return opReaff (op, right);
	if (op == Token::PLUS_AFF) return opReaff (op, right);
	if (op == Token::LEFTD_AFF) return opReaff (op, right);
	if (op == Token::RIGHTD_AFF) return opReaff (op, right);
	if (op == Token::STAR_EQUAL) return opReaff (op, right);
	if (op == Token::PERCENT_EQUAL) return opReaff (op, right);
	if (op == Token::XOR_EQUAL) return opReaff (op, right);
	if (op == Token::DAND) return opNorm (op, right);
	if (op == Token::DPIPE) return opNorm (op, right);
	if (op == Token::INF) return opTest (op, right);
	if (op == Token::SUP) return opTest (op, right);
	if (op == Token::INF_EQUAL) return opTest (op, right);
	if (op == Token::SUP_EQUAL) return opTest (op, right);
	if (op == Token::NOT_EQUAL) return opTest(op, right);
	if (op == Token::DEQUAL) return opTest(op, right);
	if (op == Token::PLUS) return opNorm (op, right);
	if (op == Token::MINUS) return opNorm (op, right);
	if (op == Token::DIV) return opNorm (op, right);
	if (op == Token::STAR) return opNorm (op, right);
	if (op == Token::PIPE) return opNorm(op, right);
	if (op == Token::AND) return opNorm(op, right);
	if (op == Token::LEFTD) return opNorm(op, right);
	if (op == Token::XOR) return opNorm(op, right);
	if (op == Token::RIGHTD) return opNorm(op, right);
	if (op == Token::PERCENT) return opNorm(op, right);
	if (op == Token::DDOT) return opRange (op, right);
	return NULL;
    }

    InfoType IFixedInfo::BinaryOpRight (Word op, syntax::Expression left) {
	if (op == Token::EQUAL) return AffectRight (left);
	return NULL;
    }
	
    InfoType IFixedInfo::UnaryOp (Word op) {
	if (op == Token::MINUS) {
	    auto ret = new (GC) IFixedInfo (true, this-> type ());
	    ret-> unopFoo = FixedUtils::UnaryMinus;
	    if (this-> value ())
		ret-> value () = this-> value ()-> UnaryOp (op);
	    return ret;
	} else if (op == Token::AND && !this-> isConst ())
	    return toPtr ();
	return NULL;
    }

    InfoType IFixedInfo::CastOp (InfoType other) {
	if (this-> isSame (other)) return this;
	if (other-> is<IBoolInfo> ()) {
	    auto aux = new (GC) IBoolInfo (this-> isConst ());
	    aux-> binopFoo = FixedUtils::InstCast;
	    return aux;
	} else if (other-> is<ICharInfo> ()) {
	    auto aux = new (GC) ICharInfo (this-> isConst ());
	    aux-> binopFoo = FixedUtils::InstCast;
	    return aux;
	} else if (auto ot = other-> to<IFloatInfo> ()) {
	    auto aux = new (GC) IFloatInfo (this-> isConst (), ot-> type ());
	    aux-> binopFoo = FixedUtils::InstCast;
	    return aux;
	} else if (auto ot = other-> to<IFixedInfo> ()) {
	    auto ret = ot-> clone ();
	    ret-> isConst (this-> isConst ());
	    ret-> binopFoo = FixedUtils::InstCast;
	    return ret;
	}
	return NULL;
    }
	
    InfoType IFixedInfo::CompOp (InfoType other) {
	if (other-> is<IUndefInfo> () || this-> isSame (other)) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &FixedUtils::InstCast;
	    return ret;
	} else if (auto ref = other-> to<IRefInfo> ()) {
	    if (!this-> isConst () && this-> isSame (ref-> content ())) {
		auto aux = new (GC) IRefInfo (this-> isConst (), this-> clone ());
		aux-> binopFoo = &FixedUtils::InstAddr;
		return aux;
	    }
	} else if (auto ot = other-> to<IFixedInfo> ()) {
	    if (this-> isSigned () && ot-> isSigned () && ot-> isSup (this)) {
		auto ret = ot-> clone ();
		ret-> binopFoo = FixedUtils::InstCast;
		return ret;
	    } else if (!this-> isSigned () && !ot-> isSigned () && ot-> isSup (this)) {
		auto ret = ot-> clone ();
		ret-> binopFoo = FixedUtils::InstCast;
		return ret;
	    }
	} else if (auto ot = other-> to<IEnumInfo> ()) {
	    return this-> CompOp (ot-> content ());
	}
	return NULL;
    }

    InfoType IFixedInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "init") return Init ();
	if (var-> token == "max") return Max ();
	if (var-> token == "min") return Min ();
	if (var-> token == "sizeof") return SizeOf ();
	if (var-> token == "typeid") return StringOf ();
	return NULL;
    }

    FixedConst	IFixedInfo::type () {
	return this-> _type;
    }

    InfoType IFixedInfo::clone () {
	auto ret = new (GC) IFixedInfo (this-> isConst (), this-> _type);
	if (this-> value ())
	    ret-> value () = this-> value ()-> clone ();
	return ret;
    }

    const char* IFixedInfo::getId () {
	return IFixedInfo::id ();
    }
	
    InfoType IFixedInfo::toPtr () {
	auto ret = new (GC) IPtrInfo (this-> isConst (), this-> clone ());
	ret-> binopFoo = &FixedUtils::InstAddr;
	return ret;
    }

    InfoType IFixedInfo::pplus () {
	auto ret = this-> clone ();
	ret-> unopFoo = &FixedUtils::InstPPlus;
	return ret;
    }

    InfoType IFixedInfo::ssub () {
	auto ret = this-> clone ();
	ret-> unopFoo = &FixedUtils::InstSSub;
	return ret;
    }

    InfoType IFixedInfo::Affect (syntax::Expression right) {
	if (auto ot = right-> info-> type-> to<IFixedInfo> ()) {
	    if (this-> isSigned () && ot-> isSigned () && this-> isSup (ot)) {
		auto ret = this-> clone ();
		ret-> binopFoo = &FixedUtils::InstAffect;
		return ret;
	    } else if (!this-> isSigned () && !ot-> isSigned () && this-> isSup (ot)) {
		auto ret = this-> clone ();
		ret-> binopFoo = &FixedUtils::InstAffect;
		return ret;
	    } else if (this-> _type == ot-> type ()) {
		auto ret = this-> clone ();
		ret-> binopFoo = &FixedUtils::InstAffect;
		return ret;
	    }
	}
	return NULL;
    }

    InfoType IFixedInfo::AffectRight (syntax::Expression left) {
	if (left-> info-> type-> is<IUndefInfo> ()) {
	    auto i = new (GC) IFixedInfo (false, this-> _type);
	    i-> binopFoo = &FixedUtils::InstAffect;
	    return i;
	}
	return NULL;
    }

    InfoType IFixedInfo::opAff (Word, syntax::Expression right) {
	if (auto ot = right-> info-> type-> to<IFixedInfo> ()) {
	    if (this-> isSigned () && ot-> isSigned () && this-> isSup (ot)) {
		auto ret = this-> clone ();
		ret-> binopFoo = &FixedUtils::InstAffect;
		return ret;
	    } else if (!this-> isSigned () && !ot-> isSigned () && this-> isSup (ot)) {
		auto ret = this-> clone ();
		ret-> binopFoo = &FixedUtils::InstAffect;
		return ret;
	    } else if (this-> _type == ot-> type ()) {
		auto ret = this-> clone ();
		ret-> binopFoo = &FixedUtils::InstAffect;
		return ret;
	    }
	}
	return NULL;
    }

    InfoType IFixedInfo::opReaff (Word, syntax::Expression right) {
	if (auto ot = right-> info-> type-> to<IFixedInfo> ()) {
	    if (this-> isSigned () && ot-> isSigned () && this-> isSup (ot)) {
		auto ret = this-> clone ();
		ret-> binopFoo = &FixedUtils::InstReaff;
		return ret;
	    } else if (!this-> isSigned () && !ot-> isSigned () && this-> isSup (ot)) {
		auto ret = this-> clone ();
		ret-> binopFoo = &FixedUtils::InstReaff;
		return ret;
	    } else if (this-> _type == ot-> type ()) {
		auto ret = this-> clone ();
		ret-> binopFoo = &FixedUtils::InstReaff;
		return ret;
	    }
	}
	return NULL;
    }
    
    InfoType IFixedInfo::opTest (Word op, syntax::Expression right) {
	if (auto ot = right-> info-> type-> to<IFixedInfo> ()) {
	    if (this-> _type == ot-> type ()) {
		auto ret = new (GC) IBoolInfo (true);
		ret-> binopFoo = &FixedUtils::InstTest;
		if (this-> value ())
		    ret-> value () = this-> value ()-> BinaryOp (op, right-> info-> type-> value ());
		return ret;	    
	    } else if (this-> isSup (ot)) {
		auto ret = new (GC) IBoolInfo (true);
		ret-> binopFoo = &FixedUtils::InstTest;
		if (this-> value ())
		    ret-> value () = this-> value ()-> BinaryOp (op, ot-> value ());
		return ret;
	    } else {
		auto ret = new (GC) IBoolInfo (true);
		ret-> binopFoo = &FixedUtils::InstTestRight;
		if (this-> value ())
		    ret-> value () = this-> value ()-> BinaryOp (op, ot-> value ());
		return ret;
	    }	    
	} else if (auto ot = right-> info-> type-> to<ICharInfo> ()) {
	    if (this-> _type == FixedConst::UBYTE) {
		auto ret = new (GC) IBoolInfo (true);
		ret-> binopFoo = &FixedUtils::InstTest;
		if (this-> value ())
		    ret-> value () = this-> value ()-> BinaryOp (op, ot-> value ());
		return ret;
	    }
	}
	return NULL;
    }

    InfoType IFixedInfo::opRange (Word, syntax::Expression right) {
	if (this-> isSame (right-> info-> type)) {
	    auto ret = new (GC) IRangeInfo (true, this-> clone ());
	    ret-> binopFoo = &FixedUtils::InstRange;
	    return ret;
	} else if (auto ot = right-> info-> type-> to <IFixedInfo> ()) {
	    if (this-> isSigned () && ot-> isSigned ()) {
		if (this-> isSup (ot)) {
		    auto ret = new (GC) IRangeInfo (true, this-> clone ());
		    ret-> binopFoo = &FixedUtils::InstRange;
		    return ret;
		} else {
		    auto ret = new (GC) IRangeInfo (true, ot-> clone ());
		    ret-> binopFoo = &FixedUtils::InstRangeRight;
		    return ret;
		}
	    } else if (!this-> isSigned () && !ot-> isSigned ()) {
		if (this-> isSup (ot)) {
		    auto ret = new (GC) IRangeInfo (true, this-> clone ());
		    ret-> binopFoo = &FixedUtils::InstRange;
		    return ret;
		} else {
		    auto ret = new (GC) IRangeInfo (true, ot-> clone ());
		    ret-> binopFoo = &FixedUtils::InstRangeRight;
		    return ret;
		}
	    }
	}
	return NULL;
    }
    
    InfoType IFixedInfo::opNorm (Word op, syntax::Expression right) {
	if (this-> isSame (right-> info-> type)) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &FixedUtils::InstNormal;
	    if (this-> value ())
		ret-> value () = this-> value ()-> BinaryOp (op, right-> info-> type-> value ());
	    return ret;
	} else if (auto ot = right-> info-> type-> to<IFixedInfo> ()) {
	    if (this-> isSigned () && ot-> isSigned ()) {
		if (this-> isSup (ot)) {
		    auto ret = this-> clone ();
		    ret-> binopFoo = &FixedUtils::InstNormal;
		    if (this-> value ())
			ret-> value () = this-> value ()-> BinaryOp (op, ot-> value ());
		    return ret;
		} else {
		    auto ret = ot-> clone ();
		    ret-> binopFoo = &FixedUtils::InstNormalRight;
		    if (this-> value ())
			ret-> value () = this-> value ()-> BinaryOp (op, ot-> value ());
		    return ret;
		}		    
	    } else if (!this-> isSigned () && !ot-> isSigned ()) {
		if (this-> isSup (ot)) {
		    auto ret = this-> clone ();
		    if (this-> value ())
			ret-> value () = this-> value ()-> BinaryOp (op, ot-> value ());
		    ret-> binopFoo = &FixedUtils::InstNormal;
		    return ret;
		} else {
		    auto ret = ot-> clone ();
		    ret-> binopFoo = &FixedUtils::InstNormalRight;
		    if (this-> value ())
			ret-> value () = this-> value ()-> BinaryOp (op, ot-> value ());
		    return ret;
		}
	    }
	}
	return NULL;
    }

    InfoType IFixedInfo::Init () {
	auto ret = new (GC) IFixedInfo (true, this-> _type);
	return ret;	
    }

    InfoType IFixedInfo::Max () {
	auto ret = new (GC) IFixedInfo (true, this-> _type);
	//TODO
	return ret;	
    }

    InfoType IFixedInfo::Min () {
	auto ret = new (GC) IFixedInfo (true, this-> _type);
	//TODO
	return ret;	
    }

    InfoType IFixedInfo::SizeOf () {
	auto ret = new (GC) IFixedInfo (true, FixedConst::UBYTE);
	//TODO
	return ret;	
    }

    bool IFixedInfo::isSigned () {
	return syntax::isSigned (this-> _type);
    }
    
    bool IFixedInfo::isSup (FixedInfo fx) {
	return this-> _type > fx-> _type;
    }

    Ymir::Tree IFixedInfo::toGeneric () {
	switch (this-> _type) {
	case FixedConst::BYTE : return signed_char_type_node;
	case FixedConst::UBYTE : return unsigned_char_type_node;
	case FixedConst::SHORT : return short_integer_type_node;
	case FixedConst::USHORT : return short_unsigned_type_node;
	case FixedConst::INT : return integer_type_node;
	case FixedConst::UINT : return unsigned_type_node;
	case FixedConst::LONG : return long_integer_type_node;
	case FixedConst::ULONG : return long_unsigned_type_node ;
	}
	return integer_type_node;
    }
    
    namespace FixedUtils {
	using namespace syntax;
	using namespace Ymir;
	
	Ymir::Tree InstAffect (Word locus, InfoType, Expression left, Expression right) {
	    auto ltree = left-> toGeneric ();
	    Ymir::Tree rtree = fold_convert_loc (locus.getLocus (), ltree.getType ().getTree (), right-> toGeneric ().getTree ());
	    
	    return Ymir::buildTree (
		MODIFY_EXPR, locus.getLocus (), void_type_node, ltree, rtree
	    );
	}

	Ymir::Tree InstReaff (Word locus, InfoType, Expression left, Expression right) {
	    auto ltree = left-> toGeneric ();
	    Ymir::Tree rtree = fold_convert_loc (locus.getLocus (), ltree.getType ().getTree (), right-> toGeneric ().getTree ());
	    tree_code code = OperatorUtils::toGeneric (locus);
	    return Ymir::buildTree (
		MODIFY_EXPR, locus.getLocus (), ltree.getType (), ltree,
		Ymir::buildTree (
		    code, locus.getLocus (), ltree.getType (), ltree, rtree
		)
	    );		
	}
	
	Ymir::Tree InstNormal (Word locus, InfoType, Expression left, Expression right) {
	    auto ltree = left-> toGeneric ();
	    Ymir::Tree rtree = fold_convert_loc (locus.getLocus (), ltree.getType ().getTree (), right-> toGeneric ().getTree ());
	    tree_code code = OperatorUtils::toGeneric (locus);
	    return Ymir::buildTree (
		code, locus.getLocus (), ltree.getType (), ltree, rtree
	    );
	}

	Ymir::Tree InstNormalRight (Word locus, InfoType, Expression left, Expression right) {
	    auto rtree = right-> toGeneric ();
	    Ymir::Tree ltree = fold_convert_loc (locus.getLocus (), rtree.getType ().getTree (), left-> toGeneric ().getTree ());

	    tree_code code = OperatorUtils::toGeneric (locus);
	    return Ymir::buildTree (
		code, locus.getLocus (), ltree.getType (), ltree, rtree
	    );
	}

	Ymir::Tree InstTest (Word locus, InfoType, Expression left, Expression right) {
	    auto ltree = left-> toGeneric ();
	    Ymir::Tree rtree = fold_convert_loc (locus.getLocus (), ltree.getType ().getTree (), right-> toGeneric ().getTree ());
	    tree_code code = OperatorUtils::toGeneric (locus);
	    return Ymir::buildTree (
		code, locus.getLocus (), boolean_type_node, ltree, rtree
	    );
	}

	Ymir::Tree InstTestRight (Word locus, InfoType, Expression left, Expression right) {
	    auto rtree = right-> toGeneric ();
	    Ymir::Tree ltree = fold_convert_loc (locus.getLocus (), rtree.getType ().getTree (), left-> toGeneric ().getTree ());

	    tree_code code = OperatorUtils::toGeneric (locus);
	    return Ymir::buildTree (
		code, locus.getLocus (), boolean_type_node, ltree, rtree
	    );
	}

	Ymir::Tree InstRange (Word locus, InfoType, Expression left, Expression right) {
	    location_t loc = locus.getLocus ();
	    TreeStmtList list;
	    auto ltree = left-> toGeneric ();
	    Ymir::Tree rtree = fold_convert_loc (locus.getLocus (), ltree.getType ().getTree (), right-> toGeneric ().getTree ());
	    auto type = IRangeInfo::toGenericStatic (left-> info-> type-> simpleTypeString (), left-> info-> type-> toGeneric ());
	    	    
	    auto aux = makeAuxVar (loc, ISymbol::getLastTmp (), type);
	    auto fst = getField (loc, aux, "fst"), scd = getField (loc, aux, "scd");
	    list.append (buildTree (
		MODIFY_EXPR, loc, void_type_node, fst, ltree
	    ));

	    list.append (buildTree (
		MODIFY_EXPR, loc, void_type_node, scd, rtree
	    ));

	    getStackStmtList ().back ().append (list.getTree ());
	    return aux;
	}

	
	Ymir::Tree InstRangeRight (Word locus, InfoType, Expression left, Expression right) {
	    location_t loc = locus.getLocus ();
	    TreeStmtList list;
	    auto rtree = right-> toGeneric ();
	    Ymir::Tree ltree = fold_convert_loc (locus.getLocus (), rtree.getType ().getTree (), left-> toGeneric ().getTree ());
	    	    
	    auto type = IRangeInfo::toGenericStatic (right-> info-> type-> simpleTypeString (), right-> info-> type-> toGeneric ());
	    	    
	    auto aux = makeAuxVar (loc, ISymbol::getLastTmp (), type);
	    auto fst = getField (loc, aux, "fst"), scd = getField (loc, aux, "scd");
	    
	    list.append (buildTree (
		MODIFY_EXPR, loc, void_type_node, fst, ltree
	    ));

	    list.append (buildTree (
		MODIFY_EXPR, loc, void_type_node, scd, rtree
	    ));

	    getStackStmtList ().back ().append (list.getTree ());
	    return aux;
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
	
	Ymir::Tree InstCast (Word locus, InfoType, Expression elem, Expression typeExpr) {
	    auto type = typeExpr-> info-> type-> toGeneric ();
	    auto lexp = elem-> toGeneric ();
	    return fold_convert_loc (locus.getLocus (), type.getTree (), lexp.getTree ());
	}
	
	Ymir::Tree InstAddr (Word locus, InfoType, Expression elem, Expression) {
	    return Ymir::getAddr (locus.getLocus (), elem-> toGeneric ());
	}
	
    }



    IFixedValue::IFixedValue (FixedConst type, ulong ul, long l) {
	this-> type = type;
	if (!isSigned (this-> type)) {
	    this-> value.ul = ul;	    
	} else this-> value.l = l;
    }
    
    long IFixedValue::getValue () {
	return this-> value.l;
    }

    ulong IFixedValue::getUValue () {
	return this-> value.ul;
    }

    Value IFixedValue::BinaryOp (Word op, Value val) {
	if (op == Token::PLUS) return this-> add (val);
	if (op == Token::MINUS) return this-> sub (val);
	if (op == Token::DIV) return this-> div (val);
	if (op == Token::STAR) return this-> mul (val);
	if (op == Token::PIPE) return this-> lor (val);
	if (op == Token::AND) return this-> land (val);
	if (op == Token::LEFTD) return this-> lshift (val);
	if (op == Token::XOR) return this-> lxor (val);
	if (op == Token::RIGHTD) return this-> rshift (val);
	if (op == Token::PERCENT) return this-> mod (val);
	if (op == Token::DAND) return  this-> dand (val);
	if (op == Token::DPIPE) return this-> dor (val);
	if (op == Token::INF) return this-> inf (val);
	if (op == Token::SUP) return this-> sup (val);
	if (op == Token::INF_EQUAL) return this-> infeq (val);
	if (op == Token::SUP_EQUAL) return this-> supeq (val);
	if (op == Token::NOT_EQUAL) return this-> neq (val);
	if (op == Token::DEQUAL) return this-> eq (val);
	return NULL;
    }
    
    const char * IFixedValue::getId () {
	return IFixedValue::id ();
    }

    std::string IFixedValue::toString () {
	if (isSigned (this-> type))
	    return Ymir::OutBuffer (this-> value.l).str ();
	else
	    return Ymir::OutBuffer (this-> value.ul).str ();
    }

    syntax::Expression IFixedValue::toYmir (Symbol sym) {
	auto ret = new (GC) IFixed (sym-> sym, this-> type);
	if (isSigned (this-> type))
	    ret-> setValue (this-> value.l);
	else
	    ret-> setUValue (this-> value.ul);
	ret-> info = sym;
	return ret;
    }


    Value IFixedValue::add (Value other) {
	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (GC) IFixedValue (this-> type, 0,
					     this-> value.l + ot-> value.l);
	    return new (GC) IFixedValue (this-> type,
					 this-> value.ul + ot-> value.ul,
					 0
	    );
	}
	return NULL;
    }
    
    Value IFixedValue::sub (Value other) {
	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (GC) IFixedValue (this-> type, 0,
					     this-> value.l - ot-> value.l);
	    return new (GC) IFixedValue (this-> type,
					 this-> value.ul - ot-> value.ul,
					 0
	    );
	}
	return NULL;
    }
    Value IFixedValue::div (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (GC) IFixedValue (this-> type, 0,
					     this-> value.l / ot-> value.l);
	    return new (GC) IFixedValue (this-> type,
					 this-> value.ul / ot-> value.ul,
					 0
	    );
	}
	return NULL;
    }
    Value IFixedValue::mul (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (GC) IFixedValue (this-> type, 0,
					     this-> value.l * ot-> value.l);
	    return new (GC) IFixedValue (this-> type,
					 this-> value.ul * ot-> value.ul,
					 0
	    );
	}
	return NULL;
    }
    Value IFixedValue::lor (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (GC) IFixedValue (this-> type, 0,
					     this-> value.l | ot-> value.l);
	    return new (GC) IFixedValue (this-> type,
					 this-> value.ul | ot-> value.ul,
					 0
	    );
	}
	return NULL;
    }

    Value IFixedValue::land (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (GC) IFixedValue (this-> type, 0,
					     this-> value.l & ot-> value.l);
	    return new (GC) IFixedValue (this-> type,
					 this-> value.ul & ot-> value.ul,
					 0
	    );
	}
	return NULL;
    }
    
    Value IFixedValue::lshift (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (GC) IFixedValue (this-> type, 0,
					     this-> value.l << ot-> value.l);
	    return new (GC) IFixedValue (this-> type,
					 this-> value.ul << ot-> value.ul,
					 0
	    );
	}
	return NULL;
    }
    
    Value IFixedValue::rshift (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (GC) IFixedValue (this-> type, 0,
					     this-> value.l >> ot-> value.l);
	    return new (GC) IFixedValue (this-> type,
					 this-> value.ul >> ot-> value.ul,
					 0
	    );
	}
	return NULL;
    }
    
    Value IFixedValue::lxor (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (GC) IFixedValue (this-> type, 0,
					     this-> value.l ^ ot-> value.l);
	    return new (GC) IFixedValue (this-> type,
					 this-> value.ul ^ ot-> value.ul,
					 0
	    );
	}
	return NULL;
    }
    
    Value IFixedValue::mod (Value other) {
	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (GC) IFixedValue (this-> type, 0,
					     this-> value.l % ot-> value.l);
	    return new (GC) IFixedValue (this-> type,
					 this-> value.ul % ot-> value.ul,
					 0
	    );
	}
	return NULL;
    }
    
    Value IFixedValue::dand (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (GC) IFixedValue (this-> type, 0,
					     this-> value.l && ot-> value.l);
	    return new (GC) IFixedValue (this-> type,
					 this-> value.ul && ot-> value.ul,
					 0
	    );
	}
	return NULL;
    }
    
    Value IFixedValue::dor (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (GC) IFixedValue (this-> type, 0,
					     this-> value.l || ot-> value.l);
	    return new (GC) IFixedValue (this-> type,
					 this-> value.ul || ot-> value.ul,
					 0
	    );
	}
	return NULL;
    }
    
    Value IFixedValue::inf (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (GC) IBoolValue (this-> value.l < ot-> value.l);
	    return new (GC) IBoolValue (this-> value.ul < ot-> value.ul);
	}
	return NULL;
    }
    
    Value IFixedValue::sup (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (GC) IBoolValue (this-> value.l > ot-> value.l);
	    return new (GC) IBoolValue (this-> value.ul > ot-> value.ul);
	}
	return NULL;
    }
    
    Value IFixedValue::infeq (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (GC) IBoolValue (this-> value.l <= ot-> value.l);
	    return new (GC) IBoolValue (this-> value.ul <= ot-> value.ul);
	}
	return NULL;
    }
    
    Value IFixedValue::supeq (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (GC) IBoolValue (this-> value.l >= ot-> value.l);
	    return new (GC) IBoolValue (this-> value.ul >= ot-> value.ul);
	}
	return NULL;
    }
    
    Value IFixedValue::neq (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (GC) IBoolValue (this-> value.l != ot-> value.l);
	    return new (GC) IBoolValue (this-> value.ul != ot-> value.ul);
	}
	return NULL;
    }
    
    Value IFixedValue::eq (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (GC) IBoolValue (this-> value.l == ot-> value.l);
	    return new (GC) IBoolValue (this-> value.ul == ot-> value.ul);
	}
	return NULL;
    }

    Value IFixedValue::clone () {
	return new (GC) IFixedValue (this-> type, this-> value.ul, this-> value.l);
    }    

    bool IFixedValue::equals (Value other) {
	if (auto ot = other-> to <IFixedValue> ()) {
	    return this-> type == ot-> type &&
		this-> value.ul == ot-> value.ul &&
		this-> value.l == ot-> value.l;
	}
	return false;
    }
    
}
