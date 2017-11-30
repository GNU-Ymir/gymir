#pragma once

#include <ymir/semantic/value/Value.hh>
#include <ymir/utils/BigInt.hh>

namespace semantic {

    class ICharValue : public IValue {

	char code;
	
    public:

	ICharValue (char);

	const char* getId () override;

	static const char* id () {
	    return "ICharValue";
	}
	
    };

}
