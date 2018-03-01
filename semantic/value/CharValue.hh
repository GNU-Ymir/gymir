#pragma once

#include <ymir/semantic/value/Value.hh>
#include <ymir/utils/BigInt.hh>

namespace semantic {

    class ICharValue : public IValue {

	char code;
	
    public:

	ICharValue (char);

	Value BinaryOp (Word, Value) override;

	Value UnaryOp (Word) override;
	
	const char* getId () override;

	static const char* id () {
	    return "ICharValue";
	}

	Value clone () override;

	syntax::Expression toYmir (Symbol sym) override;
	
    };

}
