#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/utils/FixedUtils.hh>
#include <ymir/semantic/value/BoolValue.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/ast/Constante.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/semantic/object/AggregateInfo.hh>
#include "print-tree.h"


namespace semantic {

    IBoolInfo::IBoolInfo (bool isConst) :
	IInfoType (isConst)
    {}
    
    bool IBoolInfo::isSame (InfoType other) {
	return other-> to<IBoolInfo> () != NULL;
    }
	
    InfoType IBoolInfo::BinaryOp (Word op, syntax::Expression right) {
	if (op == Token::EQUAL) return Affect (right);
	if (op == Token::DAND) return opNorm (op, right);
	if (op == Token::DPIPE) return opNorm (op, right);
	if (op == Token::PIPE) return opNorm (op, right);
	if (op == Token::AND) return opNorm (op, right);
	if (op == Token::XOR) return opNorm (op, right);
	
	if (op == Token::AND_AFF) return opReaff (op, right);
	if (op == Token::PIPE_EQUAL) return opReaff (op, right);
	if (op == Token::XOR_EQUAL) return opReaff (op, right);
	
	if (op == Token::NOT_EQUAL) return opNorm (op, right);
	if (op == Token::DEQUAL) return opNorm (op, right);
	return NULL;
    }

    InfoType IBoolInfo::BinaryOpRight (Word op, syntax::Expression left) {
	if (op == Token::EQUAL) return AffectRight (left);
	return NULL;
    }

    InfoType IBoolInfo::UnaryOp (Word op) {
	if (op == Token::NOT) {
	    auto ret = new (Z0)  IBoolInfo (false);
	    ret-> unopFoo = FixedUtils::InstNot;
	    //ret-> lintInstS.push_back (FixedUtils::InstXor);
	    if (this-> value ())
		ret-> value () = this-> value ()-> UnaryOp (op);
	    return ret;
	} else if (op == Token::AND && this-> isLvalue ()) return Ptr (op);
	return NULL;
    }

    std::string IBoolInfo::innerTypeString () {
	return std::string ("bool");
    }

    std::string IBoolInfo::innerSimpleTypeString () {
	return "b";
    }

