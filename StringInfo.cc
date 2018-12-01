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
        
    IStringInfo::IStringInfo (bool isConst, bool inner) :
	IArrayInfo (isConst, new (Z0) ICharInfo (inner))
    {
    }

    InfoType IStringInfo::BinaryOp (Word op, syntax::Expression right) {
	if (op == Token::DEQUAL) {
	    if (right-> info-> type ()-> isSame (this)) {
		if (this-> value ()) {
		    auto aux = new (Z0) IBoolInfo (true);
		    aux-> value () = this-> value ()-> BinaryOp (op, right-> info-> value ());
		    aux-> binopFoo = &ArrayUtils::InstEquals;
		    return aux;
		}
	    }
	}
	return IArrayInfo::BinaryOp (op, right);
    }
    
    InfoType IStringInfo::BinaryOpRight (Word op, syntax::Expression left) {
	if (op == Token::EQUAL) {
	    if (left-> info-> type ()-> is <IUndefInfo> ()) {
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
    }
    
    InfoType IStringInfo::onClone () {
	auto aux = new IStringInfo (this-> isConst ());
	aux-> value () = this-> value ();
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
	} else if (op == Token::DEQUAL) {
	    if (auto ot = other-> to <IStringValue> ()) {
		return new (Z0) IBoolValue (this-> value == ot-> value);
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
	auto lit = build_string_literal (this-> value.length () + 1, this-> value.c_str ());
	vec<constructor_elt, va_gc> * elms = NULL;
	auto type = sym-> type ()-> toGeneric ();
	auto sym_ = new (Z0) ISymbol (sym-> sym, NULL, new (Z0) IFixedInfo (true, FixedConst::ULONG));
	CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (type, "len").getTree (), (new (Z0) IFixedValue (FixedConst::ULONG, this-> value.length (), 0))-> toYmir (sym_)-> toGeneric ().getTree ());
	CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (type, "ptr").getTree (), lit);
	
	return new (Z0) syntax::ITreeExpression (sym-> sym, sym-> type (), build_constructor (type.getTree (), elms));
    }

    
}
