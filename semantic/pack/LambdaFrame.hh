#pragma once

#include "Frame.hh"
#include "FrameProto.hh"
#include <ymir/ast/LambdaFunc.hh>
#include <ymir/ast/ParamList.hh>

namespace semantic {

    class ILambdaFrame : public IFrame {

	syntax::LambdaFunc frame;
	std::string name;

	bool _isPure;
	bool _isMoved;

    public:

	ILambdaFrame (Namespace space, std::string& name, syntax::LambdaFunc func);

	FrameProto validate (syntax::ParamList params) override;

	FrameProto validate (const std::vector<InfoType> & params) override;

	ApplicationScore isApplicable (syntax::ParamList params) override;

	ApplicationScore isApplicable (const std::vector <InfoType> & params) override;
	
	static const char * id () {
	    return "ILambdaFrame";
	}

	bool isPure () override;

	void isPure (bool);

	bool& isMoved ();
	
	std::vector <InfoType> getParamTypes () override;

	InfoType getRetType () override;
	
	std::string getName ();
	
	syntax::LambdaFunc func ();

	std::string toString () override;
	
	virtual const char* getId ();	
	
    };

    typedef ILambdaFrame* LambdaFrame;
}
