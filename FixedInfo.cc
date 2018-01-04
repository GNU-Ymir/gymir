#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/pack/InternalFunction.hh>
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

    std::string IFixedInfo::simpleTypeString () {
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
	    auto ret = new IFixedInfo (true, this-> type ());
	    ret-> unopFoo = FixedUtils::UnaryMinus;
	    return ret;
	} else if (op == Token::AND && !this-> isConst ())
	    return toPtr ();
	return NULL;
    }

    InfoType IFixedInfo::CastOp (InfoType other) {
	if (this-> isSame (other)) return this;
	if (other-> is<IBoolInfo> ()) {
	    auto aux = new IBoolInfo (this-> isConst ());
	    aux-> binopFoo = FixedUtils::InstCast;
	    return aux;
	} else if (other-> is<ICharInfo> ()) {
	    auto aux = new ICharInfo (this-> isConst ());
	    aux-> binopFoo = FixedUtils::InstCast;
	    return aux;
	} else if (auto ot = other-> to<IFloatInfo> ()) {
	    auto aux = new IFloatInfo (this-> isConst (), ot-> type ());
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
		auto ret = this-> clone ();
		ret-> binopFoo = FixedUtils::InstCast;
		return ret;
	    } else if (!this-> isSigned () && !ot-> isSigned () && ot-> isSup (this)) {
		auto ret = this-> clone ();
		ret-> binopFoo = FixedUtils::InstCast;
		return ret;
	    }
	} else if (auto ot = other-> to<IEnumInfo> ()) {
	    return this->CompOp (ot-> content ());
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
	auto ret = new IFixedInfo (this-> isConst (), this-> _type);	
	return ret;
    }

    const char* IFixedInfo::getId () {
	return IFixedInfo::id ();
    }
	
    InfoType IFixedInfo::toPtr () {
	auto ret = new IPtrInfo (this-> isConst (), this-> clone ());
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
	    auto i = new IFixedInfo (false, this-> _type);
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
    
    InfoType IFixedInfo::opTest (Word, syntax::Expression right) {
	if (auto ot = right-> info-> type-> to<IFixedInfo> ()) {
	    if (this-> _type == ot-> type ()) {
		auto ret = new IBoolInfo (true);
		ret-> binopFoo = &FixedUtils::InstTest;
		return ret;	    
	    } else if (this-> isSup (ot)) {
		auto ret = new IBoolInfo (true);
		ret-> binopFoo = &FixedUtils::InstTest;
		return ret;
	    } else {
		auto ret = new IBoolInfo (true);
		ret-> binopFoo = &FixedUtils::InstTestRight;
		return ret;
	    }	    
	} else if (right-> info-> type-> is<ICharInfo> ()) {
	    if (this-> _type == FixedConst::UBYTE) {
		auto ret = new IBoolInfo (true);
		ret-> binopFoo = &FixedUtils::InstTest;
		return ret;
	    }
	}
	return NULL;
    }

    InfoType IFixedInfo::opRange (Word, syntax::Expression right) {
	if (this-> isSame (right-> info-> type)) {
	    auto ret = new IRangeInfo (true, this-> clone ());
	    ret-> binopFoo = &FixedUtils::InstRange;
	    return ret;
	} else if (auto ot = right-> info-> type-> to <IFixedInfo> ()) {
	    if (this-> isSigned () && ot-> isSigned ()) {
		if (this-> isSup (ot)) {
		    auto ret = new IRangeInfo (true, this-> clone ());
		    ret-> binopFoo = &FixedUtils::InstRange;
		    return ret;
		} else {
		    auto ret = new IRangeInfo (true, ot-> clone ());
		    ret-> binopFoo = &FixedUtils::InstRangeRight;
		    return ret;
		}
	    } else if (!this-> isSigned () && !ot-> isSigned ()) {
		if (this-> isSup (ot)) {
		    auto ret = new IRangeInfo (true, this-> clone ());
		    ret-> binopFoo = &FixedUtils::InstRange;
		    return ret;
		} else {
		    auto ret = new IRangeInfo (true, ot-> clone ());
		    ret-> binopFoo = &FixedUtils::InstRangeRight;
		    return ret;
		}
	    }
	}
	return NULL;
    }
    
    InfoType IFixedInfo::opNorm (Word, syntax::Expression right) {
	if (this-> isSame (right-> info-> type)) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &FixedUtils::InstNormal;
	    return ret;
	} else if (auto ot = right-> info-> type-> to<IFixedInfo> ()) {
	    if (this-> isSigned () && ot-> isSigned ()) {
		if (this-> isSup (ot)) {
		    auto ret = this-> clone ();
		    ret-> binopFoo = &FixedUtils::InstNormal;
		    return ret;
		} else {
		    auto ret = ot-> clone ();
		    ret-> binopFoo = &FixedUtils::InstNormalRight;
		    return ret;
		}		    
	    } else if (!this-> isSigned () && !ot-> isSigned ()) {
		if (this-> isSup (ot)) {
		    auto ret = this-> clone ();
		    ret-> binopFoo = &FixedUtils::InstNormal;
		    return ret;
		} else {
		    auto ret = ot-> clone ();
		    ret-> binopFoo = &FixedUtils::InstNormalRight;
		    return ret;
		}
	    }
	}
	return NULL;
    }

    InfoType IFixedInfo::Init () {
	auto ret = new IFixedInfo (true, this-> _type);
	//TODO
	return ret;	
    }

    InfoType IFixedInfo::Max () {
	auto ret = new IFixedInfo (true, this-> _type);
	//TODO
	return ret;	
    }

    InfoType IFixedInfo::Min () {
	auto ret = new IFixedInfo (true, this-> _type);
	//TODO
	return ret;	
    }

    InfoType IFixedInfo::SizeOf () {
	auto ret = new IFixedInfo (true, FixedConst::UBYTE);
	//TODO
	return ret;	
    }

    InfoType IFixedInfo::StringOf () {
	auto str = new IStringInfo (true);
	return str;
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

    Ymir::Tree IFixedInfo::getInitFnPtr () {
	if (this-> _type == FixedConst::INT) return InternalFunction::getYInitInt ();
	else return IInfoType::getInitFnPtr ();
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
    
    const char * IFixedValue::getId () {
	return IFixedValue::id ();
    }

    
}
