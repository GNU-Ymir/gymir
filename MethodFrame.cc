#include <ymir/semantic/pack/MethodFrame.hh>
#include <ymir/ast/Function.hh>
#include <ymir/ast/TypeCreator.hh>

using namespace syntax;

namespace semantic {

    IMethodFrame::IMethodFrame (Namespace space, std::string name, InfoType type, TypeConstructor cst):
	IFrame (space, NULL),
	_info (type),
        _const (cst),
	_name (name),
	_proto (NULL)
    {}


    IMethodFrame::IMethodFrame (Namespace space, std::string name, InfoType type, TypeDestructor dst):
	IFrame (space, NULL),
	_info (type),
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
	} else {
	    std::vector <Var> vars = {new (Z0) IVar (this-> _dest-> getIdent ())};
	    auto finalParams = IFrame::computeParams (vars, params);
	    return IFrame::validate (this-> _name, this-> _space, finalParams, this-> _dest-> getBlock (), new (Z0) IVoidInfo ());
	}
    }

    ApplicationScore IMethodFrame::isApplicable (ParamList params) {
	auto object = this-> _info-> TempOp ({});
	auto types = params-> getParamTypes ();
	// types.insert (types.begin (), new (Z0) IRefInfo (false, object));
	
	auto vars = this-> _const-> getParams ();
	// vars.insert (vars.begin (), new (Z0) IVar (this-> _const-> getIdent ()));
	auto ret = IFrame::isApplicable (this-> _const-> getIdent (), vars, types);
	if (ret)
	    ret-> ret = object;
	return ret;
    }

    ApplicationScore IMethodFrame::isApplicable (const std::vector <InfoType> & params) {
	//auto params = params_;
	auto object = this-> _info-> TempOp ({});
	// params.insert (params.begin (), new (Z0) IRefInfo (false, object));
	
	auto vars = this-> _const-> getParams ();
	// vars.insert (vars.begin (), new (Z0) IVar (this-> _const-> getIdent ()));
	auto ret = IFrame::isApplicable (this-> _const-> getIdent (), vars, params);
	if (ret) 
	    ret-> ret = object;
	
	return ret;
    }

    FrameProto IMethodFrame::validate (ParamList params) {
	return this-> validate (params-> getParamTypes ());
    }

    FrameProto IMethodFrame::validate () {
	auto object = this-> _info-> TempOp ({});
	if (this-> _const) {
	    auto vars = this-> _const-> getParams ();
	
	    std::vector <InfoType> types;
	    types.push_back (new (Z0) IRefInfo (false, object));
	    for (auto it : Ymir::r (0, vars.size ())) {
		auto info = vars [it]-> var ();
		if (info != NULL)
		    types.push_back (info-> info-> type);
	    }
       	
	    return validate (types);
	} else {
	    if (this-> proto == NULL) 
		this-> proto = validate ({new (Z0) IRefInfo (false, object)});
	    return this-> proto;
	}
    }
    
    const char* IMethodFrame::getId () {
	return IMethodFrame::id ();
    }
}
