#include <ymir/semantic/types/_.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/semantic/utils/PtrUtils.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/utils/FixedUtils.hh>
#include <ymir/semantic/value/FixedValue.hh>

namespace semantic {

    namespace PtrUtils {
	using namespace syntax;
	
	Ymir::Tree InstAffect (Word locus, InfoType, Expression left, Expression right) {
	    auto ltree = left-> toGeneric ();
	    auto rtree = right-> toGeneric ();
	    auto typeTree = left-> info-> type-> toGeneric ();
	    return Ymir::buildTree (
		MODIFY_EXPR, locus.getLocus (), typeTree.getTree (), ltree, rtree
	    );	    
	}
	
	Ymir::Tree InstCast (Word locus, InfoType, Expression elem, Expression typeExpr) {
	    auto type = typeExpr-> info-> type-> toGeneric ();
	    auto lexp = elem-> toGeneric ();
	    return fold_convert_loc (locus.getLocus (), type.getTree (), lexp.getTree ());
	}

	Ymir::Tree InstPlus (Word locus, InfoType, Expression left, Expression right) {
	    auto ltree = left-> toGeneric ();
	    Ymir::Tree rtree = convert (long_unsigned_type_node, right-> toGeneric ().getTree ());
	    tree_code code = POINTER_PLUS_EXPR;
	    return Ymir::buildTree (
		code, locus.getLocus (), ltree.getType (), ltree, rtree
	    );
	}

	Ymir::Tree InstPlusRight (Word locus, InfoType, Expression left, Expression right) {
	    auto ltree = right-> toGeneric ();
	    Ymir::Tree rtree = convert (long_unsigned_type_node, left-> toGeneric ().getTree ());
	    tree_code code = POINTER_PLUS_EXPR;
	    return Ymir::buildTree (
		code, locus.getLocus (), ltree.getType (), ltree, rtree
	    );
	}
	
	Ymir::Tree InstSub (Word locus, InfoType, Expression left, Expression right) {
	    auto ltree = left-> toGeneric ();
	    Ymir::Tree rtree = Ymir::buildTree (
		NEGATE_EXPR, locus.getLocus (), long_unsigned_type_node,
		convert (long_unsigned_type_node, right-> toGeneric ().getTree ())
	    );
	    
	    tree_code code = POINTER_PLUS_EXPR;
	    return Ymir::buildTree (
		code, locus.getLocus (), ltree.getType (), ltree, rtree
	    );
	}
	
	Ymir::Tree InstUnref (Word locus, InfoType type, Expression elem) {
	    auto inner = type-> toGeneric ();
	    return Ymir::getPointerUnref (locus.getLocus (), elem-> toGeneric (), inner, 0);
	}
	
    }
    
    IPtrInfo::IPtrInfo (bool isConst) :
	IInfoType (isConst),
	_content (NULL)
    {
	Ymir::Error::assert ("WH!!");
    }
    
    IPtrInfo::IPtrInfo (bool isConst, InfoType type) :
	IInfoType (isConst),
	_content (type) 
    {}    
   
    InfoType IPtrInfo::create (Word tok, const std::vector<syntax::Expression> & tmps) {
	if (tmps.size () != 1 || !tmps [0]-> is<syntax::IType> ()) {
	    if (!tmps [0]-> info-> type-> isType ()) {
		Ymir::Error::takeATypeAsTemplate (tok);
		return NULL;
	    }
	}	    	    

	if (auto ot = tmps [0]-> info-> type-> to<IStructCstInfo> ()) {
	    auto type = ot-> TempOp ({});
	    if (type == NULL) return NULL;
	    return new (Z0) IPtrInfo (false, type);
	} else {
	    return new (Z0) IPtrInfo (false, tmps [0]-> info-> type);
	}
    }

    const char * IPtrInfo::getId () {
	return IPtrInfo::id ();
    }

