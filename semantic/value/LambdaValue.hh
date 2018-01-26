#pragma once


#pragma once

#include <ymir/semantic/value/Value.hh>
#include <ymir/utils/BigInt.hh>
#include <ymir/semantic/pack/LambdaFrame.hh>

namespace semantic {

    class ILambdaValue : public IValue {

	LambdaFrame frame;
	
    public:

	ILambdaValue (LambdaFrame);

	const char* getId () override;

	static const char* id () {
	    return "ILambdaValue";
	}

	Value clone () override;

	std::string toString () override;
	
	syntax::Expression toYmir (Symbol sym) override;
	
    };

}

