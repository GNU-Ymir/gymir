#include <ymir/semantic/pack/DestructFrame.hh>
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

    
    IDestructFrame::IDestructFrame (Namespace space, std::string name, InfoType type, TypeDestructor dst):
	IFrame (space, NULL),
	_info (type),
        _dest (dst),
	_name (name),
	_proto (NULL)
    {}

           
    
    FrameProto IDestructFrame::validate (const std::vector <InfoType> & params_) {
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
	std::vector <Var> vars = {new (Z0) IVar (this-> _dest-> getIdent ())};
	auto ident = this-> _dest-> getIdent ();
	auto finalParams = IFrame::computeParams (vars, params);
	if (object-> to <IAggregateInfo> ()-> getAncestor () != NULL) {
	    auto frame = object-> to <IAggregateInfo> ()-> getAncestor ()-> getDestructor ();
	    if (frame) {
		auto call = new (Z0) IPar ({ident.getLocus (), Token::LPAR}, {ident.getLocus (), Token::RPAR},					       
					   new (Z0) IDot ({ident.getLocus (), Token::DOT},
							  new (Z0) IDot ({ident.getLocus (), Token::DOT}, vars [0], new (Z0) IVar ({ident.getLocus (), Keys::SUPER}))
							  , new (Z0) IVar ({ident.getLocus (), Keys::DISPOSE})
					   ),
					   new (Z0) IParamList ({ident.getLocus(), "()"}, {})
		);
		this-> _dest-> getBlock ()-> addFinallyAtSemantic (call);
	    }
	}	    
	this-> _proto = IFrame::validate (this-> _name, this-> _space, finalParams, this-> _dest-> getBlock (), new (Z0) IVoidInfo (), this-> isExtern ());	    
	return this-> _proto;
    }
    
    ApplicationScore IDestructFrame::isApplicable (ParamList params) {
	auto object = this-> _info-> TempOp ({});
	if (object == NULL) return NULL;
	
	auto types = params-> getParamTypes ();
	std::vector <Var> vars;
	vars = {new (Z0) IVar (this-> _dest-> getIdent ())};
	vars [0] = (Var) vars [0]-> setType (new (Z0) IRefInfo (false, object));
	auto ident = this-> _dest-> getIdent ();       	

	auto ret = IFrame::isApplicable (ident, vars, types);
	if (ret)
	    ret-> ret = object;
	
	return ret;
    }

    ApplicationScore IDestructFrame::isApplicable (const std::vector <InfoType> & params) {
	auto object = this-> _info-> TempOp ({});
	if (object == NULL) return NULL;
	
	std::vector <Var> vars;
	vars = {new (Z0) IVar (this-> _dest-> getIdent ())};
	vars [0] = (Var) vars [0]-> setType (new (Z0) IRefInfo (false, object));
	auto ident = this-> _dest-> getIdent ();       	

	auto ret = IFrame::isApplicable (ident, vars, params);
	if (ret) 
	    ret-> ret = object;
	
	return ret;
    }
        
    FrameProto IDestructFrame::validate (ParamList params) {
	return this-> validate (params-> getParamTypes ());
    }

    FrameProto IDestructFrame::validate () {
	auto object = this-> _info-> TempOp ({});
	if (object == NULL) return NULL;
	std::vector <Var> vars;
	std::vector <InfoType> types;
	if (this-> _proto != NULL) return this-> _proto;
	

	this-> _proto = validate ({new (Z0) IRefInfo (false, object)});
	return this-> _proto;
    }


    InfoType& IDestructFrame::getInfo () {
	return this-> _info;
    }
           
    const char* IDestructFrame::getId () {
	return IDestructFrame::id ();
    }

    bool& IDestructFrame::isExtern () {
	return this-> _isExtern;
    }
    
    bool& IDestructFrame::needConst () {
	return this-> _needConst;
    }
}

