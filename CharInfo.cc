#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/utils/FixedUtils.hh>

namespace semantic {

    ICharInfo::ICharInfo (bool isConst) :
	IInfoType (isConst)
    {}

    bool ICharInfo::isSame (InfoType other) {
	return other-> is<ICharInfo> ();
    }
	
    InfoType ICharInfo::BinaryOp (Word op, syntax::Expression right) {
	if (op == Token::EQUAL) return Affect (right);
	if (op == Token::MINUS_AFF) return opAff (op, right);
	if (op == Token::PLUS_AFF) return opAff (op, right);
	if (op == Token::INF) return opTest (op, right);
	if (op == Token::SUP) return opTest (op, right);
	if (op == Token::DEQUAL) return opTest (op, right);
	if (op == Token::INF_EQUAL) return opTest (op, right);
	if (op == Token::SUP_EQUAL) return opTest (op, right);
	if (op == Token::NOT_EQUAL) return opTest (op, right);
	if (op == Token::PLUS) return opNorm (op, right);
	if (op == Token::MINUS) return opNorm (op, right);
	return NULL;
    }

    InfoType ICharInfo::BinaryOpRight (Word op, syntax::Expression left) {
	if (op == Token::EQUAL) return AffectRight (left);
	if (op == Token::INF) return opTestRight (op, left);
	if (op == Token::SUP) return opTestRight (op, left);
	if (op == Token::DEQUAL) return opTestRight (op, left);
	if (op == Token::INF_EQUAL) return opTestRight (op, left);
	if (op == Token::SUP_EQUAL) return opTestRight (op, left);
	if (op == Token::NOT_EQUAL) return opTestRight (op, left);
	if (op == Token::PLUS) return opNormRight (op, left);
	if (op == Token::MINUS) return opNormRight (op, left);
	return NULL;
    }

    std::string ICharInfo::innerTypeString () {
	return "char";
    }

    std::string ICharInfo::innerSimpleTypeString () {
	return "a";
    }

    InfoType ICharInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token.getStr () == "init") return Init ();
	if (var-> token.getStr () == "sizeof") return SizeOf ();
	if (var-> token.getStr () == "typeid") return StringOf ();
	return NULL;
    }
    
    InfoType ICharInfo::CastOp (InfoType other) {
	if (other-> is<ICharInfo> ()) return this;
	if (auto ot = other-> to<IFixedInfo> ()) {
	    auto aux = ot-> clone ();
	    //TODO;
	    return aux;
	}
	return NULL;
    }

    InfoType ICharInfo::CompOp (InfoType other) {
	if (other-> is<IUndefInfo> () || other-> is<ICharInfo> ()) {
	    auto ch = new (GC) ICharInfo (this-> isConst ());
	    //TODO
	    return ch;
	} else if (auto en = other-> to<IEnumInfo> ()) {
	    return this-> CompOp (en-> content ());
	}
	return NULL;
    }

    InfoType ICharInfo::clone () {
	auto ret = new (GC) ICharInfo (this-> isConst ());
	//TODO
	return ret;
    }
		
    const char* ICharInfo::getId () {
	return ICharInfo::id ();
    }

    InfoType ICharInfo::Affect (syntax::Expression right) {
	if (right-> info-> type-> is<ICharInfo> ()) {
	    auto ch = new (GC) ICharInfo (this-> isConst ());
	    ch-> binopFoo = &FixedUtils::InstAffect;
	    return ch;
	}
	return NULL;
    }

    InfoType ICharInfo::AffectRight (syntax::Expression left) {
	if (left-> info-> type-> is<IUndefInfo> ()) {
	    auto ch = new (GC) ICharInfo (this-> isConst ());
	    ch-> binopFoo = &FixedUtils::InstAffect;
	    return ch;
	}
	return NULL;
    }

    InfoType ICharInfo::opTest (Word, syntax::Expression right) {
	if (right-> info-> type-> is<ICharInfo> ()) {
	    auto bl = new (GC) IBoolInfo (true);	    
	    //TODO
	    // if (this-> value)
	    //    ch-> value = this-> value-> BinaryOp (op, right-> info-> type-> value);
	    return bl;
	} else if (auto ot = right-> info-> type-> to<IFixedInfo> ()) {
	    if (ot-> type () == FixedConst::UBYTE) {
		auto ch = new (GC) IBoolInfo (true);
		//TODO
		// if (this-> value)
		//    ch-> value = this-> value-> BinaryOp (op, right-> info-> type-> value);
		return ch;
	    }
	}
	return NULL;
    }
    
    InfoType ICharInfo::opTestRight (Word, syntax::Expression left) {
	if (auto ot = left-> info-> type-> to<IFixedInfo> ()) {
	    if (ot-> type () == FixedConst::UBYTE) {
		auto ch = new (GC) IBoolInfo (true);
		//TODO
		// if (this-> value)
		//    ch-> value = this-> value-> BinaryOpRight (op, right-> info-> type-> value);
		return ch;
	    }
	}
	return NULL;
    }
    
    InfoType ICharInfo::opAff (Word, syntax::Expression right) {
	if (right-> info-> type-> is<ICharInfo> ()) {
	    auto ch = new (GC) ICharInfo (false);
	    //TODO ch-> lintInst = CharUtils::InstOpAff ();
	    return ch;
	} else if (auto ot = right-> info-> type-> to<IFixedInfo> ()) {
	    if (ot-> type () == FixedConst::UBYTE) {
		auto ch = new (GC) ICharInfo (false);
		//TODO ch-> lintInst = CharUtils::InstOpAff ();
		return ch;
	    }
	}
	return NULL;
    }

    InfoType ICharInfo::opNorm (Word, syntax::Expression right) {
	if (right-> info-> type-> is<ICharInfo> ()) {
	    auto ch = new (GC) ICharInfo (true);
	    //TODO
	    return ch;
	} else if (auto ot = right-> info-> type-> to<IFixedInfo> ()) {
	    if (ot-> type () == FixedConst::UBYTE) {
		auto ch = new (GC) ICharInfo (true);
		return ch;
	    }
	}
	return NULL;
    }

    InfoType ICharInfo::opNormRight (Word, syntax::Expression left) {
	if (auto ot = left-> info-> type-> to<IFixedInfo> ()) {
	    if (ot-> type () == FixedConst::UBYTE) {
		auto ch = new (GC) ICharInfo (true);
		//TODO
		return ch;
	    }
	}
	return NULL;
    }

    InfoType ICharInfo::Init () {
	auto ch = new (GC) ICharInfo (true);
	//TODO
	return ch;
    }

    InfoType ICharInfo::SizeOf () {
	auto _in = new (GC) IFixedInfo (true, FixedConst::UBYTE);
	//TODO
	return _in;
    }

    InfoType ICharInfo::StringOf () {
	auto str = new (GC) IStringInfo (true);
	//TODO
	return str;
    }

    Ymir::Tree ICharInfo::toGeneric () {
	return char_type_node;
    }
    
}
