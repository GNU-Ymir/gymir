#include <ymir/semantic/pack/TemplateMethFrame.hh>
#include <ymir/ast/Function.hh>
#include <ymir/ast/TypeCreator.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/semantic/object/AggregateInfo.hh>
#include <ymir/ast/Par.hh>
#include <ymir/ast/Dot.hh>
#include <ymir/ast/TypedVar.hh>
#include <ymir/semantic/value/BoolValue.hh>

using namespace syntax;
using namespace std;

namespace semantic {


    ITemplateMethFrame::ITemplateMethFrame (Namespace space, std::string name, InfoType type, TypeMethod meth):
	ITemplateFrame (space, meth),
	_info (type),
	_method (meth),
	_name (name),
	_proto (NULL)
    {}

    FrameProto ITemplateMethFrame::validate (ApplicationScore score, const vector<InfoType> & params_)  {
	auto object = this-> _info-> TempOp ({});
	if (object == NULL) return NULL;
	object-> isConst (this-> _needConst);
	auto params = params_;
	params.insert (params.begin (), new (Z0) IRefInfo (false, object));
	
	auto tmps = this-> templateParams ();
	auto auxTmps = TemplateSolver::instance ().solved (this-> _function-> getTemplates (), score-> tmps);
	tmps.insert (tmps.end (), auxTmps.begin (), auxTmps.end ());
	Table::instance ().enterFrame (this-> _space, this-> _name, tmps, this-> attributes (), this-> _isInternal);
	Table::instance ().enterBlock ();
	auto func = this-> _function-> templateReplace (score-> tmps);
	
	vector <Var> finalParams = IFrame::computeParams (func-> getParams (), params);
	Symbol ret = NULL;
	bool lvalue = false;
	if (func-> getType ()) {
	    auto type = func-> getType ()-> toType ();
	    if (type) {
		ret = type-> info;
		if (func-> getRetDeco () == Keys::REF && !ret-> type ()-> is <IRefInfo> ()) {
		    ret-> type (new (Z0) IRefInfo (false, ret-> type ()));
		}
		lvalue = (func-> getRetDeco () == Keys::MUTABLE || func-> getRetDeco () == Keys::REF);
	    }
	}


	auto from = Table::instance ().globalNamespace ();
	Ymir::log ("Validate template method : ", this-> _function-> getIdent (), " in space : ",  Table::instance ().getCurrentSpace ());
	auto proto = IFrame::validate (this-> _function-> getIdent (), this-> _space, from, ret, finalParams, func-> getBlock (), tmps, this-> _isVariadic, this-> _function-> pre (), this-> _function-> post (), this-> _function-> postVar ());
	proto-> isLvalue () = lvalue;
	return proto;
    }
               
       
    ApplicationScore ITemplateMethFrame::isApplicable (ParamList params) {
	auto object = this-> _info-> TempOp ({});
	if (object == NULL) return NULL;
	
	auto types = params-> getParamTypes ();
	std::vector <Var> vars;
	vars = this-> _method-> getParams ();
	vars [0] = (Var) vars [0]-> setType (new (Z0) IRefInfo (false, object));
	
	auto ident = this-> _method-> getIdent ();	
	Ymir::log ("isApplicable ?", this-> _method-> getIdent ());
	auto ret = ITemplateFrame::isApplicable (ident, vars, types);
	if (ret)
	    ret-> ret = object;
	
	return ret;
    }

    ApplicationScore ITemplateMethFrame::isApplicable (const std::vector <InfoType> & params) {
	auto object = this-> _info-> TempOp ({});
	if (object == NULL) return NULL;
	
	std::vector <Var> vars;
	vars = this-> _method-> getParams ();
	vars [0] = (Var) vars [0]-> setType (new (Z0) IRefInfo (false, object));
	    
	auto ident = this-> _method-> getIdent ();
	
	auto ret = ITemplateFrame::isApplicable (ident, vars, params);
	if (ret) 
	    ret-> ret = object;
	
	return ret;
    }
        
    InfoType& ITemplateMethFrame::getInfo () {
	return this-> _info;
    }
           
