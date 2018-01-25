#pragma once

#include <ymir/semantic/value/Value.hh>
#include <ymir/ast/Constante.hh>

namespace semantic {

    union doublePrec {
	float f;
	double d;
    };
    
    class IFloatValue : public IValue {

	doublePrec value;
	FloatConst type;
	
    public:

	IFloatValue (FloatConst cst, float ul, double l);

	float& getFloat ();

	double& getDouble ();

	Value BinaryOp (Word , Value ) override;
	
	const char* getId () override;

	static const char* id () {
	    return "IFloatValue";
	}

	std::string toString () override;

	syntax::Expression toYmir (Symbol sym) override;
	
	bool equals (Value) override;

	Value clone () override;
	
    private :

	Value add (Value other);
	Value sub (Value other);
	Value div (Value other);
	Value mul (Value other);
	Value inf (Value other);
	Value sup (Value other);
	Value infeq (Value other);
	Value supeq (Value other);
	Value neq (Value other);
	Value eq (Value other);
    };

}
