#include <ymir/semantic/object/MethodInfo.hh>
#include <ymir/ast/ParamList.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/pack/MethodFrame.hh>
#include <ymir/ast/ParamList.hh>

namespace semantic {

    namespace MethodUtils {
	using namespace Ymir;
	using namespace syntax;
	
	Tree InstCall (Word token, InfoType type, Expression left, Expression right, ApplicationScore score) {
	    auto params = right-> to <IParamList> ();
	    
	    std::vector <InfoType> treats (params-> getTreats ().begin () + 1, params-> getTreats ().end ());
	    std::vector <tree> args = params-> toGenericParams (treats);

	    Ymir::TreeStmtList list;
	    auto ltree = Ymir::getExpr (list, left-> toGeneric ());

	    args.insert (args.begin (), getAddr (ltree).getTree ());
	    std::vector <tree> types;
	    for (auto it : args)
		types.push_back (TREE_TYPE (it));
	    
	    tree ret = type-> toGeneric ().getTree ();
	    auto ptr_type = build_pointer_type (
		build_function_type_array (ret, types.size (), types.data ())
	    );
	    tree access;
	    if (score-> proto == NULL) {
		auto vtable = getField (token.getLocus (), ltree, Keys::VTABLE_FIELD);
		auto nb = score-> methIndex + 1; // The first one is the typeinfo
		auto padd = build_int_cst_type (long_unsigned_type_node, nb);
		access = Ymir::getPointerUnref (token.getLocus (), vtable, ptr_type, padd).getTree ();
	    } else {
		access = score-> proto-> toGeneric ().getTree ();
	    }
	     
	    return Ymir::compoundExpr (token.getLocus (),
				       list,
				       build_call_array_loc (token.getLocus (),
					 type-> toGeneric ().getTree (),
					 access,
					 args.size (),
					 args.data ())
	    );	    
	}

	Tree InstCallOnBinary (Word token, InfoType type, Expression left, Expression right) {
	    auto params = new (Z0) IParamList (token, {right});
	    auto score = type-> appScore;	    
	    params-> treats () = score-> treat;
	    return InstCall (token, type, left, params, score);
	}
	
    }
    
    IMethodInfo::IMethodInfo (AggregateInfo info, std::string name, const std::vector <Frame> & frames, const std::vector <int> & index, bool isStatic) :
	IInfoType (true),
	_info (info),
	_name (name),
	_frames (frames),
	_index (index),
	_isDynamic (!isStatic)
    {}

    bool IMethodInfo::isSame (InfoType) {
	return false;
    }

    InfoType IMethodInfo::onClone () {
	return new (Z0) IMethodInfo (this-> _info, this-> _name, this-> _frames, this-> _index, !this-> _isDynamic);
    }


    InfoType IMethodInfo::BinaryOp (Word op, syntax::Expression right) {
	if (op == Token::EQUAL && this-> isAttribute ()) {
	    auto params = new (Z0) syntax::IParamList (op, {right});
	    this-> eraseNonAttrib ();
	    auto score = this-> CallOp (op, params);
	    if (score) {
		auto ret = score-> ret;
		ret-> appScore = score;
		ret-> binopFoo = &MethodUtils::InstCallOnBinary;
		ret-> multFoo = NULL;
		return ret;
	    }
	}
	return NULL;
    }

    InfoType IMethodInfo::BinaryOpRight (Word, syntax::Expression) {
	return NULL;
    }

    ApplicationScore IMethodInfo::verifErrors () {
	if (itsUpToMe) {
	    itsUpToMe = false;
	    IFunctionInfo::needToReset = true;
	    IFunctionInfo::nbTmpsCreation = 0;
	}
	
	return NULL;
    }

