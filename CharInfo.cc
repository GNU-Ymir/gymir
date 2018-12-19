#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/tree/_.hh>
#include <ymir/semantic/utils/FixedUtils.hh>
#include <ymir/semantic/value/FixedValue.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/syntax/Keys.hh>

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
	if (op == Token::DDOT) return opRange (op, right);
	if (op == Token::TDOT) {
	    InfoType info = opRange (op, right);
	    if (info != NULL && info-> is <IRangeInfo> ()) info-> to <IRangeInfo> ()-> isInclusive () = true;
	    return info;
	}
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

    InfoType ICharInfo::UnaryOp (Word op) {
	if (op == Token::AND && this-> isLvalue ()) {
	    return toPtr (op);
	} else if (op == Token::DPLUS) {
	    auto ret = new (Z0) ICharInfo (true);
	    ret-> unopFoo = FixedUtils::InstPPlus;
	    if (this-> value ())
		ret-> value () = this-> value ()-> UnaryOp (op);
	    return ret;
	} else if (op == Token::DMINUS) {
	    auto ret = new (Z0) ICharInfo (true);
	    ret-> unopFoo = FixedUtils::InstSSub;
	    if (this-> value ())
		ret-> value () = this-> value ()-> UnaryOp (op);
	    return ret;
	}
	return NULL;
    }

    InfoType ICharInfo::toPtr (const Word &) {
	auto ret = new (Z0)  IPtrInfo (this-> isConst (), this-> clone ());
	ret-> binopFoo = &FixedUtils::InstAddr;
	return ret;
    }
    
    std::string ICharInfo::innerTypeString () {
	return "char";
    }

    std::string ICharInfo::innerSimpleTypeString () {
	return "a";
    }

    InfoType ICharInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	return NULL;
    }

    InfoType ICharInfo::DColonOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token.getStr () == "init") return Init ();
	if (var-> token.getStr () == "sizeof") return SizeOf ();
	if (var-> token == "typeid") return StringOf ();
	if (var-> token == "typeinfo") return TypeInfo ();
	return NULL;
    }

    
    InfoType ICharInfo::CastOp (InfoType other) {
	if (other-> is<ICharInfo> ()) return this;
	if (auto ot = other-> to<IFixedInfo> ()) {
	    auto aux = ot-> clone ();
	    aux-> binopFoo = FixedUtils::InstCast;
	    return aux;
	}
	return NULL;
    }

    InfoType ICharInfo::CompOp (InfoType other) {
	if (other-> is<IUndefInfo> () || other-> is<ICharInfo> ()) {
	    auto ch = new (Z0)  ICharInfo (this-> isConst ());
	    ch-> binopFoo = FixedUtils::InstCast;
	    return ch;
	} else if (auto ref = other-> to <IRefInfo> ()) {
	    if (this-> isLvalue () && this-> isSame (ref-> content ())) {
		auto aux = new (Z0)  IRefInfo (this-> isConst (), this-> clone ());
		aux-> binopFoo = &FixedUtils::InstAddr;
		return aux;
	    }
	} else if (auto en = other-> to<IEnumInfo> ()) {
	    return this-> CompOp (en-> content ());
	}
	return NULL;
    }

    InfoType ICharInfo::onClone () {
	return new (Z0)  ICharInfo (this-> isConst ());	
    }
		
    const char* ICharInfo::getId () {
	return ICharInfo::id ();
    }

    InfoType ICharInfo::Affect (syntax::Expression right) {
	if (right-> info-> type ()-> is<ICharInfo> ()) {
	    auto ch = new (Z0)  ICharInfo (this-> isConst ());
	    ch-> binopFoo = &FixedUtils::InstAffect;
	    return ch;
	}
	return NULL;
    }

    InfoType ICharInfo::AffectRight (syntax::Expression left) {
	if (left-> info-> type ()-> is<IUndefInfo> ()) {
	    auto ch = new (Z0)  ICharInfo (this-> isConst ());
	    ch-> binopFoo = &FixedUtils::InstAffect;
	    return ch;
	}
	return NULL;
    }

    InfoType ICharInfo::opTest (Word op, syntax::Expression right) {
	if (right-> info-> type ()-> is<ICharInfo> ()) {
	    auto bl = new (Z0)  IBoolInfo (true);	    	    
	    bl-> binopFoo = &FixedUtils::InstTest;
	    if (this-> value ()) {
		bl-> value () = this-> value ()-> BinaryOp (op, right-> info-> type ()-> value ());
	    }
	    
	    return bl;
	} else if (auto ot = right-> info-> type ()-> to<IFixedInfo> ()) {
	    if (ot-> type () == FixedConst::UBYTE) {
		auto ch = new (Z0)  IBoolInfo (true);
		ch-> binopFoo = &FixedUtils::InstTest;
		if (this-> value ())
		    ch-> value () = this-> value()-> BinaryOp (op, right-> info-> type ()-> value ());
		return ch;
	    }
	}
	return NULL;
    }
    
    InfoType ICharInfo::opTestRight (Word op, syntax::Expression left) {
	if (auto ot = left-> info-> type ()-> to<IFixedInfo> ()) {
	    if (ot-> type () == FixedConst::UBYTE) {
		auto ch = new (Z0)  IBoolInfo (true);
		ch-> binopFoo = FixedUtils::InstTestRight;
		if (this-> value ())
		    ch-> value () = this-> value ()-> BinaryOpRight (op, left-> info-> type ()-> value ());
		return ch;
	    }
	}
	return NULL;
    }

    InfoType ICharInfo::opRange (Word, syntax::Expression right) {
	if (this-> isSame (right-> info-> type ())) {
	    auto ret = new (Z0) IRangeInfo (false, this-> clone ());
	    ret-> binopFoo = &FixedUtils::InstRange;
	    ret-> leftValue () = this-> value ();
	    ret-> rightValue () = right-> info-> value ();
	    return ret;
	}
	return NULL;
    }
    
    InfoType ICharInfo::opAff (Word, syntax::Expression right) {
	if (right-> info-> type ()-> is<ICharInfo> ()) {
	    auto ch = new (Z0)  ICharInfo (false);
	    ch-> binopFoo = &FixedUtils::InstReaff;	    
	    return ch;
	} else if (auto ot = right-> info-> type ()-> to<IFixedInfo> ()) {
	    if (ot-> type () == FixedConst::UBYTE) {
		auto ch = new (Z0)  ICharInfo (false);
		ch-> binopFoo = &FixedUtils::InstReaff;
		return ch;
	    }
	}
	return NULL;
    }

    InfoType ICharInfo::opNorm (Word op, syntax::Expression right) {
	if (right-> info-> type ()-> is<ICharInfo> ()) {
	    auto ch = new (Z0)  ICharInfo (true);
	    ch-> binopFoo = &FixedUtils::InstNormal;
	    if (this-> value ())
		ch-> value () = this-> value ()-> BinaryOp (op, right-> info-> value ());
	    return ch;
	} else if (auto ot = right-> info-> type ()-> to<IFixedInfo> ()) {
	    if (ot-> type () == FixedConst::UBYTE) {		
		auto ch = new (Z0)  ICharInfo (true);
		ch-> binopFoo = &FixedUtils::InstNormal;
		if (this-> value ())
		    ch-> value () = this-> value ()-> BinaryOp (op, right-> info-> value ());
		return ch;
	    }
	}
	return NULL;
    }

    InfoType ICharInfo::opNormRight (Word op, syntax::Expression left) {
	if (auto ot = left-> info-> type ()-> to<IFixedInfo> ()) {
	    if (ot-> type () == FixedConst::UBYTE) {
		auto ch = new (Z0)  ICharInfo (true);
		ch-> binopFoo = &FixedUtils::InstNormalRight;
		if (this-> value ())
		    ch-> value () = this-> value ()-> BinaryOpRight (op, left-> info-> value ());
		return ch;
	    }
	}
	return NULL;
    }

    InfoType ICharInfo::Init () {
	auto ch = new (Z0)  ICharInfo (true);
	ch-> value () = new (Z0) IFixedValue (FixedConst::UBYTE, '\0', '\0');
	return ch;
    }

    InfoType ICharInfo::SizeOf () {
	auto _in = new (Z0)  IFixedInfo (true, FixedConst::UINT);
	_in-> unopFoo = FixedUtils::InstSizeOf;
	return _in;
    }

    Ymir::Tree ICharInfo::toGeneric () {
	return char_type_node;
    }

    Ymir::Tree ICharInfo::genericConstructor () {
	auto ret = new (Z0) syntax::IChar (Word::eof (), '\0');
	ret-> info = new (Z0) ISymbol (Word::eof (), ret, this);
	return ret-> toGeneric ();    
    }

    Ymir::Tree ICharInfo::genericTypeInfo () {
	Symbol sym  = Table::instance ().getTypeInfoSymbol (Ymir::Runtime::CHAR_INFO);
	auto rtree = sym-> treeDecl ();
	return rtree;
    }
    
}
