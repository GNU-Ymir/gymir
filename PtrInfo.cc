#include <ymir/semantic/types/_.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/semantic/utils/PtrUtils.hh>

namespace semantic {

    IPtrInfo::IPtrInfo (bool isConst) :
	IInfoType (isConst)	
    {
	Ymir::Error::assert ("WH!!");
    }
    
    IPtrInfo::IPtrInfo (bool isConst, InfoType type) :
	IInfoType (isConst),
	_content (type) 
    {}
    
   
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
	else if (token == Token::MINUS) return SubRight (left);
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
	    auto ret = new IPtrInfo (false, this-> _content-> clone ());
	    ret-> binopFoo = &PtrUtils::InstAffect;
	    return ret;
	} else if (type && this-> _content-> is<IVoidInfo> ()) {
	    this-> _content = type-> _content-> clone ();
	    auto ret = new IPtrInfo (false, this-> _content-> clone ());
	    ret-> binopFoo = &PtrUtils::InstAffect;
	    return ret;
	} else if (type && type-> _content-> is <IVoidInfo> ()) {
	    auto ret = new IPtrInfo (false, type-> _content-> clone ());
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
	    auto ret = new IPtrInfo (false, this-> _content-> clone ());
	    ret-> binopFoo = &PtrUtils::InstAffect;
	    return ret;
	}
	return NULL;
    }

    InfoType IPtrInfo::Plus (syntax::Expression right) {
	if (right-> info-> type-> is<IFixedInfo> ()) {
	    auto ptr = new IPtrInfo (this-> isConst (), this-> _content-> clone ());
	    //ptr-> binopFoo = &PtrUtils::InstPlus
	    return ptr;
	}
	return NULL;
    }
    
    InfoType IPtrInfo::Sub (syntax::Expression right) {
	if (right-> info-> type-> is<IFixedInfo> ()) {
	    auto ptr = new IPtrInfo (this-> isConst (), this-> _content-> clone ());
	    //ptr-> binopFoo = &PtrUtils::InstSub
	    return ptr;
	}
	return NULL;
    }

    InfoType IPtrInfo::PlusRight (syntax::Expression right) {
	if (right-> info-> type-> is<IFixedInfo> ()) {
	    auto ptr = new IPtrInfo (this-> isConst (), this-> _content-> clone ());
	    //ptr-> binopFoo = &PtrUtils::InstPlus
	    return ptr;
	}
	return NULL;
    }
    
    InfoType IPtrInfo::SubRight (syntax::Expression right) {
	if (right-> info-> type-> is<IFixedInfo> ()) {
	    auto ptr = new IPtrInfo (this-> isConst (), this-> _content-> clone ());
	    //ptr-> binopFoo = &PtrUtils::InstSub
	    return ptr;
	}
	return NULL;
    }

    InfoType IPtrInfo::Is (syntax::Expression right) {
	if (right-> info-> type-> is <IPtrInfo> ()) {
	    auto ret = new IBoolInfo (true);
	    //ret-> binopFoo = &PtrUtils::InstIs
	    return ret;
	} else if (right-> info-> type-> is <INullInfo> ()) {
	    auto ret = new IBoolInfo (true);
	    //ret-> binopFoo = &PtrUtils::InstIsNull
	    return ret;
	}
	return NULL;
    }

    InfoType IPtrInfo::NotIs (syntax::Expression right) {
	if (right-> info-> type-> is <IPtrInfo> ()) {
	    auto ret = new IBoolInfo (true);
	    //ret-> binopFoo = &PtrUtils::InstNotIs
	    return ret;
	} else if (right-> info-> type-> is <INullInfo> ()) {
	    auto ret = new IBoolInfo (true);
	    //ret-> binopFoo = &PtrUtils::InstNotIsNull
	    return ret;
	}
	return NULL;
    }

    InfoType IPtrInfo::Unref () {
	if (this-> _content-> is<IUndefInfo> ()) return NULL;
	else if (this-> _content-> is <IVoidInfo> ()) return NULL;
	auto ret = this-> _content-> clone ();
	//ret-> unopFoo = &PtrUtils::InstUnref;
	return ret;
    }

    InfoType IPtrInfo::toPtr () {
	auto ret = new IPtrInfo (this-> isConst (), this-> clone ());
	// ret-> unopFoo = &PtrUtils::InstPtr;
	return ret;
    }

    InfoType IPtrInfo::DotOp (syntax::Var var) {
	//if (var-> hasTemplate ()) return NULL;
	if (var-> isType ()) {
	    auto type = var-> asType ();
	    auto ret = type-> info-> type;
	    //ret-> binopFoo = &PtrUtils::InstUnrefTyped
	    return ret;
	} else if (var-> token == "init") {
	    auto type = this-> clone ();
	    //type-> unopFoo = &PtrUtils::InstNull;
	    return type;
	} else if (var-> token == "typeid") {
	    auto str = new IStringInfo (true);
	    //str-> value = new IStringValue (this-> typeString ());
	    return str;
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
	    auto ptr = new IPtrInfo (this-> isConst (), type-> _content-> clone ());
	    //ptr-> binopFoo = &PtrUtils::InstCast
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
	    ptr-> binopFoo = &PtrUtils::InstAffect;
	    return ptr;
	}
	return NULL;
    }
    
    std::string IPtrInfo::innerTypeString () {
	return "ptr!" + this-> _content-> innerTypeString ();
    }

    std::string IPtrInfo::simpleTypeString () {
	return "P" + this-> _content-> simpleTypeString ();
    }

    InfoType IPtrInfo::clone () {
	return new IPtrInfo (this-> isConst (), this-> _content-> clone ());
    }
    
    InfoType IPtrInfo::ConstVerif (InfoType other) {
	if (this-> isConst () && !other-> isConst ()) return NULL;
	else if (!this-> isConst () && other-> isConst ())
	    this-> isConst () = true;
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


    namespace PtrUtils {

	Ymir::Tree InstAffect (Word locus, syntax::Expression left, syntax::Expression right) {
	    auto ltree = left-> toGeneric ();
	    auto rtree = right-> toGeneric ();
	    auto typeTree = left-> info-> type-> toGeneric ();
	    return Ymir::buildTree (
		MODIFY_EXPR, locus.getLocus (), typeTree.getTree (), ltree, rtree
	    );	    
	}
	
    }
}