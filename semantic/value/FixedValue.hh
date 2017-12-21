#pragma once

#include <ymir/semantic/value/Value.hh>
#include <ymir/ast/Constante.hh>

namespace semantic {

    union signedun {
	long l;
	ulong ul;
    };
    
    class IFixedValue : public IValue {

	signedun value;
	FixedConst type;
	
    public:

	IFixedValue (FixedConst cst, ulong ul, long l);

	ulong getUValue ();

	long getValue ();
	
	const char* getId () override;

	static const char* id () {
	    return "IFixedValue";
	}
	
    };

}
