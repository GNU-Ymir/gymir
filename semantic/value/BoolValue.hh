#pragma once

#include <ymir/semantic/value/Value.hh>

namespace semantic {

    class IBoolValue : public IValue {

	bool value;
	
    public:

	IBoolValue (bool value);

	bool& isTrue ();

	Value BinaryOp (Word , Value ) override;

	Value UnaryOp (Word) override;
	
	const char* getId () override;

	static const char* id () {
	    return "IBoolValue";
	}
	
	Value clone () override;

	syntax::Expression toYmir (Symbol sym) override;

	bool equals (Value) override;
	
	std::string toString () override;

    private:

	Value dand (Value);
	Value dor (Value);
	Value neq (Value);
	Value eq (Value);
	Value lxor (Value);
	
	
    };

}
