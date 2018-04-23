#pragma once

#include <ymir/semantic/value/Value.hh>
#include <ymir/utils/BigInt.hh>

namespace semantic {

    class IFrame;
    typedef IFrame* Frame;

    class ILambdaValue : public IValue {

	std::vector <Frame> frame;
	
    public:

	ILambdaValue (Frame);

	ILambdaValue (std::vector <Frame> & frames);

	void push (Frame fr);
	
	const char* getId () override;

	static const char* id () {
	    return "ILambdaValue";
	}

	Value clone () override;

	bool equals (Value) override;
	
	std::string toString () override;
	
	syntax::Expression toYmir (Symbol sym) override;
	
    };

}