    InfoType IPtrInfo::BinaryOp (Word token, syntax::Expression right) {
	if (token == Token::EQUAL) return Affect (right);
	else if (token == Token::PLUS) return Plus (right);
	else if (token == Token::MINUS) return Sub (right);
	else if (token == Keys::IS) return Is (right);
	else if (token == Keys::NOT_IS) return NotIs (right);
	return NULL;
    }

    InfoType IPtrInfo::BinaryOpRight (Word token, syntax::Expression left) {
	if (token == Token::EQUAL) return AffectRight (left);
	else if (token == Token::PLUS) return PlusRight (left);
	return NULL;
    }

    InfoType IPtrInfo::UnaryOp (Word op) {
	if (op == Token::STAR) return Unref ();
	//TODO : isConst () -> isImmutable ()
	else if (op == Token::AND && !this-> isConst ()) return toPtr ();
	return NULL;
    }

    InfoType IPtrInfo::Affect (syntax::Expression right) {
	auto type = right-> info-> type-> to<IPtrInfo> ();
	if (type != NULL && type-> _content-> isSame (this-> _content)) {
	    auto ret = new (Z0)  IPtrInfo (false, this-> _content-> clone ());
	    ret-> binopFoo = &PtrUtils::InstAffect;
	    return ret;
	} else if (type && this-> _content-> is<IVoidInfo> ()) {
	    this-> _content = type-> _content-> clone ();
	    auto ret = new (Z0)  IPtrInfo (false, this-> _content-> clone ());
	    ret-> binopFoo = &PtrUtils::InstAffect;
	    return ret;
	} else if (type && type-> _content-> is <IVoidInfo> ()) {
	    auto ret = new (Z0)  IPtrInfo (false, type-> _content-> clone ());
	    ret-> binopFoo = &PtrUtils::InstAffect;
	    return ret;
	} else if (right-> info-> type-> is <INullInfo> ()) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &PtrUtils::InstAffect;
	    return ret;
	}
	return NULL;
    }

    InfoType IPtrInfo::AffectRight (syntax::Expression left) {
	if (left-> info-> type-> is<IUndefInfo> ()) {
	    auto ret = new (Z0)  IPtrInfo (false, this-> _content-> clone ());
	    ret-> binopFoo = &PtrUtils::InstAffect;
	    return ret;
	}
	return NULL;
    }

    InfoType IPtrInfo::Plus (syntax::Expression right) {
	if (right-> info-> type-> is<IFixedInfo> ()) {
	    auto ptr = new (Z0)  IPtrInfo (this-> isConst (), this-> _content-> clone ());
	    ptr-> binopFoo = &PtrUtils::InstPlus;
	    return ptr;
	}
	return NULL;
    }
    
    InfoType IPtrInfo::Sub (syntax::Expression right) {
	if (right-> info-> type-> is<IFixedInfo> ()) {
	    auto ptr = new (Z0)  IPtrInfo (this-> isConst (), this-> _content-> clone ());
	    ptr-> binopFoo = &PtrUtils::InstSub;
	    return ptr;
	}
	return NULL;
    }

    InfoType IPtrInfo::PlusRight (syntax::Expression right) {
	if (right-> info-> type-> is<IFixedInfo> ()) {
	    auto ptr = new (Z0)  IPtrInfo (this-> isConst (), this-> _content-> clone ());
	    ptr-> binopFoo = &PtrUtils::InstPlusRight;
	    return ptr;
	}
	return NULL;
    }
    

    InfoType IPtrInfo::Is (syntax::Expression right) {
	if (right-> info-> type-> is <IPtrInfo> ()) {
	    auto ret = new (Z0)  IBoolInfo (true);
	    ret-> binopFoo = &FixedUtils::InstTest;
	    return ret;
	} else if (right-> info-> type-> is <INullInfo> ()) {
	    auto ret = new (Z0)  IBoolInfo (true);
	    ret-> binopFoo = &FixedUtils::InstTest;
	    return ret;
	}
	return NULL;
    }
    
    InfoType IPtrInfo::NotIs (syntax::Expression right) {
	if (right-> info-> type-> is <IPtrInfo> ()) {
	    auto ret = new (Z0)  IBoolInfo (true);
	    ret-> binopFoo = &FixedUtils::InstTest;
	    return ret;
	} else if (right-> info-> type-> is <INullInfo> ()) {
	    auto ret = new (Z0)  IBoolInfo (true);
	    ret-> binopFoo = &FixedUtils::InstTest;
	    return ret;
	}
	return NULL;
    }
    
    InfoType IPtrInfo::Unref () {
	if (this-> _content-> is<IUndefInfo> ()) return NULL;
	else if (this-> _content-> is <IVoidInfo> ()) return NULL;
	else {
	    auto ret = this-> _content-> clone ();
	    ret-> isConst (false);
	    ret-> unopFoo = &PtrUtils::InstUnref;
	    return ret;
	}
    }

    InfoType IPtrInfo::toPtr () {
	auto ret = new (Z0)  IPtrInfo (this-> isConst (), this-> clone ());
	ret-> binopFoo = &FixedUtils::InstAddr;
	return ret;
    }

    InfoType IPtrInfo::DotOp (syntax::Var var) {
	if (var-> isType ()) {
	    auto type = var-> asType ();
	    auto ret = type-> info-> type;
	    //ret-> binopFoo = &PtrUtils::InstUnrefTyped
	    return ret;
	}
	return NULL;
    }
    
    InfoType IPtrInfo::DColonOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "init") {
	    auto type = this-> clone ();
	    type-> value () = new (Z0) IFixedValue (FixedConst::ULONG, 0, 0);
	    return type;
	} else if (var-> token == "typeid") {
	    return StringOf ();
	}
	return NULL;
    }
    
    InfoType& IPtrInfo::content () {
	return this-> _content;
    }

    InfoType IPtrInfo::CastOp (InfoType other) {
	auto type = other-> to<IPtrInfo> ();
	if (type && type-> _content-> isSame (this-> _content)) {
	    return this;
	} else if (type) {
	    auto ptr = new (Z0)  IPtrInfo (this-> isConst (), type-> _content-> clone ());
	    ptr-> binopFoo = &PtrUtils::InstCast;
	    return ptr;
	    // }  else if (auto tu = other-> to <ITupleInfo> ()) {
	} else if (auto ul = other-> to <IFixedInfo> ()) {
	    if (ul-> type () == FixedConst::ULONG) {
		auto ot = ul-> cloneOnExit ();
		// ot-> binopFoo = &PtrUtils::InstCast
		return ot;
	    }
	}
	return NULL;
    }

    InfoType IPtrInfo::CompOp (InfoType other) {
	if (other-> isSame (this) || other-> is<IUndefInfo> ()) {
	    auto ptr = this-> clone ();
	    ptr-> binopFoo = &PtrUtils::InstCast;
	    return ptr;
	}
	return NULL;
    }
    
    std::string IPtrInfo::innerTypeString () {
	return "ptr!" + this-> _content-> innerTypeString ();
    }

    std::string IPtrInfo::innerSimpleTypeString () {
	return "P" + this-> _content-> simpleTypeString ();
    }

    InfoType IPtrInfo::onClone () {
	return new (Z0) IPtrInfo (this-> isConst (), this-> _content-> clone ());
    }
    
    InfoType IPtrInfo::ConstVerif (InfoType other) {
	if (this-> isConst () && !other-> isConst ()) return NULL;
	else if (!this-> isConst () && other-> isConst ())
	    this-> isConst (true);
	return this;
    }

    bool IPtrInfo::isSame (InfoType other) {
	if (auto ot = other-> to<IPtrInfo> ()) {
	    return ot-> _content-> isSame (this-> _content); 
	}
	return NULL;
    }

    Ymir::Tree IPtrInfo::toGeneric () {
	Ymir::Tree inner = this-> _content-> toGeneric ();
	return build_pointer_type (
	    inner.getTree ()
	);
    }
    
    InfoType IPtrInfo::getTemplate (ulong nb) {
	if (nb == 0) return this-> _content;
	return NULL;
    }


}
