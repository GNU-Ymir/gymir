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
	
	bool isPure () override;

	void isPure (bool);
	
	bool& isExtern ();

	bool isWeak () const override;
	
	virtual ApplicationScore isApplicable (Word ident, const std::vector<syntax::Var> & attrs, const std::vector<InfoType> & args) override;

	virtual ApplicationScore isApplicable (syntax::ParamList params) override;

	virtual ApplicationScore isApplicable (const std::vector<InfoType> & params) override;

	virtual FrameProto validate (const std::vector<InfoType> & params) override;

	virtual FrameProto validate (syntax::ParamList params) override;

	virtual FrameProto validate (ApplicationScore score, const std::vector<InfoType> & params) override;

	virtual FrameProto validate () override;

	Frame TempOp (const std::vector<syntax::Expression> & params) override;

	const char * getId () override;
	
    protected : 

	ApplicationScore isApplicableVariadic (Word ident, const std::vector<syntax::Var> & attrs, const std::vector<InfoType> & params);	

	ApplicationScore isApplicableSimple (Word ident, const std::vector<syntax::Var> & attrs, const std::vector<InfoType> & args);

	ApplicationScore getScoreSimple (Word ident, const std::vector<syntax::Var> & attrs, const std::vector<InfoType> & args, bool transform = true);

	virtual ApplicationScore getScoreVaridadic (Word ident, const std::vector<syntax::Var> & attrs, const std::vector<InfoType> & args);

	virtual Frame getScoreTempOp (const std::vector <syntax::Expression> & params);
	
	std::string computeName (std::string name);

	FrameProto validateExtern ();

	std::string getName () override;
	
	bool validateTest (syntax::Expression test);
	
	std::vector <syntax::Var> transformParams (long & score,
						   const std::vector<syntax::Var> & attrs,
						   const std::vector<InfoType> & args,
						   std::map <std::string, syntax::Expression> & tmps);

	Namespace computeSpace (const std::map <std::string, syntax::Expression> & tmps);
	
    };
    
}
