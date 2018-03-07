#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/utils/StringUtils.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/ast/TreeExpression.hh>
#include <ymir/ast/ParamList.hh>
#include <ymir/errors/Error.hh>
#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/semantic/pack/InternalFunction.hh>
#include <ymir/semantic/value/_.hh>
#include <ymir/ast/_.hh>
#include <ymir/semantic/utils/ArrayUtils.hh>

namespace semantic {
        
    IStringInfo::IStringInfo (bool isConst) :
	IArrayInfo (isConst, new (Z0) ICharInfo (false))
    {
	this-> isText () = isConst;
    }
    
    InfoType IStringInfo::BinaryOpRight (Word op, syntax::Expression left) {
	if (op == Token::EQUAL) {
	    if (left-> info-> type-> is <IUndefInfo> ()) {
		auto arr = this-> clone ();
		arr-> binopFoo = &ArrayUtils::InstAffect;
		return arr;
	    }
	}
	return IArrayInfo::BinaryOpRight (op, left);
    }

    bool IStringInfo::isConst () {
	return IInfoType::isConst ();
    }
    
    void IStringInfo::isConst (bool isConst) {
	IInfoType::isConst (isConst);
	this-> isText () = isConst;
    }
    
    InfoType IStringInfo::onClone () {
	auto aux = new IStringInfo (this-> isConst ());
	aux-> value () = this-> value ();
	aux-> isText () = this-> isConst ();
	return aux;
    }

    IStringValue::IStringValue (std::string &val)
	: value (val)
    {}

    IStringValue::IStringValue (const char* val)
	: value (val)
    {}

    const char* IStringValue::getId () {
	return IStringValue::id ();
    }

    Value IStringValue::BinaryOp (Word op, Value other) {
	if (other == NULL) return NULL;
	if (op == Token::TILDE) {
	    if (auto ot = other-> to <IStringValue> ()) {
		return new (Z0) IStringValue ((this-> value + ot-> value).c_str ());
	    }
	}
	return NULL;
    }

    Value IStringValue::AccessOp (syntax::Expression expr) {
	if (expr-> info-> value () == NULL) return NULL;
	else if (auto fix = expr-> info-> value ()-> to<IFixedValue> ()) {
	    auto index = fix-> getUValue ();
	    if (index >= this-> value.length ()) {
		Ymir::Error::overflowArray (expr-> token, index, this-> value.length ());
		return NULL;
	    } else {
		return new (Z0) IFixedValue (FixedConst::UBYTE, this-> value [index], this-> value [index]);
	    }
	}
	return NULL;
    }

    Value IStringValue::getLen () {
	return new (Z0) IFixedValue (FixedConst::ULONG, this-> value.length (), this-> value.length ());
    }
    
    std::string IStringValue::toString () {
	return this-> value;
    }

    Value IStringValue::clone () {
	return new (Z0)  IStringValue (this-> value);
    }

    bool IStringValue::equals (Value other) {
	if (auto ot = other-> to<IStringValue> ()) {
	    return this-> value == ot-> value;
	}
	return false;
    }
    
    syntax::Expression IStringValue::toYmir (Symbol sym) {
	auto ret = new (Z0) syntax::IString (sym-> sym, this-> value);
	ret-> info = sym;
	return ret;
    }

    
}
