#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/pack/Frame.hh>
#include <ymir/utils/Options.hh>
#include <ymir/utils/Mangler.hh>
#include <ymir/semantic/pack/PureFrame.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/semantic/value/Value.hh>
#include <ymir/semantic/value/LambdaValue.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/semantic/pack/InternalFunction.hh>

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

	
	Tree createClosureVarMoved (location_t loc, ApplicationScore score) {
	    Ymir::TreeStmtList list;
	    auto closureType = score-> proto-> createClosureType ();	
	    auto type_tree = build_pointer_type (closureType.getTree ());
	    auto fn = InternalFunction::getMalloc ();
	    auto byte_len = TYPE_SIZE_UNIT (closureType.getTree ());
	    auto alloc = build_call_array_loc (loc,
					       type_tree,
					       fn.getTree (), 1, &byte_len);
	
	    auto closureVar = Ymir::makeAuxVar (BUILTINS_LOCATION, ISymbol::getLastTmp (), type_tree);
	
	    list.append (buildTree (
		MODIFY_EXPR,
		loc,
		Ymir::Tree (type_tree), closureVar, alloc
	    ));

	    for (auto it : score-> proto-> closure ()) {
		auto field = Ymir::getField (BUILTINS_LOCATION,
					     Ymir::getPointerUnref (loc, closureVar, closureType, 0), it-> token.getStr ());
		auto lastInfo = it-> lastInfoDecl ();
	    
		list.append (buildTree (
		    MODIFY_EXPR, loc, void_type_node, field, lastInfo
		));
	    }
	
	    return Ymir::compoundExpr (loc,
				       list.getTree (),
				       closureVar
	    );	    
	}

	Tree createClosureVar (location_t loc, ApplicationScore score) {
	    Ymir::TreeStmtList list;
	    if (score-> proto-> isMoved ()) return createClosureVarMoved (loc, score);
	    auto values = score-> proto-> closure ();
	    auto closureType = score-> proto-> createClosureType ();
	    auto closureVar = Ymir::makeAuxVar (loc, ISymbol::getLastTmp (), closureType);
	    
	    for (auto it : values) {
		auto field = Ymir::getField (loc, closureVar, it-> token.getStr ());
		auto lastInfo = it-> lastInfoDecl ();
		list.append (buildTree (
		    MODIFY_EXPR, loc, void_type_node, field, getAddr (lastInfo)
		));
	    }
	    
	    return Ymir::compoundExpr (loc,
				       list.getTree (),
				       getAddr (closureVar)
	    );	   
	}
	
	Tree InstAffectDelegate (Word locus, InfoType type, Expression, Expression) {
	    auto loc = locus.getLocus ();
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
	    tree fnPtr = build1 (ADDR_EXPR, build_pointer_type (fndecl_type), fndecl);

	    Ymir::TreeStmtList list;
	    auto closureVar = createClosureVar (loc, func-> getScore ());
	    
	    auto finalType = type-> toGeneric ();
	    auto finalRet = Ymir::makeAuxVar (locus.getLocus (), ISymbol::getLastTmp (), finalType);
	    
	    auto obj = Ymir::getField (loc, finalRet, "obj");
	    auto ptr = Ymir::getField (loc, finalRet, "ptr");

	    
	    list.append (buildTree (
		MODIFY_EXPR, locus.getLocus (), void_type_node, obj, closureVar
	    ));
	    
	    list.append (buildTree (
		MODIFY_EXPR, locus.getLocus (), void_type_node, ptr, fnPtr
	    ));
	    
	    return Ymir::compoundExpr (locus.getLocus (), list.getTree (), finalRet);
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

    bool IFunctionInfo::passingConst (InfoType other) {
	if (auto ot = other-> to <IPtrFuncInfo> ()) {
	    return ot-> isDelegate ();
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
	ret-> _isLambda = this-> _isLambda;
	ret-> _fromTemplates = this-> _fromTemplates;
	ret-> value () = this-> value ();
	ret-> _isConstr = this-> _isConstr;
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
	    if (auto fun = it-> type ()-> to<IFunctionInfo> ()) {
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
	if (op == Token::EQUAL && left-> info-> type ()-> is <IUndefInfo> ()) {
	    auto frames = getFrames ();
	    if (frames.size () == 1) {//&& frames [0]-> isPure ()) {
		auto infoTypes = frames [0]-> getParamTypes ();
		auto score = this-> CallOp ({op.getLocus (), ""}, infoTypes);
		if (score == NULL || score-> ret == NULL) return NULL;
		auto ret = new (Z0) IPtrFuncInfo (true);
		ret-> getParams () = infoTypes;
		ret-> getType () = score-> ret-> clone ();
		ret-> getScore () = score;
		
		ret-> isDelegate () = score-> proto-> isDelegate ();
		if (ret-> isDelegate ()) {			 
		    if (!score-> proto-> attached ()-> isMoved ()) {
			std::vector <Symbol> closures;
			for (auto it : score-> proto-> attached ()-> closure ())
			    closures.push_back (it-> info);
			
			if (Table::instance ().verifyClosureLifeTime (			
			    left-> info-> lifeTime (),
			    closures
			))			    
			    ret-> addClosure (closures);
		    }
		    ret-> nextBinop.push_back (&FunctionUtils::InstAffectDelegate);
		} else {
		    ret-> nextBinop.push_back (&FunctionUtils::InstAffect);
		}
		
		ret-> binopFoo = &PtrFuncUtils::InstAffectComp;
		
		return ret;	
	    }
	}
	return this-> CompOp (left-> info-> type ());
    }

    InfoType IFunctionInfo::Affect () {
	auto frames = getFrames ();
	if (frames.size () == 1) {//&& frames [0]-> isPure ()) {
	    auto infoTypes = frames [0]-> getParamTypes ();
	    auto score = this-> CallOp (Word::eof (), infoTypes);
	    if (score == NULL || score-> ret == NULL) return NULL;
	    auto ret = new (Z0) IPtrFuncInfo (true);
	    ret-> getParams () = infoTypes;
	    ret-> getType () = score-> ret-> clone ();
	    ret-> getScore () = score;
		
	    ret-> isDelegate () = score-> proto-> isDelegate ();
	    if (ret-> isDelegate ()) {			 
		if (!score-> proto-> attached ()-> isMoved ()) {
		    std::vector <Symbol> closures;
		    for (auto it : score-> proto-> attached ()-> closure ())
			closures.push_back (it-> info);

		    if (Table::instance ().verifyClosureLifeTime (			
			0,
			closures
		    ))
			ret-> addClosure (closures);					
		}
		ret-> binopFoo = (&FunctionUtils::InstAffectDelegate);
	    } else {
		ret-> binopFoo = (&FunctionUtils::InstAffect);
	    }
				
	    return ret;	
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
    
    ApplicationScore IFunctionInfo::CallAndThrow (Word tok, const std::vector <InfoType> & params, FrameProto & info) {
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

	if (!Table::instance ().addCall (tok)) return NULL;
	
	if (right-> toValidate) {
	    if (Table::instance ().hasCurrentContext (Keys::SAFE) && !(right-> toValidate-> has (Keys::SAFE) || right-> toValidate-> has (Keys::TRUSTED)))
		Ymir::Error::callUnsafeInSafe (tok);
	    
	    info = right-> toValidate-> validate (right, right-> treat);
	    right-> proto = info;
	} else {
	    if (Table::instance ().hasCurrentContext (Keys::SAFE) && !(goods [0]-> has (Keys::SAFE) || goods [0]-> has (Keys::TRUSTED)))
		Ymir::Error::callUnsafeInSafe (tok);
	    
	    info = goods [0]-> validate (right, right-> treat);
	    right-> proto = info;
	}
	return right;
    }

    ApplicationScore IFunctionInfo::CallOp (Word tok, const std::vector<InfoType> & params) {
	if (needToReset) {
	    itsUpToMe = true;
	    needToReset = false;
	}
	
	FrameProto info;
	ulong nbErrorBeg = Ymir::Error::nb_errors;
	auto right = this-> CallAndThrow (tok, params, info);

	if (Ymir::Error::nb_errors - nbErrorBeg && !tok.isEof ()) {
	    nbTmpsCreation ++;
	    if (nbTmpsCreation < 4 || Options::instance ().isVerbose ()) {
		Ymir::Error::templateCreation (tok);
	    } else if (this-> itsUpToMe) {
		Ymir::Error::templateCreation2 (tok, nbTmpsCreation - 3);
	    }
	    
	    if (right)
		right-> ret = NULL;
	    verifErrors ();
	    return right;
	} else if (right == NULL) return NULL;

	verifErrors ();
	if (info == NULL) return NULL;
	right-> ret = info-> type ()-> type ()-> clone ();
	right-> ret-> value () = info-> type ()-> value ();

	return right;
    }

    
    ApplicationScore IFunctionInfo::CallOp (Word tok, syntax::ParamList params) {
	if (needToReset) {
	    itsUpToMe = true;
	    needToReset = false;
	}
	
	ulong nbErrorBeg = Ymir::Error::nb_errors;

	FrameProto info;
	auto types = params-> getParamTypes ();
	auto right = this-> CallAndThrow (tok, types, info);
	
	if (Ymir::Error::nb_errors - nbErrorBeg && !tok.isEof ()) {
	    nbTmpsCreation ++;
	    if (nbTmpsCreation < 4 || Options::instance ().isVerbose ()) {
		Ymir::Error::templateCreation (tok);
	    } else if (this-> itsUpToMe) {
		println (nbTmpsCreation - 3);
		Ymir::Error::templateCreation2 (tok, nbTmpsCreation - 3);
	    }
	    
	    if (right) 
		right-> ret = NULL;
	    verifErrors ();
	    return right;
	} else if (right == NULL) return NULL;
	
	verifErrors ();
	if (info == NULL) return NULL;
	right-> ret = info-> type ()-> type ()-> clone ();
	right-> ret-> value () = info-> type ()-> value ();

	return right;
    }

    InfoType IFunctionInfo::UnaryOp (Word op) {
	if (op == Token::AND) return toPtr (op);
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
	    auto fin = this-> cloneOnExitWithInfo ()-> to <IFunctionInfo> ();
	    fin-> _fromTemplates = ret;
	    fin-> _alone = false;
	    return fin;
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
	    ret-> isDelegate () = score-> proto-> isDelegate () || ot-> isDelegate ();
	    if (!score-> proto-> isDelegate ()) ret-> forcedDelegate () = true;

	    if (ret-> isDelegate () && !ot-> isDelegate ()) return NULL;
	    if (ot-> isDelegate ()) score-> proto-> isForcedDelegate ();
	    if (ret-> isDelegate ()) {       	
		if (!score-> proto-> attached ()-> isMoved ()) {
		    std::vector <Symbol> closures;
		    for (auto it : score-> proto-> attached ()-> closure ())
			closures.push_back (it-> info);
		    
		    if (!other-> symbol ()) {
			if (Table::instance ().verifyClosureLifeTime (			
			    other-> symbol ()-> lifeTime (),
			    closures
			))
			ret-> addClosure (closures);
		    } else {
			if (Table::instance ().verifyClosureLifeTime (			
			    0,
			    closures
			))
			    ret-> addClosure (closures);			
		    }
		}
		ret-> binopFoo = &FunctionUtils::InstAffectDelegate;
	    } else {
		ret-> binopFoo = &FunctionUtils::InstAffect;
	    }
	    return ret;	
	} else if (other-> is <IUndefInfo> ()) {
	    return this-> Affect ();
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

    bool& IFunctionInfo::isLambda () {
	return this-> _isLambda;
    }

    bool& IFunctionInfo::isConstr () {
	return this-> _isConstr;
    }

    bool& IFunctionInfo::isVirtual () {
	return this-> _isVirtual;
    }

    bool& IFunctionInfo::isOver () {
	return this-> _isOver;
    }
    
    std::string IFunctionInfo::name () {
	return this-> _name;
    }

    InfoType IFunctionInfo::toPtr (Word op) {
	auto frames = getFrames ();
	if (frames.size () == 1) {
	    auto infoTypes = frames [0]-> getParamTypes ();
	    auto score = this-> CallOp ({op.getLocus (), ""}, infoTypes);
	    if (score == NULL || score-> ret == NULL) return NULL;
	    auto ret = new (Z0) IPtrFuncInfo (true);
	    ret-> getParams () = infoTypes;
	    ret-> getType () = score-> ret-> clone ();
	    ret-> getScore () = score;
	    
	    ret-> binopFoo = &FunctionUtils::InstAffect;	    
	    return ret;	
	}
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
