#pragma once

#include <ymir/semantic/value/Value.hh>

namespace semantic {

    class IBoolValue : public IValue {

	bool value;
	
    public:

	IBoolValue (bool value);

	bool isTrue ();

	const char* getId () override;

	static const char* id () {
	    return "IBoolValue";
	}
	
	Value clone () override;

    };

}
