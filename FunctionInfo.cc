#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/pack/Frame.hh>
#include <ymir/utils/Mangler.hh>
#include <ymir/semantic/pack/PureFrame.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/semantic/value/Value.hh>
#include <ymir/semantic/value/LambdaValue.hh>

namespace semantic {

    int IFunctionInfo::nbTmpsCreation = 0;
    bool IFunctionInfo::needToReset = true;
    
    namespace FunctionUtils {
	using namespace syntax;
	using namespace std;
	using namespace Ymir;

	Tree InstAffect (Word, InfoType type, Expression, Expression) {
	    PtrFuncInfo func = (PtrFuncInfo) type;
	    tree ret = void_type_node;
	    tree fndecl_type = build_function_type_array (
		ret, 0, NULL
	    );
	    std::string name = Mangler::mangle_function (
		func-> getScore ()-> proto-> name (),
		func-> getScore ()-> proto
	    );
	    
	    tree fndecl = build_fn_decl (name.c_str (), fndecl_type);
	    return build1 (ADDR_EXPR, build_pointer_type (fndecl_type), fndecl);
	}       

    }
    
    IFunctionInfo::IFunctionInfo (Namespace space, std::string name) :
	IInfoType (true),
	_name (name),
	_space (space),
	_info (NULL),
	_alone (false)
    {}

    IFunctionInfo::IFunctionInfo (Namespace space, std::string name, const std::vector<Frame> & infos) :
	IInfoType (true),
	_name (name),
	_space (space),
	_fromTemplates (infos),
	_alone (false)
    {}

    bool IFunctionInfo::isSame (InfoType other) {
	if (auto ot = other-> to<IFunctionInfo> ()) {
	    if (ot-> _space == this-> _space && this-> _name == ot-> _name) return true;
	    if (ot-> value () == this-> value ()) return true;	    
	}
	return false;
    }

    void IFunctionInfo::set (Frame fr) {
	this-> _info = fr;
    }

    Frame IFunctionInfo::frame () {
	return this-> _info;
    }

    Namespace IFunctionInfo::space () {
	return this-> _space;
    }

    InfoType IFunctionInfo::onClone () {
	auto ret = new (Z0) IFunctionInfo (this-> _space, this-> _name);
	ret-> _info = this-> _info;
	ret-> _alone = this-> _alone;
	ret-> _fromTemplates = this-> _fromTemplates;
	ret-> value () = this-> value ();
	return ret;
    }
    
    std::vector <Frame> IFunctionInfo::getFrames () {	
	if (this-> _alone) return {this-> _info};
	if (this-> _fromTemplates.size () != 0) return this-> _fromTemplates;
	std::vector <Frame> alls;
	std::vector <Symbol> others;
	if (this-> _onlyInMe) others = this-> _onlyInMe-> getAllFor (this-> _name, Table::instance ().space ()); 
	else others = Table::instance ().getAll (this-> _name);
	
	bool adds = false;
	for (auto it : others) {
	    if (auto fun = it-> type-> to<IFunctionInfo> ()) {
		if (!fun-> _alone) {
		    alls.push_back (fun-> _info);
		    if (fun-> _info == this-> _info) adds = true;
		}		
	    }
	}
	if (!adds) alls.push_back (this-> _info);	
	return alls;
    }

    InfoType IFunctionInfo::BinaryOpRight (Word op, syntax::Expression left) {
	if (op == Token::EQUAL && left-> info-> type-> is <IUndefInfo> ()) {
	    auto frames = getFrames ();
	    if (frames.size () == 1) {//&& frames [0]-> isPure ()) {
		auto infoTypes = frames [0]-> getParamTypes ();
		auto score = this-> CallOp ({op.getLocus (), ""}, infoTypes);
		if (score == NULL || score-> ret == NULL) return NULL;
		auto ret = new (Z0) IPtrFuncInfo (true);
		ret-> getParams () = infoTypes;
		ret-> getType () = score-> ret-> cloneConst ();
		ret-> getScore () = score;
		
		ret-> nextBinop.push_back (&FunctionUtils::InstAffect);
		ret-> binopFoo = &PtrFuncUtils::InstAffectComp;
		
		return ret;	
	    }
	}
	return NULL;
    }

    ApplicationScore IFunctionInfo::verifErrors () {
	if (itsUpToMe) {
	    itsUpToMe = false;
	    needToReset = true;
	    nbTmpsCreation = 0;
	}
	
	return NULL;
    }
    
