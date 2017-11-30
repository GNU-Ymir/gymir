#include <ymir/semantic/value/_.hh>
#include <ymir/semantic/types/InfoType.hh>
#include <ymir/ast/_.hh>

namespace semantic {
    using namespace syntax;
    
    Value IValue::BinaryOp (Word token, Value right) { return NULL; }

    Value IValue::BinaryOpRight (Word token, Value left) { return NULL; }

    Value IValue::UnaryOp (Word token) { return NULL; }

    Value IValue::AccessOp (ParamList params) { return NULL; } 
    
    Value IValue::AccessOp (Expression expr) { return NULL; }

    Value IValue::CastOp (InfoType type) { return NULL; }

    Value IValue::CompOp (InfoType type) { return NULL; }

    Value IValue::CastTo (InfoType type) { return NULL; }

    Value IValue::DotOp (Var attr) { return NULL; }


    IBoolValue::IBoolValue (bool value) :
	value (value)
    {}
    
    bool IBoolValue::isTrue () {
	return this-> value;
    }

    const char* IBoolValue::getId () {
	return IBoolValue::id ();
    }
    
    IFixedValue::IFixedValue (std::string value) :
	value (value)
    {}    
    
    const char* IFixedValue::getId ()  {
	return IFixedValue::id ();
    }

    ICharValue::ICharValue (char code) :
	code (code)
    {}

    const char* ICharValue::getId () {
	return ICharValue::id ();
    }
}
