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
       
    Value IBoolValue::clone () {
	return new (Z0)  IBoolValue (this-> value);
    }
    
    const char* IBoolValue::getId () {
	return IBoolValue::id ();
    }

    std::string IBoolValue::toString () {
	return Ymir::OutBuffer (this-> value).str ();
    }
    
    ICharValue::ICharValue (char code) :
	code (code)
    {}

    const char* ICharValue::getId () {
	return ICharValue::id ();
    }

    Value ICharValue::clone () {
	return new (Z0)  ICharValue (this-> code);
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
	auto ret = new (Z0)  IString (sym-> sym, this-> value);
	ret-> info = sym;
	return ret;
    }

    
}
