#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/tree/Tree.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/utils/FixedUtils.hh>
#include <ymir/semantic/value/_.hh>
#include <ymir/syntax/Keys.hh>
#include <climits>

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
	    auto ret = new (Z0)  IFixedInfo (true, this-> type ());
	    ret-> unopFoo = FixedUtils::UnaryMinus;
	    if (this-> value ())
		ret-> value () = this-> value ()-> UnaryOp (op);
	    return ret;
	} else if (op == Token::AND && !this-> isConst ()) {
	    return toPtr (op);
	} else if (op == Token::DPLUS) {
	    auto ret = new (Z0)  IFixedInfo (true, this-> type ());
	    ret-> unopFoo = FixedUtils::InstPPlus;
	    if (this-> value ())
		ret-> value () = this-> value ()-> UnaryOp (op);
	    return ret;
	} else if (op == Token::DMINUS) {
	    auto ret = new (Z0)  IFixedInfo (true, this-> type ());
	    ret-> unopFoo = FixedUtils::InstSSub;
	    if (this-> value ())
		ret-> value () = this-> value ()-> UnaryOp (op);
	    return ret;
	}
	return NULL;
    }

    InfoType IFixedInfo::CastOp (InfoType other) {
	if (this-> isSame (other)) return this;
	if (other-> is<IBoolInfo> ()) {
	    auto aux = new (Z0)  IBoolInfo (this-> isConst ());
	    aux-> binopFoo = FixedUtils::InstCast;
	    return aux;
	} else if (other-> is<ICharInfo> ()) {
	    auto aux = new (Z0)  ICharInfo (this-> isConst ());
	    aux-> binopFoo = FixedUtils::InstCast;
	    return aux;
	} else if (auto ot = other-> to<IFloatInfo> ()) {
	    auto aux = new (Z0)  IFloatInfo (this-> isConst (), ot-> type ());
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
	} else if (auto ref = other-> to <IRefInfo> ()) {
	    if (!this-> isConst () && this-> isSame (ref-> content ())) {
		auto aux = new (Z0)  IRefInfo (this-> isConst (), this-> clone ());
		aux-> binopFoo = &FixedUtils::InstAddr;
		return aux;
	    }
	} else if (auto ot = other-> to <IFixedInfo> ()) {
	    if (this-> isSigned () && ot-> isSigned () && ot-> isSup (this)) {
		auto ret = ot-> clone ();
		ret-> binopFoo = FixedUtils::InstCast;
		return ret;
	    } else if (!this-> isSigned () && !ot-> isSigned () && ot-> isSup (this)) {
		auto ret = ot-> clone ();
		ret-> binopFoo = FixedUtils::InstCast;
		return ret;
	    }
	}
	return NULL;
    }

    InfoType IFixedInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	return NULL;
    }
    
    InfoType IFixedInfo::DColonOp (syntax::Var var) {
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

    InfoType IFixedInfo::onClone () {
	return new (Z0)  IFixedInfo (this-> isConst (), this-> _type);
    }

    const char* IFixedInfo::getId () {
	return IFixedInfo::id ();
    }
	
    InfoType IFixedInfo::toPtr (Word &) {	
	auto ret = new (Z0)  IPtrInfo (this-> isConst (), this-> clone ());
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
	    auto i = new (Z0)  IFixedInfo (false, this-> _type);
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
		auto ret = new (Z0)  IBoolInfo (true);
		ret-> binopFoo = &FixedUtils::InstTest;
		if (this-> value ())
		    ret-> value () = this-> value ()-> BinaryOp (op, right-> info-> type-> value ());
		return ret;	    
	    } else if (this-> isSup (ot)) {
		auto ret = new (Z0)  IBoolInfo (true);
		ret-> binopFoo = &FixedUtils::InstTest;
		if (this-> value ())
		    ret-> value () = this-> value ()-> BinaryOp (op, ot-> value ());
		return ret;
	    } else {
		auto ret = new (Z0)  IBoolInfo (true);
		ret-> binopFoo = &FixedUtils::InstTestRight;
		if (this-> value ())
		    ret-> value () = this-> value ()-> BinaryOp (op, ot-> value ());
		return ret;
	    }	    
	} else if (auto ot = right-> info-> type-> to<ICharInfo> ()) {
	    if (this-> _type == FixedConst::UBYTE) {
		auto ret = new (Z0)  IBoolInfo (true);
		ret-> binopFoo = &FixedUtils::InstTest;
		if (this-> value ())
		    ret-> value () = this-> value ()-> BinaryOp (op, ot-> value ());
		return ret;
	    }
	}
	return NULL;
    }

    InfoType IFixedInfo::opRange (Word, syntax::Expression right) {
	RangeInfo ret = NULL;
	if (this-> isSame (right-> info-> type)) {
	    ret = new (Z0)  IRangeInfo (true, this-> clone ());
	    ret-> binopFoo = &FixedUtils::InstRange;	    
	} else if (auto ot = right-> info-> type-> to <IFixedInfo> ()) {
	    if (this-> isSigned () && ot-> isSigned ()) {
		if (this-> isSup (ot)) {
		    ret = new (Z0)  IRangeInfo (true, this-> clone ());
		    ret-> binopFoo = &FixedUtils::InstRange;
		} else {
		    ret = new (Z0)  IRangeInfo (true, ot-> clone ());
		    ret-> binopFoo = &FixedUtils::InstRangeRight;
		}
	    } else if (!this-> isSigned () && !ot-> isSigned ()) {
		if (this-> isSup (ot)) {
		    ret = new (Z0)  IRangeInfo (true, this-> clone ());
		    ret-> binopFoo = &FixedUtils::InstRange;
		} else {
		    ret = new (Z0)  IRangeInfo (true, ot-> clone ());
		    ret-> binopFoo = &FixedUtils::InstRangeRight;
		}
	    }
	}
	
	if (ret) {
	    ret-> leftValue () = this-> value ();
	    ret-> rightValue () = right-> info-> value ();
	}
	return ret;
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
	auto ret = new (Z0) IFixedInfo (true, this-> _type);
	ret-> value () = new (Z0) IFixedValue (this-> _type, 0, 0);
	return ret;	
    }

    InfoType IFixedInfo::Max () {
	auto retI = new (Z0)  IFixedInfo (true, this-> _type);	
	IFixedValue* ret = new (Z0) IFixedValue (this-> _type, 0, 0);
	switch (this-> _type) {
	case FixedConst::BYTE : ret-> getValue () = SCHAR_MAX; break;
	case FixedConst::UBYTE : ret-> getValue () = UCHAR_MAX; break;
	case FixedConst::SHORT : ret-> getValue () = SHRT_MAX; break;
	case FixedConst::USHORT : ret-> getValue () = USHRT_MAX; break;
	case FixedConst::INT : ret-> getValue () = INT_MAX; break;
	case FixedConst::UINT : ret-> getValue () = UINT_MAX; break;
	case FixedConst::LONG : ret-> getValue () = LONG_MAX; break;
	case FixedConst::ULONG : ret-> getValue () = ULONG_MAX; break;	    
	}
	retI-> value () = ret;
	return retI;	
    }

    InfoType IFixedInfo::Min () {
	auto retI = new (Z0)  IFixedInfo (true, this-> _type);	
	IFixedValue* ret = new (Z0) IFixedValue (this-> _type, 0, 0);
	switch (this-> _type) {
	case FixedConst::BYTE : ret-> getValue () = SCHAR_MIN; break;
	case FixedConst::UBYTE : ret-> getValue () = 0; break;
	case FixedConst::SHORT : ret-> getValue () = SHRT_MIN; break;
	case FixedConst::USHORT : ret-> getValue () = 0; break;
	case FixedConst::INT : ret-> getValue () = INT_MIN; break;
	case FixedConst::UINT : ret-> getValue () = 0; break;
	case FixedConst::LONG : ret-> getValue () = LONG_MIN; break;
	case FixedConst::ULONG : ret-> getValue () = 0; break;	    
	}
	retI-> value () = ret;
	return retI;
    }

    InfoType IFixedInfo::SizeOf () {
	auto ret = new (Z0)  IFixedInfo (true, FixedConst::UBYTE);
	ret-> unopFoo = FixedUtils::InstSizeOf;
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
	    auto loc = locus.getLocus ();
	    auto ltree = left-> toGeneric ();
	    auto rtree = right-> toGeneric ();
	    rtree = convert (ltree.getType ().getTree (), rtree.getTree ());
	    
	    return Ymir::buildTree (
		MODIFY_EXPR, loc, ltree.getType (), ltree, rtree
	    );
	}

	Ymir::Tree InstReaff (Word locus, InfoType, Expression left, Expression right) {
	    auto ltree = left-> toGeneric ();
	    Ymir::Tree rtree = convert (ltree.getType ().getTree (), right-> toGeneric ().getTree ());
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
	    Ymir::Tree rtree = convert (ltree.getType ().getTree (), right-> toGeneric ().getTree ());
	    tree_code code = OperatorUtils::toGeneric (locus);
	    return Ymir::buildTree (
		code, locus.getLocus (), ltree.getType (), ltree, rtree
	    );
	}

	Ymir::Tree InstNormalRight (Word locus, InfoType, Expression left, Expression right) {
	    auto rtree = right-> toGeneric ();
	    Ymir::Tree ltree = convert (rtree.getType ().getTree (), left-> toGeneric ().getTree ());

	    tree_code code = OperatorUtils::toGeneric (locus);
	    return Ymir::buildTree (
		code, locus.getLocus (), ltree.getType (), ltree, rtree
	    );
	}

	Ymir::Tree InstTest (Word locus, InfoType, Expression left, Expression right) {
	    auto ltree = left-> toGeneric ();
	    Ymir::Tree rtree = convert (ltree.getType ().getTree (), right-> toGeneric ().getTree ());
	    tree_code code = OperatorUtils::toGeneric (locus);
	    return Ymir::buildTree (
		code, locus.getLocus (), boolean_type_node, ltree, rtree
	    );
	}

	Ymir::Tree InstNot (Word locus, InfoType, Expression left) {
	    auto ltree = left-> toGeneric ();
	    auto rtree = convert (ltree.getType ().getTree (), build_int_cst_type (boolean_type_node, true));
	    tree_code code = OperatorUtils::toGeneric ({locus.getLocus (), Token::XOR});
	    return Ymir::buildTree (
		code, locus.getLocus (), boolean_type_node, ltree, rtree
	    );
	}
	
	Ymir::Tree InstTestRight (Word locus, InfoType, Expression left, Expression right) {
	    auto rtree = right-> toGeneric ();	    
	    Ymir::Tree ltree = convert (rtree.getType ().getTree (), left-> toGeneric ().getTree ());

	    tree_code code = OperatorUtils::toGeneric (locus);
	    return Ymir::buildTree (
		code, locus.getLocus (), boolean_type_node, ltree, rtree
	    );
	}

	Ymir::Tree InstRange (Word locus, InfoType, Expression left, Expression right) {
	    location_t loc = locus.getLocus ();
	    TreeStmtList list;
	    auto ltree = left-> toGeneric ();
	    Ymir::Tree rtree = convert (ltree.getType ().getTree (), right-> toGeneric ().getTree ());
	    auto type = IRangeInfo::toGenericStatic (left-> info-> type-> simpleTypeString (), left-> info-> type-> toGeneric ());
	    	    
	    auto aux = makeAuxVar (loc, ISymbol::getLastTmp (), type);
	    auto fst = getField (loc, aux, "fst"), scd = getField (loc, aux, "scd");
	    list.append (buildTree (
		MODIFY_EXPR, loc, fst.getType (), fst, ltree
	    ));

	    list.append (buildTree (
		MODIFY_EXPR, loc, scd.getType (), scd, rtree
	    ));

	    return Ymir::compoundExpr (loc, list.getTree (), aux);
	}

	
	Ymir::Tree InstRangeRight (Word locus, InfoType, Expression left, Expression right) {
	    location_t loc = locus.getLocus ();
	    TreeStmtList list;
	    auto rtree = right-> toGeneric ();
	    Ymir::Tree ltree = convert (rtree.getType ().getTree (), left-> toGeneric ().getTree ());
	    	    
	    auto type = IRangeInfo::toGenericStatic (right-> info-> type-> simpleTypeString (), right-> info-> type-> toGeneric ());
	    	    
	    auto aux = makeAuxVar (loc, ISymbol::getLastTmp (), type);
	    auto fst = getField (loc, aux, "fst"), scd = getField (loc, aux, "scd");
	    
	    list.append (buildTree (
		MODIFY_EXPR, loc, fst.getType (), fst, ltree
	    ));

	    list.append (buildTree (
		MODIFY_EXPR, loc, scd.getType (), scd, rtree
	    ));

	    return Ymir::compoundExpr (loc, list.getTree (), aux);
	}

	
	Ymir::Tree UnaryMinus (Word locus, InfoType, Expression elem) {
	    auto lexp = elem-> toGeneric ();
	    return Ymir::buildTree (
		NEGATE_EXPR, locus.getLocus (), lexp.getType (), lexp
	    );
	}

	Ymir::Tree InstSSub (Word locus, InfoType, Expression elem) {
	    auto ltree = elem-> toGeneric ();
	    auto rtree = build_int_cst_type (ltree.getType ().getTree (), 1);
	    return Ymir::buildTree (
		MODIFY_EXPR, locus.getLocus (), ltree.getType (), ltree,
		Ymir::buildTree (
		    MINUS_EXPR, locus.getLocus (), ltree.getType (), ltree, rtree
		)
	    );		
	}
	
	Ymir::Tree InstPPlus (Word locus, InfoType, Expression elem) {
	    auto ltree = elem-> toGeneric ();
	    auto rtree = build_int_cst_type (ltree.getType ().getTree (), 1);
	    return Ymir::buildTree (
		MODIFY_EXPR, locus.getLocus (), ltree.getType (), ltree,
		Ymir::buildTree (
		    PLUS_EXPR, locus.getLocus (), ltree.getType (), ltree, rtree
		)
	    );		
	}
	
	Ymir::Tree InstCast (Word, InfoType, Expression elem, Expression typeExpr) {
	    auto type = typeExpr-> info-> type-> toGeneric ();
	    auto lexp = elem-> toGeneric ();
	    return convert (type.getTree (), lexp.getTree ());
	}
	
	Ymir::Tree InstAddr (Word locus, InfoType, Expression elem, Expression) {
	    return Ymir::getAddr (locus.getLocus (), elem-> toGeneric ());
	}

	Ymir::Tree InstSizeOf (Word, InfoType, Expression elem) {	    
	    return TYPE_SIZE_UNIT (elem-> info-> type-> toGeneric ().getTree ());
	}
	
    }


    IFixedValue::IFixedValue (FixedConst type, ulong ul, long l) {
	this-> type = type;
	if (!isSigned (this-> type)) {
	    this-> value.ul = ul;	    
	} else this-> value.l = l;
    }

    FixedConst IFixedValue::getType () {
	return this-> type;
    }
    
    long& IFixedValue::getValue () {
	return this-> value.l;
    }

    ulong& IFixedValue::getUValue () {
	return this-> value.ul;
    }

    Value IFixedValue::BinaryOp (Word op, Value val) {
	if (val == NULL) return NULL;
	if (op == Token::PLUS) return this-> add (op, val);
	if (op == Token::MINUS) return this-> sub (op, val);
	if (op == Token::DIV) return this-> div (op, val);
	if (op == Token::STAR) return this-> mul (op, val);
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
	auto ret = new (Z0)  IFixed (sym-> sym, this-> type, FixedMode::BUILTINS);
	if (isSigned (this-> type))
	    ret-> setValue (this-> value.l);
	else
	    ret-> setUValue (this-> value.ul);
	ret-> info = sym;
	return ret;
    }


    Value IFixedValue::add (Word op, Value other) {
	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type)) {
		auto ret = new (Z0)  IFixedValue (this-> type, 0,
						  this-> value.l + ot-> value.l);
		
		if ((this-> value.l < 0 && ot-> value.l < 0 && ret-> value.l >= 0)
		    ||
		    (this-> value.l >= 0 && ot-> value.l >= 0 && ret-> value.l < 0)
		    ) 
		    Ymir::Error::overflow (op, name (this-> type));
		check (op, ret-> value.l, this-> type);
		return ret;
	    } else {
		auto ret = new (Z0)  IFixedValue (this-> type,
						  this-> value.ul + ot-> value.ul,
						  0
						  );
		if (ret-> value.ul < this-> value.ul || ret-> value.ul < ot-> value.ul)
		    Ymir::Error::overflow (op, name (this-> type));
		check (op, ret-> value.ul, this-> type);
		return ret;
	    }
	}
	return NULL;
    }
    
    Value IFixedValue::sub (Word op, Value other) {
	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type)) {
		auto ret = new (Z0)  IFixedValue (this-> type, 0,
						  this-> value.l - ot-> value.l);
		if ((this-> value.l < 0 && ot-> value.l < 0 && ret-> value.l >= 0)
		    ||
		    (this-> value.l >= 0 && ot-> value.l < 0 && (ret-> value.l < 0 || ot-> value.l == LONG_MIN))
		    )
		    Ymir::Error::overflow (op, name (this-> type));
		check (op, ret-> value.l, this-> type);
		return ret;
	    }
	    if (this-> value.ul < ot-> value.ul)
		Ymir::Error::overflow (op, name (this-> type));
	    
	    auto ret = new (Z0)  IFixedValue (this-> type,
					  this-> value.ul - ot-> value.ul,
					  0
					  );
	    check (op, ret-> value.ul, this-> type);
	    return ret;
	}
	return NULL;
    }
    
    Value IFixedValue::div (Word, Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (Z0)  IFixedValue (this-> type, 0,
					     this-> value.l / ot-> value.l);
	    return new (Z0)  IFixedValue (this-> type,
					 this-> value.ul / ot-> value.ul,
					 0
	    );
	}
	return NULL;
    }
    Value IFixedValue::mul (Word op, Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type)) {
		auto ret = new (Z0)  IFixedValue (this-> type, 0,
						  this-> value.l * ot-> value.l);
		if ((this-> value.l & (long) (-2)) && ((ret-> value.l == ot-> value.l) ? ret-> value.l : (ret-> value.l / this-> value.l) != ot-> value.l))
		    Ymir::Error::overflow (op, name (this-> type));
		
		check (op, ret-> value.l, this-> type);
		return ret;
	    } 
	    
	    auto ret = new (Z0)  IFixedValue (this-> type,
					      this-> value.ul * ot-> value.ul,
					      0
					      );
	    if (ret-> value.ul >> 32 && ret-> value.ul / this-> value.ul != ot-> value.ul)
		Ymir::Error::overflow (op, name (this-> type));
	    check (op, ret-> value.ul, this-> type);
	    return ret;	    
	}
	return NULL;
    }

    void IFixedValue::check (Word op, long l, FixedConst cst) {
	bool overflow = false;
	switch (cst) {
	case FixedConst::BYTE : overflow = (l < SCHAR_MIN || l > SCHAR_MAX); break;
	case FixedConst::SHORT : overflow = (l < SHRT_MIN || l > SHRT_MAX); break;
	case FixedConst::INT : overflow = (l < INT_MIN || l > INT_MAX); break;
	default: break;
	}
	if (overflow)
	    Ymir::Error::overflow (op, name (cst));
    }
    
    void IFixedValue::check (Word op, ulong l, FixedConst cst) {
	bool overflow = false;
	switch (cst) {
	case FixedConst::UBYTE : overflow = (l > UCHAR_MAX); break;
	case FixedConst::USHORT : overflow = (l > USHRT_MAX); break;
	case FixedConst::UINT : overflow = (l > UINT_MAX); break;
	default: break;
	}
	if (overflow)
	    Ymir::Error::overflow (op, name (cst));
    }

    Value IFixedValue::lor (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (Z0)  IFixedValue (this-> type, 0,
					     this-> value.l | ot-> value.l);
	    return new (Z0)  IFixedValue (this-> type,
					 this-> value.ul | ot-> value.ul,
					 0
	    );
	}
	return NULL;
    }

    Value IFixedValue::land (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (Z0)  IFixedValue (this-> type, 0,
					     this-> value.l & ot-> value.l);
	    return new (Z0)  IFixedValue (this-> type,
					 this-> value.ul & ot-> value.ul,
					 0
	    );
	}
	return NULL;
    }
    
    Value IFixedValue::lshift (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (Z0)  IFixedValue (this-> type, 0,
					     this-> value.l << ot-> value.l);
	    return new (Z0)  IFixedValue (this-> type,
					 this-> value.ul << ot-> value.ul,
					 0
	    );
	}
	return NULL;
    }
    
    Value IFixedValue::rshift (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (Z0)  IFixedValue (this-> type, 0,
					     this-> value.l >> ot-> value.l);
	    return new (Z0)  IFixedValue (this-> type,
					 this-> value.ul >> ot-> value.ul,
					 0
	    );
	}
	return NULL;
    }
    
    Value IFixedValue::lxor (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (Z0)  IFixedValue (this-> type, 0,
					     this-> value.l ^ ot-> value.l);
	    return new (Z0)  IFixedValue (this-> type,
					 this-> value.ul ^ ot-> value.ul,
					 0
	    );
	}
	return NULL;
    }
    
    Value IFixedValue::mod (Value other) {
	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (Z0)  IFixedValue (this-> type, 0,
					     this-> value.l % ot-> value.l);
	    return new (Z0)  IFixedValue (this-> type,
					 this-> value.ul % ot-> value.ul,
					 0
	    );
	}
	return NULL;
    }
    
    Value IFixedValue::dand (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (Z0)  IFixedValue (this-> type, 0,
					     this-> value.l && ot-> value.l);
	    return new (Z0)  IFixedValue (this-> type,
					 this-> value.ul && ot-> value.ul,
					 0
	    );
	}
	return NULL;
    }
    
    Value IFixedValue::dor (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (Z0)  IFixedValue (this-> type, 0,
					     this-> value.l || ot-> value.l);
	    return new (Z0)  IFixedValue (this-> type,
					 this-> value.ul || ot-> value.ul,
					 0
	    );
	}
	return NULL;
    }
    
    Value IFixedValue::inf (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (Z0)  IBoolValue (this-> value.l < ot-> value.l);
	    return new (Z0)  IBoolValue (this-> value.ul < ot-> value.ul);
	}
	return NULL;
    }
    
    Value IFixedValue::sup (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (Z0)  IBoolValue (this-> value.l > ot-> value.l);
	    return new (Z0)  IBoolValue (this-> value.ul > ot-> value.ul);
	}
	return NULL;
    }
    
    Value IFixedValue::infeq (Value other) {
	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (Z0)  IBoolValue (this-> value.l <= ot-> value.l);
	    return new (Z0)  IBoolValue (this-> value.ul <= ot-> value.ul);
	}
	return NULL;
    }
    
    Value IFixedValue::supeq (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (Z0)  IBoolValue (this-> value.l >= ot-> value.l);
	    return new (Z0)  IBoolValue (this-> value.ul >= ot-> value.ul);
	}
	return NULL;
    }
    
    Value IFixedValue::neq (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (Z0)  IBoolValue (this-> value.l != ot-> value.l);
	    return new (Z0)  IBoolValue (this-> value.ul != ot-> value.ul);
	}
	return NULL;
    }
    
    Value IFixedValue::eq (Value other) {
    	if (auto ot = other-> to<IFixedValue> ()) {
	    if (isSigned (this-> type))
		return new (Z0)  IBoolValue (this-> value.l == ot-> value.l);
	    return new (Z0)  IBoolValue (this-> value.ul == ot-> value.ul);
	}
	return NULL;
    }

    Value IFixedValue::clone () {
	return new (Z0)  IFixedValue (this-> type, this-> value.ul, this-> value.l);
    }    

    Value IFixedValue::UnaryOp (Word op) {
	if (op == Token::MINUS) return new (Z0) IFixedValue (this-> type, -this-> value.ul, -this-> value.l);
	else if (op == Token::DPLUS) {
	    if (isSigned (this-> type)) this-> value.l++;
	    else this-> value.ul++;
	    return this;
	} else if (op == Token::DMINUS) {
	    if (isSigned (this-> type)) this-> value.l--;
	    else this-> value.ul--;
	    return this;
	}
	return NULL;
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
