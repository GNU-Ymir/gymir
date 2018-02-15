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
    
    FrameProto ITemplateFrame::validate (ApplicationScore score, const vector<InfoType> & params)  {
	if (this-> _isExtern) return validateExtern ();
	else if (this-> _isPure) return validate ();
	Table::instance ().enterFrame (this-> _space, this-> name, this-> _isInternal);
	Table::instance ().enterBlock ();
	auto func = this-> _function-> templateReplace (score-> tmps);
	vector <Var> finalParams = IFrame::computeParams (func-> getParams (), params);

	auto ret = func-> getType () != NULL ? func-> getType ()-> asType ()-> info : NULL;
	if (func-> getType ()) {
	    ret-> isConst (func-> getType ()-> deco != Keys::REF && func-> getType ()-> deco != Keys::MUTABLE);
	}

	auto from = Table::instance ().globalNamespace ();
	auto proto = IFrame::validate (this-> _function-> getIdent (), this-> _space, from, ret, finalParams, func-> getBlock (), getValues (score-> tmps), this-> _isVariadic);
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
    
    ApplicationScore ITemplateFrame::isApplicableVariadic (Word, const vector<Var> & attrs, const vector<InfoType> & params) {
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

	    auto func = this-> _function-> templateReplace (res.elements);
	    Frame tmps;
	    if (!TemplateSolver::instance ().isSolved (this-> _function-> getTemplates (), res)) {
		func-> getTemplates () = TemplateSolver::instance ().unSolved (this-> _function-> getTemplates (), res);
		tmps = new (Z0) ITemplateFrame (this-> space (), func);		
	    } else tmps = new (Z0) IUnPureFrame (this-> space (), func);

	    tmps-> isVariadic (true);
	    std::vector<InfoType> types (params.begin (), params.begin () + attrs.size () - 1);
	    auto tuple = new (Z0) ITupleInfo (false);
	    tuple-> getParams () = others;
	    types.push_back (tuple);

	    auto score = tmps-> isApplicable (types);
	    if (score) {
		score-> score += res.score;
		score-> toValidate = tmps;
	    }
	    return score;
	}
	return NULL;
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
	Ymir::OutBuffer space;
	space.write ("(");

	for (auto i : Ymir::r (0, params.size ())) {
	    auto &it = params [i];
	    if (auto _val = it-> info-> value ()) {
		space.write (_val-> toString ());
	    } else
		space.write (it-> info-> type-> simpleTypeString ());
	    if (i < (int) params.size () - 1) space.write (",");
	}
	
	space.write (")");
	for (auto &it : res.elements) {
	    if (it.second-> info) {
		if (it.second-> info-> isImmutable ()) {
		    it.second = it.second-> info-> value ()-> toYmir (it.second-> info);		
		} else
		    it.second = it.second-> templateExpReplace ({});
	    }
	}
	
	auto func = this-> _function-> templateReplace (res.elements);
	func-> name ((func-> name () + space.str ()).c_str ());

	if (TemplateSolver::instance ().isSolved (this-> _function-> getTemplates (), res)) {
	    if (func-> getTest ()) {
		auto valid = func-> getTest ()-> templateExpReplace (res.elements);
		if (!validateTest (valid)) return NULL;
	    }
	    
	    Frame ret;
	    if (!this-> _isPure) ret = new (Z0)  IUnPureFrame (this-> space (), func);
	    else if (this-> _isExtern) ret = new (Z0)  IExternFrame (this-> space (), func);
	    else ret = new (Z0)  IPureFrame (this-> space (), func);

	    ret-> currentScore () = this-> currentScore () + res.score;
	    ret-> templateParams () = getValues (res.elements);
	    return ret;	    
	} else {
	    func-> getTemplates () = TemplateSolver::instance ().unSolved (this-> _function-> getTemplates (), res);
	    auto aux = new (Z0)  ITemplateFrame (this-> space (), func);
	    aux-> _currentScore = this-> currentScore () + res.score;
	    aux-> _isPure = this-> _isPure;
	    aux-> _isExtern = this-> _isExtern;
	    return aux;
	}
	
    }
    

    ApplicationScore ITemplateFrame::getScoreSimple (Word ident, const vector<Var> & attrs, const vector<InfoType> & args) {
	auto score = new (Z0)  IApplicationScore (ident);
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
		    score-> score += res.score;
		    info = res.type;
		    if (tvar-> getDeco () == Keys::CONST) info-> isConst (true);
		} else {
		    tvar = param-> setType (args [it]-> clone ());
		    info = tvar-> getType ()-> clone ();
		    CONST_SAME_TMP = this-> CONST_CHANGE;
		    SAME_TMP = this-> CHANGE;
		}

		if (param-> getDeco () == Keys::REF && !info-> is <IRefInfo> ())
		    info = new (Z0)  IRefInfo (false, info);

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
						       tmps)) {
		return NULL;
	    }
	    for (auto exp : tmps) {
		if (exp.second-> info-> isImmutable ()) {
		    exp.second = exp.second-> info-> value ()-> toYmir (exp.second-> info);
		} else {
		    exp.second = exp.second-> templateExpReplace ({});
		}
	    }
	    
	    if (this-> _function-> getTest ()) {
		auto valid = this-> _function-> getTest ()-> templateExpReplace (tmps);
		if (!validateTest (valid)) return NULL;
	    }
	    
	    score-> tmps = tmps;
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
