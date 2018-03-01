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

    
    std::string IValue::toString () {
	Ymir::Error::assert ((std::string ("TODO") + this-> getId ()).c_str ());
	return "";
    }

    syntax::Expression IValue::toYmir (Symbol) {
	Ymir::Error::assert ("TODO");
	return NULL;
    }
    

    
}
