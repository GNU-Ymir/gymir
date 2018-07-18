#include <ymir/semantic/pack/MethodFrame.hh>
#include <ymir/ast/Function.hh>
#include <ymir/ast/TypeCreator.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/semantic/object/AggregateInfo.hh>
#include <ymir/ast/Par.hh>
#include <ymir/ast/Dot.hh>

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
	if (this-> _echec) return NULL;
	else if (this-> _proto) return this-> _proto;
	
	Table::instance ().enterFrame (this-> _space, this-> _name, this-> templateParams (), this-> attributes (), false);
	Table::instance ().enterBlock ();
	auto object = this-> _info-> TempOp ({});
	if (object == NULL) return NULL;
		
	auto params = params_;
	params.insert (params.begin (), new (Z0) IRefInfo (false, object));
	
	auto from = Table::instance ().globalNamespace ();
	Table::instance ().setCurrentSpace (Namespace (this-> _space, this-> _name));
	if (this-> _const != NULL) {
	    auto vars = this-> _const-> getParams ();
	    vars.insert (vars.begin (), new (Z0) IVar (this-> _const-> getIdent ()));
	    auto finalParams = IFrame::computeParams (vars, params);
	    if (object-> to <IAggregateInfo> ()-> getAncestor () != NULL)
		if (!verifCallSuper ()) {
		    Ymir::Error::mustCallSuperConstructor (this-> _const-> getIdent (), object-> to <IAggregateInfo> ()-> getAncestor ());
		    this-> _echec = true;
		    return NULL;
		}
	    
	    auto ret = IFrame::validate (this-> _name, this-> _space, finalParams, this-> _const-> getBlock (), new (Z0) IVoidInfo (), this-> isExtern ());	    
	    if (ret) {
		ret-> type ()-> type (object);
	    }
	    this-> _proto = ret;
	    return this-> _proto;	
	} else if (this-> _method) {
	    auto vars = this-> _method-> getParams ();	    
	    auto finalParams = IFrame::computeParams (vars, params);
	    
	    if (this-> _isVirtual) from = this-> _space;
	    this-> _proto = IFrame::validate (this-> _space, from, finalParams, false, this-> isExtern () && this-> _isVirtual);
	    return this-> _proto;
	} else {
	    std::vector <Var> vars = {new (Z0) IVar (this-> _dest-> getIdent ())};
	    auto finalParams = IFrame::computeParams (vars, params);
	    this-> _proto = IFrame::validate (this-> _name, this-> _space, finalParams, this-> _dest-> getBlock (), new (Z0) IVoidInfo (), this-> isExtern ());
	    return this-> _proto;
	}
    }

    ApplicationScore IMethodFrame::isApplicable (ParamList params) {
	auto object = this-> _info-> TempOp ({});
	if (object == NULL) return NULL;
	
	auto types = params-> getParamTypes ();
	std::vector <Var> vars;
	Word ident;
	if (this-> _const) {
	    vars = this-> _const-> getParams ();
	    ident = this-> _const-> getIdent ();
	} else {
	    vars = this-> _method-> getParams ();
	    vars [0] = (Var) vars [0]-> setType (new (Z0) IRefInfo (false, object));
	    
	    ident = this-> _method-> getIdent ();
	}
	
	auto ret = IFrame::isApplicable (ident, vars, types);
	if (ret)
	    ret-> ret = object;
	return ret;
    }

    ApplicationScore IMethodFrame::isApplicable (const std::vector <InfoType> & params) {
	auto object = this-> _info-> TempOp ({});
	if (object == NULL) return NULL;
	
	std::vector <Var> vars;
	Word ident;
	if (this-> _const) {	   
	    vars = this-> _const-> getParams ();
	    ident = this-> _const-> getIdent ();
	} else {
	    vars = this-> _method-> getParams ();
	    vars [0] = (Var) vars [0]-> setType (new (Z0) IRefInfo (false, object));
	    
	    ident = this-> _method-> getIdent ();
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
	if (object == NULL) return NULL;
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
		types.push_back (info-> info-> type ());
	}
       	
	this-> _proto = validate (types);
	return this-> _proto;
    }

    bool IMethodFrame::verifCallSuper () {
	auto block = this-> _const-> getBlock ();
	auto insts = block-> getInsts ();
	if (insts.size () == 0) return false;
	auto fst = insts [0];
	if (auto par = fst-> to <IPar> ()) {
	    auto left = par-> left ();
	    if (auto dcol = left-> to <IDColon> ()) {
		auto lcol = dcol-> getLeft ();
		auto rcol = dcol-> getRight ();
		if (rcol-> to <IVar> () == NULL || rcol-> token.getStr () != Keys::INIT) return false;
		if (auto dot = lcol-> to <IDot> ()) {
		    auto ldot = dot-> getLeft ();
		    auto rdot = dot-> getRight ();
		    if (ldot-> to <IVar> () == NULL || ldot-> token.getStr () != Keys::SELF) return false;
		    if (rdot-> to <IVar> () == NULL || rdot-> token.getStr () != Keys::SUPER) return false;
		    return true;
		}
	    }
	}
	
	return false;
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

