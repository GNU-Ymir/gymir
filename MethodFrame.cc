#include <ymir/semantic/pack/MethodFrame.hh>
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


    IMethodFrame::IMethodFrame (Namespace space, std::string name, InfoType type, TypeMethod meth):
	IFrame (space, meth),
	_info (type),
	_method (meth),
	_name (name),
	_proto (NULL)
    {}

           
    
    FrameProto IMethodFrame::validate (const std::vector <InfoType> & params_) {
	if (this-> _echec) return NULL;
	else if (this-> _proto) return this-> _proto;

	Table::instance ().enterFrame (this-> _space, this-> _name, this-> templateParams (), this-> attributes (), false);
	Table::instance ().enterBlock ();

	auto object = this-> _info-> TempOp ({});
	if (object == NULL) return NULL;
	object-> isConst (this-> _needConst);
	auto params = params_;
	params.insert (params.begin (), new (Z0) IRefInfo (false, object));
	
	auto from = Table::instance ().globalNamespace ();
	Table::instance ().setCurrentSpace (Namespace (this-> _space, this-> _name));
	auto vars = this-> _method-> getParams ();	    
	auto finalParams = IFrame::computeParams (vars, params);
	    
	if (this-> _isVirtual) from = this-> _space;
	this-> _proto = IFrame::validate (this-> _space, from, finalParams, false, this-> isExtern ());
	
	if (this-> _isVirtual && this-> _proto && this-> _proto-> type ())
	    this-> _proto-> type ()-> value () = NULL;
	return this-> _proto;
    }
    
    ApplicationScore IMethodFrame::isApplicable (ParamList params) {
	auto object = this-> _info-> TempOp ({});
	if (object == NULL) return NULL;
	
	auto types = params-> getParamTypes ();
	std::vector <Var> vars;
	vars = this-> _method-> getParams ();
	vars [0] = (Var) vars [0]-> setType (new (Z0) IRefInfo (false, object));
	
	auto ident = this-> _method-> getIdent ();	

	auto ret = IFrame::isApplicable (ident, vars, types);
	if (ret)
	    ret-> ret = object;
	
	return ret;
    }

    ApplicationScore IMethodFrame::isApplicable (const std::vector <InfoType> & params) {
	auto object = this-> _info-> TempOp ({});
	if (object == NULL) return NULL;
	
	std::vector <Var> vars;
	vars = this-> _method-> getParams ();
	vars [0] = (Var) vars [0]-> setType (new (Z0) IRefInfo (false, object));
	    
	auto ident = this-> _method-> getIdent ();
	
	auto ret = IFrame::isApplicable (ident, vars, params);
	if (ret) 
	    ret-> ret = object;
	
	return ret;
    }
        
    FrameProto IMethodFrame::validate (ParamList params) {
	return this-> validate (params-> getParamTypes ());
    }

    FrameProto IMethodFrame::validate () {
	auto object = this-> _info-> TempOp ({});
	if (object == NULL) return NULL;
	std::vector <Var> vars;
	std::vector <InfoType> types;
	if (this-> _proto != NULL) return this-> _proto;
		
	vars = this-> _method-> getParams ();
	vars = std::vector <Var> (vars.begin () + 1, vars.end ());
	types.push_back (new (Z0) IRefInfo (false, object));	


	for (auto it : Ymir::r (0, vars.size ())) {
	    auto info = vars [it]-> to <ITypedVar> ()-> getType ();
	    if (info != NULL)
		types.push_back (info);
	}	
       	
	this-> _proto = validate (types);
	return this-> _proto;
    }


    InfoType& IMethodFrame::getInfo () {
	return this-> _info;
    }
           
    const char* IMethodFrame::getId () {
	return IMethodFrame::id ();
    }

    void IMethodFrame::isExtern (bool isE) {
	this-> _isExtern = isE;
    }
    
    bool IMethodFrame::isExtern () {
	return this-> _isExtern;
    }

    syntax::TypeMethod IMethodFrame::getMethod () {
	return this-> _method;
    }
    
    bool IMethodFrame::isVirtual () {
	return this-> _isVirtual;
    }

    void IMethodFrame::isVirtual (bool isVirt) {
	this-> _isVirtual = isVirt;
    }
    
    bool& IMethodFrame::needConst () {
	return this-> _needConst;
    }

    // Frame IMethodFrame::TempOp (const std::vector <Expression> & params) {
    // 	this-> currentScore () = 0;
    // 	if (this-> _method == NULL) return NULL;
    // 	if (params.size () > this-> _method-> getTemplates ().size ()) {
    // 	    if (this-> _method-> getTemplates ().size () == 0) return NULL;
    // 	    if (!this-> _method-> getTemplates ().back ()-> is<IVariadicVar> ())
    // 		return NULL;
    // 	}

    // 	auto globSpace = Table::instance ().space ();
    // 	auto tScope = Table::instance ().templateNamespace ();
    // 	Table::instance ().setCurrentSpace (Namespace (this-> _space, this-> _method-> name ()));
    // 	Table::instance ().templateNamespace () = globSpace;

    // 	auto ret = getScoreTempOp (params);

    // 	Table::instance ().setCurrentSpace (globSpace);
    // 	Table::instance ().templateNamespace () = tScope;
    // 	return ret;
    // }

    // bool IMethodFrame::validateTest (syntax::Expression test) {
    // 	if (test) {
    // 	    auto res = test-> expression ();
    // 	    if (!res) return false;
    // 	    if (!res-> info-> isImmutable ()) {
    // 		Ymir::Error::notImmutable (test-> token, res-> info);
    // 		return false;
    // 	    } else if (!res-> info-> value ()-> is<IBoolValue> ()) {
    // 		Ymir::Error::incompatibleTypes (res-> token, res-> info, new (Z0) IBoolInfo (true));
    // 		return false;
    // 	    } else if (!res-> info-> value ()-> to <IBoolValue> ()-> isTrue ()) {
    // 		return false;
    // 	    }
    // 	}
    // 	return true;
    // }
    
    // Frame IMethodFrame::getScoreTempOp (const std::vector <Expression> & params) {
    // 	std::vector <InfoType> totals;
    // 	std::vector <Expression> finals;
    // 	std::vector <Expression> vars;

    // 	totals.resize (this-> _method-> getTemplates ().size ());
    // 	auto res = TemplateSolver::instance (). solve (this-> _method-> getTemplates (), params);

    // 	if (!res.valid) return NULL;
    // 	for (auto &it : res.elements) {
    // 	    if (it.second-> info) {
    // 		if (it.second-> info-> isImmutable ()) {
    // 		    it.second = it.second-> info-> value ()-> toYmir (it.second-> info);		
    // 		} else {
    // 		    it.second = it.second-> templateExpReplace ({});
    // 		    if (!it.second) return NULL;
    // 		}
    // 	    }
    // 	}

    // 	auto auxTmps = TemplateSolver::instance ().solved (this-> _function-> getTemplates (), res.elements);	
    // 	auto meth = this-> _method-> templateReplace (res.elements);
    // 	if (TemplateSolver::instance ().isSolved (this-> _function-> getTemplates (), res)) {
    // 	    if (meth-> getTest ()) {
    // 		auto valid = meth-> getTest ()-> templateExpReplace (res.elements);
    // 		if (!validateTest (valid)) return NULL;
    // 		else res.score += 1;
    // 	    }
	    
    // 	    auto ret = new (Z0)  IMethodFrame (this-> _space, this-> _name, this-> _info, meth);
    // 	    ret-> isVirtual () = false;
    // 	    ret-> attributes () = this-> attributes ();
	    
    // 	    ret-> currentScore () = this-> currentScore () + res.score;	    
    // 	    ret-> templateParams () = this-> templateParams ();
    // 	    ret-> templateParams ().insert (ret-> templateParams ().end (), auxTmps.begin (), auxTmps.end ());
    // 	    return ret;	    
    // 	} else {
    // 	    meth-> getTemplates () = TemplateSolver::instance ().unSolved (this-> _function-> getTemplates (), res);
    // 	    auto aux = new (Z0)  IMethodFrame (this-> _space, this-> _name, this-> _info, meth);
    // 	    aux-> isVirtual () = false;
    // 	    aux-> attributes () = this-> attributes ();
	    
    // 	    aux-> templateParams () = this-> templateParams ();
    // 	    aux-> templateParams ().insert (aux-> templateParams ().end (), auxTmps.begin (), auxTmps.end ());
    // 	    aux-> _currentScore = this-> currentScore () + res.score;
    // 	    return aux;
    // 	}	
       	
    // }
    
    
}