    ApplicationScore IFunctionInfo::CallOp (Word tok, const std::vector<InfoType> & params) {
	if (needToReset) {
	    itsUpToMe = true;
	    needToReset = false;
	}
	
	ulong nbErrorBeg = Ymir::Error::nb_errors;
	std::vector <ApplicationScore> total;
	std::vector <Frame> frames = getFrames ();

	for (auto it : frames)
	    total.push_back (it-> isApplicable (params));

	std::vector <Frame> goods;
	ApplicationScore right = new (Z0)  IApplicationScore ();
	for (uint it = 0; it < total.size () ; it++) {
	    if (total [it]) {
		if (goods.size () == 0 && total [it]-> score != 0) {
		    right = total [it];
		    goods.push_back (frames [it]);
		} else if (right-> score < total [it]-> score) {
		    goods.clear ();
		    goods.push_back (frames [it]);
		    right = total [it];
		} else if (right-> score == total [it]-> score && total [it]-> score != 0) {
		    goods.push_back (frames [it]);
		}
	    }
	}

	if (goods.size () == 0) return NULL;
	else if (goods.size () != 1) {
	    Ymir::Error::templateSpecialisation (goods [0]-> ident (),
						 goods [1]-> ident ()
	    );
	    return verifErrors ();
	}

	Table::instance ().addCall (tok);
	FrameProto info;
	if (right-> toValidate) {
	    info = right-> toValidate-> validate (right, right-> treat);
	    if (info != NULL)
		right-> name = Mangler::mangle_functionv (info-> name (), info);
	    right-> proto = info;
	} else {
	    info = goods [0]-> validate (right, right-> treat);
	    if (info != NULL)
		right-> name = Mangler::mangle_function (info-> name (), info);
	    right-> proto = info;
	}

	if (Ymir::Error::nb_errors - nbErrorBeg && !tok.isEof ()) {
	    nbTmpsCreation ++;
	    if (nbTmpsCreation < 4) {
		Ymir::Error::templateCreation (tok);
	    } else if (this-> itsUpToMe) {
		Ymir::Error::templateCreation2 (tok);
	    }
	    right-> ret = NULL;
	    verifErrors ();
	    return right;
	}

	if (info == NULL) return NULL;
	right-> ret = info-> type ()-> type-> clone ();
	right-> ret-> value () = info-> type ()-> value ();

	return right;
    }

    ApplicationScore IFunctionInfo::CallOp (Word tok, syntax::ParamList params) {
	if (needToReset) {
	    itsUpToMe = true;
	    needToReset = false;
	}
	
	ulong nbErrorBeg = Ymir::Error::nb_errors;
	std::vector <ApplicationScore> total;
	std::vector <Frame> frames = getFrames ();

	for (auto it : frames) {
	    total.push_back (it-> isApplicable (params));	    
	}

	std::vector <Frame> goods;
	ApplicationScore right = new (Z0)  IApplicationScore ();
	for (uint it = 0; it < total.size () ; it++) {
	    if (total [it]) {
		if (goods.size () == 0 && total [it]-> score != 0) {
		    right = total [it];
		    goods.push_back (frames [it]);
		} else if (right-> score < total [it]-> score) {
		    goods.clear ();
		    goods.push_back (frames [it]);
		    right = total [it];
		} else if (right-> score == total [it]-> score && total [it]-> score != 0) {
		    goods.push_back (frames [it]);
		}
	    }
	}
	
	if (goods.size () == 0) return NULL;
	else if (goods.size () != 1) {
	    Ymir::Error::templateSpecialisation (goods [0]-> ident (),
						 goods [1]-> ident ()
	    );
	    return verifErrors ();
	}

	Table::instance ().addCall (tok);
	FrameProto info;
	if (right-> toValidate) {
	    info = right-> toValidate-> validate (right, right-> treat);
	    if (info != NULL)
		right-> name = Mangler::mangle_functionv (info-> name (), info);
	    right-> proto = info;
	} else {
	    info = goods [0]-> validate (right, right-> treat);
	    if (info != NULL)
		right-> name = Mangler::mangle_function (info-> name (), info);
	    right-> proto = info;
	}


	if (Ymir::Error::nb_errors - nbErrorBeg && !tok.isEof ()) {
	    nbTmpsCreation ++;
	    if (nbTmpsCreation < 4) {
		Ymir::Error::templateCreation (tok);
	    } else if (this-> itsUpToMe) {
		Ymir::Error::templateCreation2 (tok);
	    }
	    right-> ret = NULL;
	    verifErrors ();
	    return right;
	}

	if (info == NULL) return NULL;
	right-> ret = info-> type ()-> type-> clone ();
	right-> ret-> value () = info-> type ()-> value ();

	return right;
    }

    InfoType IFunctionInfo::UnaryOp (Word op) {
	if (op == Token::AND) return toPtr ();
	return NULL;
    }