    ApplicationScore IMethodInfo::CallAndThrow (Word tok, const std::vector <InfoType> & params, InfoType & retType) {
	std::vector <ApplicationScore> total;
	for (auto it : this-> _frames) {
	    total.push_back (it-> isApplicable (params));
	}

	std::vector <Frame> goods;
	std::vector <ulong> index;
	ApplicationScore right = new (Z0) IApplicationScore ();
	for (uint it = 0 ; it < total.size () ; it ++) {
	    if (total [it]) {
		if (goods.size () == 0 && total [it]-> score != 0) {
		    right = total [it];
		    goods.push_back (this-> _frames [it]);
		    index.push_back (this-> _index [it]);
		} else if (right-> score < total [it]-> score) {
		    goods.clear ();
		    index.clear ();
		    goods.push_back (this-> _frames [it]);
		    index.push_back (this-> _index [it]);
		} else if (right-> score == total [it]-> score && total [it]-> score != 0) {
		    goods.push_back (this-> _frames [it]);
		    index.push_back (this-> _index [it]);
		}		
	    }
	}

	if (goods.size () == 0) return NULL;
	else if (goods.size () != 1) {
	    Ymir::Error::templateSpecialisation (goods [0]-> ident (),
						 goods [1]-> ident ());
	    return verifErrors ();
	}

	right-> methIndex = index.back ();
	if (!Table::instance ().addCall (tok)) return NULL;
	auto fparams = std::vector <InfoType> (right-> treat.begin () + 1, right-> treat.end ());
	if (right-> toValidate) {
	    if (Table::instance ().hasCurrentContext (Keys::SAFE) && !(right-> toValidate-> has (Keys::SAFE) || right-> toValidate-> has (Keys::TRUSTED)))
		Ymir::Error::callUnsafeInSafe (tok);
	    FrameProto info = right-> toValidate-> validate (right, fparams);
	    retType = info-> type ()-> type ();
	    if (!right-> toValidate-> is <IMethodFrame> () || !right-> toValidate-> to <IMethodFrame> ()-> isVirtual () || !this-> _isDynamic) {
		right-> proto = info;
	    }
	    
	    right-> dyn = true;	    
	    right-> isMethod = true;
	} else {
	    if (Table::instance ().hasCurrentContext (Keys::SAFE) && !(goods [0]-> has (Keys::SAFE) || goods [0]-> has (Keys::TRUSTED)))
		Ymir::Error::callUnsafeInSafe (tok);
	    
	    FrameProto info = goods [0]-> validate (right, fparams);
	    retType = info-> type ()-> type ();
	    if (!goods [0]-> is <IMethodFrame> () || !goods [0]-> to<IMethodFrame> ()-> isVirtual () || !this-> _isDynamic) {
		right-> proto = info;
	    }
	    
	    right-> dyn = true;
	    right-> isMethod = true;
	}
	return right;
	
    }
    
    ApplicationScore IMethodInfo::CallOp (Word tok, syntax::ParamList params) {
	if (IFunctionInfo::needToReset) {
	    itsUpToMe = true;
	    IFunctionInfo::needToReset = false;
	}
	
	ulong nbErrorBeg = Ymir::Error::nb_errors;
	
	InfoType retType;
	auto types = params-> getParamTypes ();
	types.insert (types.begin (), new (Z0) IRefInfo (this-> _info-> isConst (), this-> _info-> clone ()));
	
	auto right = this-> CallAndThrow (tok, types, retType);
	
	if (Ymir::Error::nb_errors - nbErrorBeg && !tok.isEof ()) {
	    IFunctionInfo::nbTmpsCreation ++;
	    if (IFunctionInfo::nbTmpsCreation < 4 || Options::instance ().isVerbose ()) {
		Ymir::Error::templateCreation (tok);
	    } else if (this-> itsUpToMe) {
		Ymir::Error::templateCreation2 (tok, IFunctionInfo::nbTmpsCreation - 3);
	    }
	    
	    if (right)
		right-> ret = NULL;
	    verifErrors ();
	    return right;
	} else if (right == NULL) return NULL;

	verifErrors ();
	right-> ret = retType-> clone ();
	right-> ret-> value () = retType-> value ();
	right-> ret-> multFoo = &MethodUtils::InstCall;	
	
	return right;
    }

    InfoType IMethodInfo::TempOp (const std::vector <syntax::Expression> & params) {
	auto clone = this-> cloneOnExitWithInfo ()-> to <IMethodInfo> ();
	std::vector <Frame> ret;
	for (auto it : this-> _frames) {
	    if (auto aux = it-> TempOp (params)) {
		ret.push_back (aux);
	    }
	}

	if (ret.size () != 0) {
	    clone-> _frames = ret;
	    return clone;
	}
	return NULL;
    }
    
    
    
    std::string IMethodInfo::typeString () {
	return this-> innerTypeString ();
    }
    
    std::string IMethodInfo::innerTypeString () {
	return "method <" + this-> _info-> typeString () + "." + this-> _name + ">";
    }

    std::string IMethodInfo::innerSimpleTypeString () {
	Ymir::Error::assert ("TODO");
	return "";
    }

    const char* IMethodInfo::getId () {
	return IMethodInfo::id ();
    }            
    
    void IMethodInfo::eraseNonAttrib () {
	std::vector <Frame> frames;
	std::vector <int> index;
	if (this-> _name != Keys::COPY) {// Constructeur de copie
	    for (auto it : Ymir::r (0, this-> _frames.size ())) { 
		if (this-> _frames [it]-> has (Keys::ATTRIBUTE)) {
		    frames.push_back (this-> _frames [it]);
		    index.push_back (this-> _index [it]);
		}
	    }
	    this-> _frames = frames;
	    this-> _index = index;
	}
    }

    bool IMethodInfo::isAttribute () {
	for (auto it : this-> _frames) {
	    if (it-> has (Keys::ATTRIBUTE)) return true;
	}
	if (this-> _name == "#cpy") return true;
	return false;
    }

}
