#include <ymir/semantic/pack/TemplateFrame.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/ast/TypedVar.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/semantic/value/_.hh>
#include <ymir/semantic/value/BoolValue.hh>

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

    bool ITemplateFrame::isPure () {
	return this-> _isPure;
    }

    void ITemplateFrame::isPure (bool isPure) {
	this-> _isPure = isPure;
    }
    
    bool& ITemplateFrame::isExtern () {
	return this-> _isExtern;
    }

    ApplicationScore ITemplateFrame::isApplicable (Word ident, const vector<Var> & attrs, const vector <InfoType> &args)  {
	if (args.size () > this-> _function-> getParams ().size ())
	    return this-> isApplicableVariadic (ident, attrs, args);
	else return this-> isApplicableSimple (ident, attrs, args);
    }

    ApplicationScore ITemplateFrame::isApplicable (ParamList params)  {
	return this-> isApplicable (this-> _function-> getIdent (), this-> _function-> getParams (), params-> getParamTypes ());
    }

    ApplicationScore ITemplateFrame::isApplicable (const vector<InfoType> & params)  {
	return this-> isApplicable (this-> _function-> getIdent (), this-> _function-> getParams (), params);
    }

    FrameProto ITemplateFrame::validate (const vector<InfoType> &)  {
	return NULL;
    }

    FrameProto ITemplateFrame::validate (ParamList)  {
	return NULL;
    }

    template <typename K, typename V>
    vector <V> getValues (map <K, V> dico) {
	vector <V> vec;
	for (auto it : dico)
	    vec.push_back (it.second);
	return vec;
    }

    Namespace ITemplateFrame::computeSpace (const map<string, Expression> &tmps) {
	if (tmps.size () != 0) {
	    Ymir::OutBuffer name;
	    for (auto it = tmps.begin (); it != tmps.end (); ) {
		if (auto params = it-> second-> to <IParamList> ()) {
		    name.write ("{");
		    for (auto it_ : Ymir::r (0, params-> getParamTypes ().size ())) {
			name.write (params-> getParamTypes () [it_]-> simpleTypeString ());
			if (it_ < (int) params-> getParamTypes ().size () - 1)
			    name.write (",");
		    }
		    name.write ("}");
		} else {
		    if (it-> second-> info-> isImmutable ())
			name.write (it-> second-> info-> value ()-> toString ());
		    else
			name.write (it-> second-> info-> simpleTypeString ());
		}
		++it;
	    }
	    return Namespace (this-> name + name.str ());
	}
	return this-> name;
    }
    
    FrameProto ITemplateFrame::validate (ApplicationScore score, const vector<InfoType> & params)  {
	if (this-> _isExtern) return validateExtern ();
	else if (this-> _isPure) return validate ();

	auto tmps = this-> templateParams ();
	auto auxTmps = TemplateSolver::instance ().solved (this-> _function-> getTemplates (), score-> tmps);
	tmps.insert (tmps.end (), auxTmps.begin (), auxTmps.end ());
	Table::instance ().enterFrame (this-> _space, this-> name, tmps, this-> attributes (), this-> _isInternal);
	Table::instance ().enterBlock ();
	auto func = this-> _function-> templateReplace (score-> tmps);
	
	vector <Var> finalParams = IFrame::computeParams (func-> getParams (), params);
	Symbol ret = NULL;
	bool lvalue = false;
	if (func-> getType ()) {
	    auto type = func-> getType ()-> asType ();
	    if (type) {
		ret = type-> info;
		lvalue = func-> getType ()-> deco == Keys::MUTABLE;
	    }
	}


	auto from = Table::instance ().globalNamespace ();
	auto proto = IFrame::validate (this-> _function-> getIdent (), this-> _space, from, ret, finalParams, func-> getBlock (), tmps, this-> _isVariadic, this-> _function-> pre (), this-> _function-> post (), this-> _function-> postVar ());
	proto-> isLvalue () = lvalue;
	return proto;
    }

    FrameProto ITemplateFrame::validate ()  {
	Ymir::Error::assert ("TODO");
	return NULL;
    }

    Frame ITemplateFrame::TempOp (const vector<Expression> & params)  {
	this-> currentScore () = 0;
	if (params.size () > this-> _function-> getTemplates ().size ()) {
	    if (!this-> _function-> getTemplates ().back ()-> is<IVariadicVar> ())
		return NULL;
	}

	auto globSpace = Table::instance ().space ();
	auto tScope = Table::instance ().templateNamespace ();
	Table::instance ().setCurrentSpace (Namespace (this-> space (), this-> _function-> name ()));
	Table::instance ().templateNamespace () = globSpace;
	
	auto ret = getScoreTempOp (params);

	Table::instance ().setCurrentSpace (globSpace);
	Table::instance ().templateNamespace () = tScope;	
	return ret;
    }
    
    ApplicationScore ITemplateFrame::isApplicableVariadic (Word ident, const vector<Var> & attrs, const vector<InfoType> & args) {
	auto tScope = Table::instance ().templateNamespace ();
	auto globSpace = Table::instance ().space ();
	Table::instance ().setCurrentSpace (Namespace (this-> _space, this-> name));
	Table::instance ().templateNamespace () = globSpace;
	
	auto score = getScoreVaridadic (ident, attrs, args);
	
	Table::instance ().setCurrentSpace (globSpace);
	Table::instance ().templateNamespace () = tScope;
	return score;
    }

    bool ITemplateFrame::validateTest (Expression test) {
	if (test) {
	    auto res = test-> expression ();
	    if (!res) return false;
	    if (!res-> info-> isImmutable ()) {
		Ymir::Error::notImmutable (test-> token, res-> info);
		return false;
	    } else if (!res-> info-> value ()-> is<IBoolValue> ()) {
		Ymir::Error::incompatibleTypes (res-> token, res-> info, new (Z0) IBoolInfo (true));
		return false;
	    } else if (!res-> info-> value ()-> to <IBoolValue> ()-> isTrue ()) {
		return false;
	    }
	}
	return true;
    }
    
    
    Frame ITemplateFrame::getScoreTempOp (const std::vector <Expression>& params) {
	std::vector <InfoType> totals;
	std::vector <Expression> finals;
	std::vector <Expression> vars;

	totals.resize (this-> _function-> getTemplates ().size ());
	auto res = TemplateSolver::instance (). solve (this-> _function-> getTemplates (), params);

	if (!res.valid) return NULL;
	for (auto &it : res.elements) {
	    if (it.second-> info) {
		if (it.second-> info-> isImmutable ()) {
		    it.second = it.second-> info-> value ()-> toYmir (it.second-> info);		
		} else {
		    it.second = it.second-> templateExpReplace ({});
		    if (!it.second) return NULL;
		}
	    }
	}
		
	auto auxTmps = TemplateSolver::instance ().solved (this-> _function-> getTemplates (), res.elements);	
	auto func = this-> _function-> templateReplace (res.elements);
	if (TemplateSolver::instance ().isSolved (this-> _function-> getTemplates (), res)) {
	    if (func-> getTest ()) {
		auto valid = func-> getTest ()-> templateExpReplace (res.elements);
		if (!validateTest (valid)) return NULL;
	    }
	    
	    Frame ret;
	    if (!this-> _isPure) ret = new (Z0)  IUnPureFrame (this-> _space, func);
	    else if (this-> _isExtern) ret = new (Z0)  IExternFrame (this-> _space, func);
	    else ret = new (Z0)  IPureFrame (this-> _space, func);
	    ret-> attributes () = this-> attributes ();
	    
	    ret-> currentScore () = this-> currentScore () + res.score;	    
	    ret-> templateParams () = this-> templateParams ();
	    ret-> templateParams ().insert (ret-> templateParams ().end (), auxTmps.begin (), auxTmps.end ());
	    return ret;	    
	} else {
	    func-> getTemplates () = TemplateSolver::instance ().unSolved (this-> _function-> getTemplates (), res);
	    auto aux = new (Z0)  ITemplateFrame (this-> _space, func);
	    aux-> attributes () = this-> attributes ();
	    
	    aux-> templateParams () = this-> templateParams ();
	    aux-> templateParams ().insert (aux-> templateParams ().end (), auxTmps.begin (), auxTmps.end ());
	    aux-> _currentScore = this-> currentScore () + res.score;
	    aux-> _isPure = this-> _isPure;
	    aux-> _isExtern = this-> _isExtern;
	    return aux;
	}	
    }

    std::vector <Expression> replace (std::vector <Expression> exprs, const std::map <std::string, Expression> & tmps) {
	std::vector <Expression> re (exprs.size ());
	for (auto exp : tmps) {
	    if (exp.second-> info) {
		if (exp.second-> info-> isImmutable ()) {
		    exp.second = exp.second-> info-> value ()-> toYmir (exp.second-> info);
		} else {
		    exp.second = exp.second-> templateExpReplace ({});
		    if (exp.second == NULL) return {};
		}
	    }
	}

	for (auto it : Ymir::r (0, exprs.size ())) {
	    re [it] = exprs [it]-> templateExpReplace (tmps);
	}
	return re;
    }

    vector <Var> ITemplateFrame::transformParams (long & score, const vector <Var> & attrs, const vector <InfoType> & args, map <string, Expression>& tmps) {
	auto templates = this-> _function-> getTemplates ();
	for (auto it : Ymir::r (0, args.size ())) {
	    auto param = attrs [it];
	    if (auto tvar = param-> to <ITypedVar> ()) {
		TemplateSolution res = TemplateSolver::instance ().solve (templates, tvar, args [it]);
		Ymir::log ("Soluce for : (", templates, ") (", tvar-> prettyPrint (), ") with (", args [it], ") : ", res.toString ());
		if (!res.valid || !TemplateSolver::instance ().merge (score, tmps, res))
		    return {};
		score += res.score;		
	    }
	}

	Ymir::log ("Total solve : ", tmps);	
	for (auto exp : tmps) {
	    if (exp.second-> info) {
		if (exp.second-> info-> isImmutable ()) {
		    exp.second = exp.second-> info-> value ()-> toYmir (exp.second-> info);
		} else {
		    exp.second = exp.second-> templateExpReplace ({});
		}
	    }
	}
	
	if (this-> _function-> getTest ()) {
	    auto valid = this-> _function-> getTest ()-> templateExpReplace (tmps);
	    if (!validateTest (valid)) return {};
	    else score += 1;//__TEST__;
	}
	std::vector <Var> finals (attrs.size ());
	for (auto it : Ymir::r (0, finals.size ()))
	    finals [it] = attrs [it]-> templateExpReplace (tmps)-> to <IVar> ();
	
	return finals;
    }

    // tvar = tvar-> templateExpReplace (tmps)-> to <ITypedVar> ();
    // this-> changed = false;
    // TemplateSolution res = TemplateSolver::instance ().solve (templates, tvar, args [it]);
    // if (!res.valid || !TemplateSolver::instance ().merge (score-> score, tmps, res))
    // 	return NULL;
		    
    // score-> score += res.score;
    // info = res.type;
    // if (tvar-> getDeco () == Keys::CONST) info = info-> cloneConst ();
    
    ApplicationScore ITemplateFrame::getScoreSimple (Word ident, const vector<Var> & attrs, const vector<InfoType> & args, bool transform) {
	auto score = new (Z0)  IApplicationScore (ident);
	map <string, Expression> tmps;
	auto templates = this-> _function-> getTemplates ();
	if (attrs.size () == 0 && args.size () == 0) return NULL;
	else if (attrs.size () == args.size ()) {
	    std::vector <Var> realAttrs;
	    if (transform) {
		Ymir::log ("Template solving for ", ident, " start");
		realAttrs = transformParams (score-> score, attrs, args, tmps);
		Ymir::log ("Template solving for ", ident, " end");
		if (!TemplateSolver::instance ().isSolved (this-> _function-> getTemplates (), tmps))
		    return NULL;
		if (realAttrs.size () != attrs.size ()) return NULL;
	    } else realAttrs = attrs;
	    
	    for (auto it : Ymir::r (0, args.size ())) {
		InfoType info = NULL;
		auto param = realAttrs [it];
		if (auto tvar = param-> to <ITypedVar> ()) {
		    auto getType = tvar-> getType ();
		    if (getType == NULL) return NULL;
		    info = getType-> clone ();
		} else {
		    tvar = param-> setType (args [it]-> clone ());
		    info = tvar-> getType ()-> clone ();
		    CONST_SAME_TMP = this-> CONST_CHANGE;
		    SAME_TMP = this-> CHANGE;
		}

		auto type = args [it]-> CompOp (info);
		if (type) type = type-> ConstVerif (info);
		else return NULL;

		if (type && type-> isSame (args [it]-> getIntern ())) {
		    if (args [it]-> passingConst (info))
			score-> score += this-> changed ? CONST_CHANGE_TMP : CONST_SAME_TMP;
		    else score-> score += this-> changed ? CHANGE_TMP : SAME_TMP;
		    score-> treat.push_back (type);
		} else if (type != NULL) {
		    bool change = false;
		    if (auto ref = args [it]-> to<IRefInfo> ()) {
			if (ref-> content ()-> isSame (type)) 
			    changed = true;			   			
		    }
		    
		    if (args [it]-> passingConst (info))
			score-> score += change ? CONST_CHANGE_TMP : CONST_AFF_TMP;
		    else score-> score += change ? CHANGE_TMP : AFF_TMP;
		    score-> treat.push_back (type);
		} else return NULL;				
	    }
	    
	    score-> tmps = tmps;
	    return score;
	}
	return NULL;
    }
    
    ApplicationScore ITemplateFrame::getScoreVaridadic (Word ident, const vector<Var> & attrs, const vector<InfoType> & params) {
	if (attrs.size () == 0)  
	    return NULL;
	else if (auto tvar = attrs.back()-> to<ITypedVar> ()) {
	    auto last = this-> _function-> getTemplates ().back ();
	    if (!last-> is<IVariadicVar> ()) return NULL;

	    std::vector <InfoType> others (params.begin () + attrs.size () - 1, params.end ());
	    TemplateSolution res (0, true);
	    if (tvar-> typeVar ()) 
		res = TemplateSolver::instance ().solveVariadic (this-> _function-> getTemplates (), tvar-> typeVar (), others);
	    else {
		res = TemplateSolver::instance ().solveVariadic (this-> _function-> getTemplates (), tvar-> typeExp (), others);
	    }
	    if (!res.valid) return NULL;
	    map <string, Expression> attrTmps;
	    vector <Var> auxAttrs (attrs.begin (), attrs.end () - 1);
	    vector <InfoType> auxParams (params.begin (), params.begin () + attrs.size () - 1);
	    auto realAttrs = transformParams (res.score, auxAttrs, auxParams, attrTmps);
	    if (realAttrs.size () != auxAttrs.size ()) return NULL;

	    if (!TemplateSolver::instance ().merge (res.score, res.elements, attrTmps)) return NULL;
	    auto func = this-> _function-> templateReplace (res.elements);
	    if (!TemplateSolver::instance ().isSolved (this-> _function-> getTemplates (), res)) return NULL;
	    auto frame = new (Z0) ITemplateFrame (this-> _space, func);
	    
	    frame-> templateParams () = this-> templateParams ();	    
	    auto auxTmps = TemplateSolver::instance ().solved (this-> _function-> getTemplates (), res.elements);
	    frame-> templateParams ().insert (frame-> templateParams ().end (), auxTmps.begin (), auxTmps.end ());
	    frame-> isVariadic (true);
	    std::vector<InfoType> types (params.begin (), params.begin () + attrs.size () - 1);
	    	    
	    auto tuple = new (Z0) ITupleInfo (false);
	    tuple-> setFake ();
	    tuple-> getParams () = others;	    
	    types.push_back (tuple);
	    auto score = frame-> getScoreSimple (ident, func-> getParams (), types, false);
	    if (score) {
		score-> score += res.score;
		score-> toValidate = frame;
	    }
	    
	    return score;	
	}
	return NULL;
    }

    ApplicationScore ITemplateFrame::isApplicableSimple (Word ident, const vector<Var> & attrs, const vector <InfoType> &args) {
	auto tScope = Table::instance ().templateNamespace ();
	auto globSpace = Table::instance ().space ();
	Table::instance ().setCurrentSpace (Namespace (this-> _space, this-> name));
	Table::instance ().templateNamespace () = globSpace;
	
	auto score = getScoreSimple (ident, attrs, args);
	
	Table::instance ().setCurrentSpace (globSpace);
	Table::instance ().templateNamespace () = tScope;
	return score;
    }

    string ITemplateFrame::computeName (string name) {
	Ymir::OutBuffer buf;
	buf.write (name, "(");
	int i = 0;
	for (auto it : this-> _function-> getTemplates ()) {
	    if (auto _val = it-> expression ()-> info-> value ())
		buf.write (_val-> toString ());
	    else buf.write (it-> info-> typeString ());
	    if (i < (int) this-> _function-> getTemplates ().size () - 1)
		buf.write (", ");
	    i = i + 1;
	}
	buf.write (")");
	return buf.str ();
    }

    FrameProto ITemplateFrame::validateExtern () {
	Ymir::Error::assert ("TODO");
	return NULL;
    }       

    const char * ITemplateFrame::getId () {
	return "ITemplateFrame";
    }
    
}