    InfoType IFunctionInfo::TempOp (const std::vector<syntax::Expression> & params) {
	auto frames = getFrames ();
	std::vector <Frame> ret;
	for (auto it : frames) {
	    if (auto aux = it-> TempOp (params))
		ret.push_back (aux);
	}

	if (ret.size () != 0) {
	    return new (Z0)  IFunctionInfo (this-> _space, this-> _name, ret);
	}
	return NULL;
    }

    InfoType IFunctionInfo::CompOp (InfoType other) {
	if (auto ot = other-> to<IPtrFuncInfo> ()) {
	    auto score = this-> CallOp (Word::eof (), ot-> getParams ());
	    
	    if (score == NULL) return NULL;
	    if (!score-> ret-> isSame (ot-> getType ()))
		return NULL;

	    for (auto it : Ymir::r (0, score-> treat.size ())) {
		if (!ot-> getParams() [it]-> isSame (score-> treat [it]))
		    return NULL;
	    }
	    
	    auto ret = (PtrFuncInfo) ot-> cloneConst ();
	    ret-> getScore () = score;
	    ret-> binopFoo = &FunctionUtils::InstAffect;
	    return ret;	
	}
	return NULL;
    }

    std::string IFunctionInfo::innerTypeString () {
	auto frames = getFrames ();
	if (frames.size () == 1 && this-> _info) {
	    Ymir::OutBuffer buf;
	    if (!this-> _info-> isPrivate ()) {
		buf.write (Ymir::format ("function <%.%> %",
					 this-> _space.toString ().c_str (),
					 this-> _name.c_str (),				     
					 this-> _info-> toString ().c_str ()
		));
	    } else {
		buf.write (Ymir::format ("prv function <%.%> %",
					 this-> _space.toString ().c_str (),
					 this-> _name.c_str (),				     
					 this-> _info-> toString ().c_str ()
		));
	    }
	    return buf.str ();
	} else {
	    return Ymir::format ("function <%.%>",
				 this-> _space.toString ().c_str (),
				 this-> _name.c_str ()
	    );
	}
    }

    std::string IFunctionInfo::typeString () {
	return innerTypeString ();
    }
    
    std::string IFunctionInfo::innerSimpleTypeString () {
	return "F";
    }

    InfoType IFunctionInfo::getTemplate (ulong i) {
	auto frames = this-> getFrames ();
	if (frames.size () == 1) {
	    auto infoTypes = frames [0]-> getParamTypes ();
	    if (i < infoTypes.size ()) return infoTypes [i];
	    else if (i == infoTypes.size ())
		return frames [0]-> getRetType ();
	}
	return NULL;
    }

    std::vector <InfoType> IFunctionInfo::getTemplate (ulong bef, ulong af) {
	auto frames = this-> getFrames ();
	if (frames.size () == 1) {
	    auto infoTypes = frames [0]-> getParamTypes ();
	    if (bef < infoTypes.size ()) {
		std::vector <InfoType> ret;
		for (auto it : Ymir::r (bef, infoTypes.size () - af)) {
		    ret.push_back (infoTypes [it]);
		}
		return ret;
	    }
	}
	return {NULL};
    }
    
    ulong IFunctionInfo::nbTemplates () {
	auto frames = this-> getFrames ();
	if (frames.size () == 1) {
	    auto infoTypes = frames [0]-> getParamTypes ();
	    return infoTypes.size () + 1;
	}
	return 0;
    }
    
    bool& IFunctionInfo::alone () {
	return this-> _alone;
    }

    std::string IFunctionInfo::name () {
	return this-> _name;
    }

    InfoType IFunctionInfo::toPtr () {
	// auto frames = getFrames ();
	// if (frames.size () == 1) {
	//     auto infoTypes = frames [0]-> getParamTypes ();
	//     auto score = this-> CallOp ({op.getLocus (), ""}, infoTypes);
	//     if (score == NULL || score-> ret == NULL) return NULL;
	//     auto ret = new (Z0) IPtrFuncInfo (true);
	//     ret-> getParams () = infoTypes;
	//     ret-> getType () = score-> ret-> cloneConst ();
	//     ret-> getScore () = score;
	    
	//     ret-> nextBinop.push_back (&FunctionUtils::InstAffect);
	//     ret-> binopFoo = &PtrFuncUtils::InstAffectComp;
	    
	//     return ret;	
	// }
	return NULL;
    }

    std::map<Word, std::string> IFunctionInfo::candidates () {
	std::map <Word, std::string> rets;
	auto frames = this-> getFrames ();
	/*for (auto : frames) {
	    // TODO
	    //ret [it-> ident] = it-> protoString ();
	    }*/
	return rets;
    }

    const char* IFunctionInfo::getId () {
	return IFunctionInfo::id ();
    }
    
}