    InfoType IBoolInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	return NULL;
    }

    InfoType IBoolInfo::DColonOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token.getStr () == "init") return Init ();
	if (var-> token.getStr () == "sizeof") return SizeOf ();
	if (var-> token == "typeid") return StringOf ();
	if (var-> token == "typeinfo") return TypeInfo ();
	return NULL;
    }
    
    InfoType IBoolInfo::CastOp (InfoType) {
	//TODO
	return NULL;
    }

    InfoType IBoolInfo::CompOp (InfoType other) {
	if (other-> is<IBoolInfo> () || other-> is<IUndefInfo> ()) {
	    auto bl = new (Z0)  IBoolInfo (this-> isConst ());
	    bl-> binopFoo = FixedUtils::InstCast;
	    return bl;
	} else if (auto en = other-> to<IEnumInfo> ()) {
	    return this-> CompOp (en-> content ());
	} else if (auto ref = other-> to <IRefInfo> ()) {
	    if ((this-> isLvalue () && !this-> isConst ()) && this-> isSame (ref-> content ())) {
		auto aux = new (Z0) IRefInfo (this-> isConst (), this-> clone ());
		aux-> binopFoo = &FixedUtils::InstAddr;
		return aux;
	    }
	}
	return NULL;
    }

    InfoType IBoolInfo::onClone () {
	return new (Z0)  IBoolInfo (this-> isConst ());
    }
		
    InfoType IBoolInfo::Ptr (const Word &) {	
	auto ptr = new (Z0)  IPtrInfo (this-> isConst (), this-> clone ());
	ptr-> binopFoo = &FixedUtils::InstAddr;
	return ptr;
    }

    InfoType IBoolInfo::Affect (syntax::Expression right) {
	if (right-> info-> type ()-> is<IBoolInfo> ()) {
	    auto b = new (Z0)  IBoolInfo (this-> isConst ());
	    b-> binopFoo = FixedUtils::InstAffect;
	    return b;
	}
	return NULL;
    }

    InfoType IBoolInfo::AffectRight (syntax::Expression left) {
	if (left-> info-> type ()-> is<IUndefInfo> ()) {
	    auto b = new (Z0)  IBoolInfo (this-> isConst ());	    
	    b-> binopFoo = FixedUtils::InstAffect;
	    return b;
	}
	return NULL;
    }

    InfoType IBoolInfo::opNorm (Word op, syntax::Expression right) {
	if (right-> info-> type ()-> is<IBoolInfo> ()) {
	    auto b = new (Z0)  IBoolInfo (false);
	    b-> binopFoo = &FixedUtils::InstNormal;
	    if (this-> value ()) {
		b-> value () = this-> value ()-> BinaryOp (op, right-> info-> value ());
	    }
	    return b;
	}
	return NULL;
    }

    InfoType IBoolInfo::opReaff (Word, syntax::Expression right) {
	if (right-> info-> type ()-> is<IBoolInfo> ()) {
	    auto b = new (Z0)  IBoolInfo (false);
	    b-> binopFoo = &FixedUtils::InstReaff;
	    return b;
	}
	return NULL;
    }
    
    InfoType IBoolInfo::Init () {
	auto ret = new (Z0)  IBoolInfo (false);
	ret-> value () = new (Z0) IBoolValue (false);
	return ret;
    }

    InfoType IBoolInfo::SizeOf () {	
	auto ret = new (Z0)  IFixedInfo (false, FixedConst::UINT);
	ret-> unopFoo = FixedUtils::InstSizeOf;
	return ret;
    }

    InfoType IBoolInfo::TypeInfo () {
	auto ret = Table::instance ().getTypeInfo ()-> TempOp ({});
	ret-> unopFoo = FixedUtils::InstTypeInfoBool;
	return ret;
    }
    
    const char* IBoolInfo::getId () {
	return IBoolInfo::id ();
    }

    Ymir::Tree IBoolInfo::toGeneric () {
	return boolean_type_node;
    }
    
    IBoolValue::IBoolValue (bool value) :
	value (value)
    {}
    
    bool& IBoolValue::isTrue () {
	return this-> value;
    }
       
    Value IBoolValue::clone () {
	return new (Z0) IBoolValue (this-> value);
    }
    
    const char* IBoolValue::getId () {
	return IBoolValue::id ();
    }

    std::string IBoolValue::toString () {
	return Ymir::OutBuffer (this-> value).str ();
    }

    Value IBoolValue::BinaryOp (Word op, Value val) {
	if (val == NULL) return NULL;
	if (op == Token::DAND) return this-> dand (val);
	if (op == Token::DPIPE) return this-> dor (val);
	if (op == Token::NOT_EQUAL) return this-> neq (val);
	if (op == Token::XOR) return this-> lxor (val);
	if (op == Token::AND) return this-> dand (val);
	if (op == Token::PIPE) return this-> dor (val);
	if (op == Token::DEQUAL) return this-> eq (val);	
	return NULL;
    }
    
    Value IBoolValue::dand (Value other) {
	if (auto ot = other-> to<IBoolValue> ()) {
	    return new (Z0) IBoolValue (this-> value && ot-> value);
	}
	return NULL;
    }

    Value IBoolValue::dor (Value other) {
	if (auto ot = other-> to<IBoolValue> ()) {
	    return new (Z0) IBoolValue (this-> value || ot-> value);
	}
	return NULL;	
    }

    Value IBoolValue::neq (Value other) {
	if (auto ot = other-> to<IBoolValue> ()) {
	    return new (Z0) IBoolValue (this-> value != ot-> value);
	}
	return NULL; 
    }

    Value IBoolValue::eq (Value other) {
	if (auto ot = other-> to<IBoolValue> ()) {
	    return new (Z0) IBoolValue (this-> value == ot-> value);
	}
	return NULL;	
    }

    Value IBoolValue::lxor (Value other) {
	if (auto ot = other-> to<IBoolValue> ()) {
	    return new (Z0) IBoolValue (this-> value == ot-> value);
	}
	return NULL;	
    }
    
    Value IBoolValue::UnaryOp (Word op) {
	if (op == Token::NOT) {
	    return new (Z0) IBoolValue (!this-> value);
	}
	return NULL;
    }

    syntax::Expression IBoolValue::toYmir (Symbol sym) {
	auto ret = new (Z0) syntax::IBool (sym-> sym);
	ret-> value () = this-> value;
	ret-> info = sym;
	return ret;
    }
    
    bool IBoolValue::equals (Value other) {
	if (auto ot = other-> to <IBoolValue> ()) {
	    return this-> value == ot-> value;
	}
	return false;
    }
    

    
}