    const char* ITemplateMethFrame::getId () {
	return ITemplateMethFrame::id ();
    }

    bool& ITemplateMethFrame::isExtern () {
	return this-> _isExtern;
    }

    syntax::TypeMethod ITemplateMethFrame::getMethod () {
	return this-> _method;
    }
    
    bool& ITemplateMethFrame::isVirtual () {
	return this-> _isVirtual;
    }
    
    bool& ITemplateMethFrame::needConst () {
	return this-> _needConst;
    }    

    Frame ITemplateMethFrame::getScoreTempOp (const std::vector <Expression>& params) {
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
	auto func = this-> _method-> templateReplace (res.elements);
	if (TemplateSolver::instance ().isSolved (this-> _function-> getTemplates (), res)) {
	    if (func-> getTest ()) {
		auto valid = func-> getTest ()-> templateExpReplace (res.elements);
		if (!validateTest (valid)) return NULL;
		else res.score += 1;
	    }
	    
	    MethodFrame ret = new (Z0)  IMethodFrame (this-> _space, this-> _name, this-> _info, func);
	    ret-> isVirtual (false);
	    ret-> attributes () = this-> attributes ();
	    
	    ret-> currentScore () = this-> currentScore () + res.score;	    
	    ret-> templateParams () = this-> templateParams ();
	    ret-> templateParams ().insert (ret-> templateParams ().end (), auxTmps.begin (), auxTmps.end ());
	    return ret;	    
	} else {
	    func-> templates () = TemplateSolver::instance ().unSolved (this-> _function-> getTemplates (), res);
	    auto aux = new (Z0)  ITemplateMethFrame (this-> _space, this-> _name, this-> _info, func);
	    aux-> attributes () = this-> attributes ();
	    aux-> isVirtual () = false;
	    
	    aux-> templateParams () = this-> templateParams ();
	    aux-> templateParams ().insert (aux-> templateParams ().end (), auxTmps.begin (), auxTmps.end ());
	    aux-> _currentScore = this-> currentScore () + res.score;
	    return aux;
	}	
    }


    ApplicationScore ITemplateMethFrame::getScoreVaridadic (Word ident, const vector<Var> & attrs, const vector<InfoType> & params) {
	if (attrs.size () == 0)  
	    return NULL;
	else if (auto tvar = attrs.back()-> to<ITypedVar> ()) {
	    auto last = this-> _method-> getTemplates ().back ();
	    if (!last-> is<IVariadicVar> ()) return NULL;

	    std::vector <InfoType> others (params.begin () + attrs.size () - 1, params.end ());
	    TemplateSolution res (0, true);
	    
	    if (auto var = tvar-> typeExp ()-> to<IVar> ()) 
		res = TemplateSolver::instance ().solveVariadic (this-> _method-> getTemplates (), var, others);
	    else {
		res = TemplateSolver::instance ().solveVariadic (this-> _method-> getTemplates (), tvar-> typeExp (), others);
	    }
	    if (!res.valid) return NULL;
	    Ymir::log ("Variadic soluce for : {", attrs, " | ", params, "} : ", res.elements);
	    map <string, Expression> attrTmps;
	    vector <Var> auxAttrs (attrs.begin (), attrs.end () - 1);
	    vector <InfoType> auxParams (params.begin (), params.begin () + attrs.size () - 1);
	    auto realAttrs = transformParams (res.score, auxAttrs, auxParams, attrTmps);
	    if (realAttrs.size () != auxAttrs.size ()) return NULL;

	    if (!TemplateSolver::instance ().merge (res.score, res.elements, attrTmps)) return NULL;
	    auto func = this-> _method-> templateReplace (res.elements);
	    if (!TemplateSolver::instance ().isSolved (this-> _method-> getTemplates (), res)) return NULL;
	    auto frame = new (Z0) ITemplateMethFrame (this-> _space, this-> _name, this-> _info, func);
	    
	    frame-> templateParams () = this-> templateParams ();	    
	    auto auxTmps = TemplateSolver::instance ().solved (this-> _method-> getTemplates (), res.elements);
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

    
}

