#pragma once

#include "Frame.hh"
#include "FrameProto.hh"
#include <ymir/ast/Function.hh>
#include <ymir/ast/ParamList.hh>
#include <ymir/semantic/pack/TemplateSolver.hh>

namespace semantic {

    class ITemplateFrame : public IFrame {

	std::string name;
	bool changed;
	bool _isPure = false;
	bool _isExtern = false;
	FrameProto fr;

	static long CONST_SAME_TMP ;// = 9;
	static long SAME_TMP ;// = 10;
	static long CONST_AFF_TMP ;// = 4;    
	static long AFF_TMP ;// = 5;
	static long CONST_CHANGE_TMP ;// = 6;    
	static long CHANGE_TMP ;// = 7;

	

    public:

	ITemplateFrame (Namespace space, syntax::Function func);
	
	bool& isPure ();

	bool& isExtern ();

	ApplicationScore isApplicable (Word ident, std::vector <syntax::Var> attrs, std::vector <InfoType> args) override;

	ApplicationScore isApplicable (syntax::ParamList params) override;

	ApplicationScore isApplicable (std::vector <InfoType> params) override;

	FrameProto validate (std::vector <InfoType> params) override;

	FrameProto validate (syntax::ParamList params) override;

	FrameProto validate (ApplicationScore score, std::vector <InfoType> params) override;

	FrameProto validate () override;

	Frame TempOp (std::vector <syntax::Expression> params) override;
	
    private : 

	ApplicationScore isApplicableVariadic (Word ident, std::vector <syntax::Var> attrs, std::vector <InfoType> params);	

	ApplicationScore isApplicableSimple (Word ident, std::vector <syntax::Var> attrs, std::vector <InfoType> args);

	ApplicationScore getScoreSimple (Word ident, std::vector <syntax::Var> attrs, std::vector <InfoType> args);

	std::string computeName (std::string name);

	FrameProto validateExtern ();
	
    };
    
}
