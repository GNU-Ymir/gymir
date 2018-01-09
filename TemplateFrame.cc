#include <ymir/semantic/pack/TemplateFrame.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/ast/TypedVar.hh>

namespace semantic {
    using namespace std;
    using namespace syntax;

    long ITemplateFrame::CONST_SAME_TMP = 9;
    long ITemplateFrame::SAME_TMP = 10;
    long ITemplateFrame::CONST_AFF_TMP = 4;    
    long ITemplateFrame::AFF_TMP = 5;
    long ITemplateFrame::CONST_CHANGE_TMP = 6;    
    long ITemplateFrame::CHANGE_TMP = 7;
    
    ITemplateFrame::ITemplateFrame (Namespace space, Function func) :
	IFrame (space, func)
    {
	this-> name = func-> getIdent ().getStr ();
    }

    bool& ITemplateFrame::isPure () {
	return this-> _isPure;
    }

    bool& ITemplateFrame::isExtern () {
	return this-> _isExtern;
    }

    ApplicationScore ITemplateFrame::isApplicable (Word ident, vector <Var> attrs, vector <InfoType> args)  {
	if (args.size () > this-> _function-> getParams ().size ())
	    return this-> isApplicableVariadic (ident, attrs, args);
	else return this-> isApplicableSimple (ident, attrs, args);
    }

    ApplicationScore ITemplateFrame::isApplicable (ParamList params)  {
	
    }

    ApplicationScore ITemplateFrame::isApplicable (vector <InfoType> params)  {}

    FrameProto ITemplateFrame::validate (vector <InfoType> params)  {}

    FrameProto ITemplateFrame::validate (ParamList params)  {}

    FrameProto ITemplateFrame::validate (ApplicationScore score, vector <InfoType> params)  {}

    FrameProto ITemplateFrame::validate ()  {}

    Frame ITemplateFrame::TempOp (vector <Expression> params)  {}
	
    ApplicationScore ITemplateFrame::isApplicableVariadic (Word ident, vector <Var> attrs, vector <InfoType> params) {
	Ymir::Error::assert ("TODO");
	return NULL;
    }

    ApplicationScore ITemplateFrame::getScoreSimple (Word ident, vector <Var> attrs, vector <InfoType> args) {
	auto score = new (GC) IApplicationScore (ident);
	map <string, Expression> tmps;
	
	if (attrs.size () == 0 && args.size () == 0) return NULL;
	else if (attrs.size () == args.size ()) {
	    for (auto it : Ymir::r (0, args.size ())) {
		InfoType info = NULL;
		auto param = attrs [it];
		if (auto tvar = param-> to <ITypedVar> ()) {
		    this-> changed = false;
		    TemplateSolution res = TemplateSolver::instance ().solve (this-> _function-> getTemplates (), tvar, args [it]);
		    if (!res.valid || !TemplateSolver::instance ().merge (score-> score, tmps, res))
			return NULL;

		    info = res.type;
		    if (tvar-> getDeco () == Keys::REF && !info-> is <IRefInfo> ())
			info = new (GC) IRefInfo (info);
		    if (tvar-> getDeco () == Keys::CONST) info-> isConst (true);
		    else info-> isConst (false);
		} else {
		    tvar = param-> setType (new (GC) IUndefInfo ());
		    info = tvar-> getType ()-> clone ();
		    CONST_SAME_TMP = this-> CONST_CHANGE;
		    SAME_TMP = this-> CHANGE;
		}	       
		
		auto type = args [it]-> CompOp (info);
		if (type) type = type-> ConstVerif (info);
		else return NULL;

		if (type && type-> isSame (args [it])) {
		    if (args [it]-> isConst () != info-> isConst ())
			score-> score += this-> changed ? CONST_CHANGE_TMP : CONST_SAME_TMP;
		    else score-> score += this-> changed ? CHANGE_TMP : SAME_TMP;
		    score-> treat.push_back (type);
		} else if (type != NULL) {
		    if (args [it]-> isConst () != info-> isConst ())
			score-> score += CONST_AFF_TMP;
		    else score-> score += AFF_TMP;
		    score-> treat.push_back (type);
		} else return NULL;				
	    }
	    if (!TemplateSolver::instance ().isSolved (this-> _function-> getTemplates (),
						       tmps)) return NULL;
	    for (auto exp : tmps) {
		if (exp.second-> info-> isImmutable ()) {
		    // TODO exp = exp-> info-> valye-> toYmir (exp-> info);
		}
	    }

	    score-> tmps = tmps;
	    return score;
	}
    }
    
    ApplicationScore ITemplateFrame::isApplicableSimple (Word ident, vector <Var> attrs, vector <InfoType> args) {
	auto tScope = Table::instance ().templateNamespace ();
	auto globSpace = Table::instance ().space ();
	Table::instance ().setCurrentSpace (Namespace (this-> _space, this-> name));
	Table::instance ().templateNamespace () = globSpace;
	
	auto score = getScoreSimple (ident, attrs, args);
	Table::instance ().setCurrentSpace (globSpace);
	Table::instance ().templateNamespace () = tScope;
	return score;
    }

    string ITemplateFrame::computeName (string name) {}

    FrameProto ITemplateFrame::validateExtern () {}       

}
