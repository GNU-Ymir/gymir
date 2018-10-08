#include <ymir/semantic/pack/ConstructFrame.hh>
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

    IConstructFrame::IConstructFrame (Namespace space, std::string name, InfoType type, TypeConstructor cst):
	IFrame (space, NULL),
	_info (type),
        _const (cst),
	_name (name),
	_proto (NULL)
    {}
           
    
    FrameProto IConstructFrame::validate (const std::vector <InfoType> & params_) {
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
    }
    
    ApplicationScore IConstructFrame::isApplicable (ParamList params) {
	auto object = this-> _info-> TempOp ({});
	if (object == NULL) return NULL;
	
	auto types = params-> getParamTypes ();
	std::vector <Var> vars;
	Word ident = this-> _const-> getIdent ();
	if (this-> _isCopy) {
	    vars = {new (Z0) IVar (this-> _const-> getIdent ())};
	    vars [0] = (Var) vars [0]-> setType (new (Z0) IRefInfo (false, object));
	} else 
	    vars = this-> _const-> getParams ();

	auto ret = IFrame::isApplicable (ident, vars, types);
	if (ret)
	    ret-> ret = object;
	
	return ret;
    }

    ApplicationScore IConstructFrame::isApplicable (const std::vector <InfoType> & params) {
	auto object = this-> _info-> TempOp ({});
	if (object == NULL) return NULL;
	
	std::vector <Var> vars;
	Word ident = this-> _const-> getIdent ();
	if (this-> _isCopy) {
	    vars = {new (Z0) IVar (this-> _const-> getIdent ())};
	    vars [0] = (Var) vars [0]-> setType (new (Z0) IRefInfo (false, object));
	} else {
	    vars = this-> _const-> getParams ();
	}
	
	auto ret = IFrame::isApplicable (ident, vars, params);
	if (ret) 
	    ret-> ret = object;
	
	return ret;
    }
        
    FrameProto IConstructFrame::validate (ParamList params) {
	return this-> validate (params-> getParamTypes ());
    }

    FrameProto IConstructFrame::validate () {
	auto object = this-> _info-> TempOp ({});
	if (object == NULL) return NULL;
	std::vector <Var> vars;
	std::vector <InfoType> types;
	if (this-> _proto != NULL) return this-> _proto;
	
	vars = this-> _const-> getParams ();
	types.push_back (new (Z0) IRefInfo (false, object));
	
	if (this-> _isCopy) {
	    types.push_back (new (Z0) IRefInfo (true, object));
	} else {
	    for (auto it : Ymir::r (0, vars.size ())) {
		auto info = vars [it]-> to <ITypedVar> ()-> getType ();
		if (info != NULL)
		    types.push_back (info);
	    }
	}
       	
	this-> _proto = validate (types);
	return this-> _proto;
    }

    bool IConstructFrame::verifCallSuper () {
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

    InfoType& IConstructFrame::getInfo () {
	return this-> _info;
    }
           
    const char* IConstructFrame::getId () {
	return IConstructFrame::id ();
    }

    bool& IConstructFrame::isExtern () {
	return this-> _isExtern;
    }

    bool& IConstructFrame::isCopy () {
	return this-> _isCopy;
    }
    
    bool& IConstructFrame::needConst () {
	return this-> _needConst;
    }
        
}

