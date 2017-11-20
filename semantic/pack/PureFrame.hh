#pragma once

#include "Frame.hh"
#include "FrameProto.hh"
#include <ymir/ast/Function.hh>
#include <ymir/ast/ParamList.hh>

namespace semantic {

    class IPureFrame : public IFrame {
	std::string name;
	FrameProto proto = NULL;
	bool isValide = false;
	bool pass = false;

    public:

	IPureFrame (Namespace space, syntax::Function fun);

	FrameProto validate (syntax::ParamList) override ;

	FrameProto validate () override ;

	static const char* id () {
	    return "IPureFrame";
	}

	virtual const char* getId ();
	
    private:
	
	FrameProto validateMain ();
	
    };
    
}
