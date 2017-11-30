#pragma once

#include <ymir/semantic/value/Value.hh>
#include <ymir/utils/BigInt.hh>

namespace semantic {

    class IFixedValue : public IValue {

	Ymir::BigInt value;
	
    public:

	IFixedValue (std::string value);

	const char* getId () override;

	static const char* id () {
	    return "IFixedValue";
	}
	
    };

}
