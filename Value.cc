#include <ymir/semantic/value/_.hh>
#include <ymir/semantic/types/InfoType.hh>
#include <ymir/ast/_.hh>

namespace semantic {
    using namespace syntax;
    
    Value IValue::BinaryOp (Word , Value ) { return NULL; }

    Value IValue::BinaryOpRight (Word , Value) { return NULL; }

    Value IValue::UnaryOp (Word ) { return NULL; }

    Value IValue::AccessOp (ParamList ) { return NULL; } 
    
    Value IValue::AccessOp (Expression) { return NULL; }

    Value IValue::CastOp (InfoType ) { return NULL; }

    Value IValue::CompOp (InfoType ) { return NULL; }

    Value IValue::CastTo (InfoType ) { return NULL; }

    Value IValue::DotOp (Var ) { return NULL; }


    IBoolValue::IBoolValue (bool value) :
	value (value)
    {}
    
    bool IBoolValue::isTrue () {
	return this-> value;
    }

    const char* IBoolValue::getId () {
	return IBoolValue::id ();
    }
    
    ICharValue::ICharValue (char code) :
	code (code)
    {}

    const char* ICharValue::getId () {
	return ICharValue::id ();
    }

    std::string IValue::toString () {
	Ymir::Error::assert ((std::string ("TODO") + this-> getId ()).c_str ());
	return "";
    }

    syntax::Expression IValue::toYmir (Symbol) {
	Ymir::Error::assert ("TODO");
	return NULL;
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

    std::string IStringValue::toString () {
	return this-> value;
    }

    syntax::Expression IStringValue::toYmir (Symbol sym) {
	auto ret = new (GC) IString (sym-> sym, this-> value);
	ret-> info = sym;
	return ret;
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

    
}
