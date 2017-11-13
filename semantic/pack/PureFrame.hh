#pragma once

#include "Frame.hh"
#include "FrameProto.hh"
#include "../../ast/Function.hh"
#include "../../ast/ParamList.hh"

namespace semantic {

    class IPureFrame : public IFrame {
	std::string name;
	FrameProto proto = NULL;
	bool isValide = false;
	bool pass = false;

    public:

	IPureFrame (Namespace space, Function fun);

	FrameProto validate (ParamList) override ;

	FrameProto validate () override ;

    private:
	
	FrameProto validateMain ();
	
    };
    
}
