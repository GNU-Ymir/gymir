#pragma once

#include "Frame.hh"
#include "FrameProto.hh"
#include <ymir/ast/Function.hh>
#include <ymir/ast/ParamList.hh>

namespace semantic {

    class IPureFrame : public IFrame {
	std::string name;
	FrameProto proto = NULL;
	bool isValid = false;
	bool pass = false;
	bool _isExternC = false;
	
    public:

	IPureFrame (Namespace space, syntax::Function fun);

	FrameProto validate (syntax::ParamList) override ;

	FrameProto validate (const std::vector<InfoType> &) override ;
	
	FrameProto validate () override ;

	bool isPure () override;
	
	std::string getName () override;
	
	std::vector <InfoType> getParamTypes () override;

	InfoType getRetType () override;
	
	
	static const char* id () {
	    return "IPureFrame";
	}

	virtual const char* getId ();
	
    private:
	
	FrameProto validateMain ();
	
    };
    
}
