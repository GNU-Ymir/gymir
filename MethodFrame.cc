#include <ymir/semantic/pack/MethodFrame.hh>
#include <ymir/ast/Function.hh>
#include <ymir/ast/TypeCreator.hh>

using namespace syntax;

namespace semantic {

    IMethodFrame::IMethodFrame (Namespace space, std::string name, InfoType type, TypeConstructor cst):
	IFrame (space, NULL),
	_info (type),
        _const (cst),
	_method (NULL),
	_dest (NULL),
	_name (name),
	_proto (NULL)
    {}


    IMethodFrame::IMethodFrame (Namespace space, std::string name, InfoType type, TypeMethod meth):
	IFrame (space, meth),
	_info (type),
	_const (NULL),
	_method (meth),
        _dest (NULL),
	_name (name),
	_proto (NULL)
    {}

    
    IMethodFrame::IMethodFrame (Namespace space, std::string name, InfoType type, TypeDestructor dst):
	IFrame (space, NULL),
	_info (type),
	_const (NULL),
	_method (NULL),
        _dest (dst),
	_name (name),
	_proto (NULL)
    {}

           
    
    FrameProto IMethodFrame::validate (const std::vector <InfoType> & params_) {
	Table::instance ().enterFrame (this-> _space, this-> _name, this-> templateParams (), this-> attributes (), false);
	auto object = this-> _info-> TempOp ({});
	auto params = params_;
	params.insert (params.begin (), new (Z0) IRefInfo (false, object));

	auto from = Table::instance ().globalNamespace ();
	Table::instance ().setCurrentSpace (Namespace (this-> _space, this-> _name));

	if (this-> _const != NULL) {
	    auto vars = this-> _const-> getParams ();
	    vars.insert (vars.begin (), new (Z0) IVar (this-> _const-> getIdent ()));
	    auto finalParams = IFrame::computeParams (vars, params);
	    auto ret = IFrame::validate (this-> _name, this-> _space, finalParams, this-> _const-> getBlock (), new (Z0) IVoidInfo ());
	    if (ret) {
		ret-> type ()-> type = object;
	    }
	    return ret;
	} else if (this-> _method) {
	    auto vars = this-> _method-> getParams ();
	    auto finalParams = IFrame::computeParams (vars, params);
	    
	    return IFrame::validate (this-> _space, from, finalParams, false);
	} else {
	    std::vector <Var> vars = {new (Z0) IVar (this-> _dest-> getIdent ())};
	    auto finalParams = IFrame::computeParams (vars, params);
	    return IFrame::validate (this-> _name, this-> _space, finalParams, this-> _dest-> getBlock (), new (Z0) IVoidInfo ());
	}
    }

    ApplicationScore IMethodFrame::isApplicable (ParamList params) {
	auto object = this-> _info-> TempOp ({});
	auto types = params-> getParamTypes ();
	std::vector <Var> vars;
	Word ident;
	if (this-> _const) {
	    vars = this-> _const-> getParams ();
	    ident = this-> _const-> getIdent ();
	} else {
	    vars = this-> _method-> getParams ();
	    ident = this-> _method-> getIdent ();
	}
	
	auto ret = IFrame::isApplicable (ident, vars, types);
	if (ret)
	    ret-> ret = object;
	return ret;
    }

    ApplicationScore IMethodFrame::isApplicable (const std::vector <InfoType> & params) {
	auto object = this-> _info-> TempOp ({});
	std::vector <Var> vars;
	Word ident;
	if (this-> _const) {	   
	    vars = this-> _const-> getParams ();
	    ident = this-> _const-> getIdent ();
	} else {
	    vars = this-> _method-> getParams ();
	    ident = this-> _method->getIdent ();
	}
	
	
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
	std::vector <Var> vars;
	std::vector <InfoType> types;
	if (this-> _proto != NULL) return this-> _proto;	
	if (this-> _const) {
	    vars = this-> _const-> getParams ();
	    types.push_back (new (Z0) IRefInfo (false, object));
	} else if (this-> _method) {
	    vars = this-> _method-> getParams ();
	    vars = std::vector <Var> (vars.begin () + 1, vars.end ());
	    types.push_back (new (Z0) IRefInfo (false, object));
	} else {
	    this-> _proto = validate ({new (Z0) IRefInfo (false, object)});
	    return this-> _proto;
	}
		
	for (auto it : Ymir::r (0, vars.size ())) {
	    auto info = vars [it]-> var ();
	    if (info != NULL)
		types.push_back (info-> info-> type);
	}
       	
	this-> _proto = validate (types);
	return this-> _proto;
    }
    
    const char* IMethodFrame::getId () {
	return IMethodFrame::id ();
    }

    bool& IMethodFrame::isExtern () {
	return this-> _isExtern;
    }

    bool& IMethodFrame::isVirtual () {
	return this-> _isVirtual;
    }

}

