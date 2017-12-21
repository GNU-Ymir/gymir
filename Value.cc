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
}
