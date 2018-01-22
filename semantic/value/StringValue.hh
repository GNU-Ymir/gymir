#pragma once

#include <ymir/semantic/value/Value.hh>

namespace semantic {
    
    class IStringValue : public IValue {

	std::string value;
	
    public:

	IStringValue (std::string &val);

	IStringValue (const char * val);
	
	const char * getId () override;

	static const char * id () {
	    return "IStringValue";
	}
	
	std::string toString () override;

	syntax::Expression toYmir (Symbol sym);	
    };
    
}
