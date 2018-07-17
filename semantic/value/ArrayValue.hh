#pragma once

#include <ymir/semantic/value/Value.hh>

namespace semantic {

    class IArrayValue : public IValue {

	std::vector <Value> _values;

    public:

	IArrayValue ();

	void addValue (Value val);

	Value BinaryOp (Word, Value) override;
	
	Value AccessOp (syntax::Expression) override;
	
	const char * getId () override;

	static const char * id () {
	    return "IStringValue";
	}

	Value getLen ();

	std::string toString () override;

	syntax::Expression toYmir (Symbol sym);

	bool equals (Value) override;

	Value clone () override;	
	
    };
    
}
