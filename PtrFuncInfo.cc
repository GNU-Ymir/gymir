#include <ymir/semantic/types/_.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/semantic/utils/PtrUtils.hh>
#include <ymir/semantic/utils/FixedUtils.hh>
#include <ymir/ast/ParamList.hh>
#include <ymir/semantic/tree/Generic.hh>

namespace semantic {

    namespace PtrFuncUtils {
	using namespace syntax;
	using namespace std;
	using namespace Ymir;

	template <typename T>
	T getAndRemoveBack (std::list <T> &list) {
	    auto last = list.back ();
	    list.pop_back ();
	    return last;
	}
	
	Tree InstAffectDelegate (Word loc, InfoType, Expression left, Expression right) {
	    auto ltree = left-> toGeneric ();
	    auto rtree = right-> toGeneric ();
	    auto ptrl = Ymir::getAddr (loc.getLocus (), ltree).getTree ();
	    auto ptrr = Ymir::getAddr (loc.getLocus (), rtree).getTree ();
	    tree tmemcopy = builtin_decl_explicit (BUILT_IN_MEMCPY);
	    tree size = TYPE_SIZE_UNIT (ltree.getType ().getTree ());
	    auto result = build_call_expr (tmemcopy, 3, ptrl, ptrr, size);
	    return Ymir::compoundExpr (loc.getLocus (), result, ltree);
	}

	Tree InstCastDelegate (Word loc, InfoType, Expression left, Expression) {
	    auto ret = left-> toGeneric (); 
	    if (ret.getType ().getTreeCode () == RECORD_TYPE) return ret;
	    else {
		auto auxType = Ymir::makeStructType ("", 2,
						get_identifier ("obj"),
						build_pointer_type (void_type_node),
						get_identifier ("ptr"),
						ret.getType ().getTree ()
						);
		
		auto auxVar = Ymir::makeAuxVar (loc.getLocus (), ISymbol::getLastTmp (), auxType);
		auto ptr = Ymir::getField (loc.getLocus (), auxVar, "ptr");
		auto result = buildTree (MODIFY_EXPR, loc.getLocus (), void_type_node, ptr, ret);
		return Ymir::compoundExpr (loc.getLocus (), result, auxVar);
	    }
	}

		
	Tree InstAffectComp (Word loc, InfoType type, Expression left, Expression right) {
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);

	    auto ltree = left-> toGeneric ();
	    auto rtree = type-> buildBinaryOp (
		loc, type, left, right
	    );
	    if (ltree.getType ().getTree () == rtree.getType ().getTree ()) 
		return buildTree (MODIFY_EXPR, loc.getLocus (), ltree.getType (), ltree, rtree);
	    else {		
		auto ptrl = Ymir::getAddr (loc.getLocus (), ltree).getTree ();
		auto ptrr = Ymir::getAddr (loc.getLocus (), rtree).getTree ();
		tree tmemcopy = builtin_decl_explicit (BUILT_IN_MEMCPY);
		tree size = TYPE_SIZE_UNIT (ltree.getType ().getTree ());
		auto result = build_call_expr (tmemcopy, 3, ptrl, ptrr, size);
		return Ymir::compoundExpr (loc.getLocus (), result, ltree);
	    }
	}

	Tree InstCall (Word loc, InfoType ret, Expression left, Expression paramsExp) {
	    ParamList params = paramsExp-> to <IParamList> ();
	    auto fn = left-> toGeneric ();
	    std::vector <tree> args = params-> toGenericParams (params-> getTreats ());
	    auto func = left-> info-> type-> to<IPtrFuncInfo> ();
	    if (auto ref = left-> info-> type-> to <IRefInfo> ()) {
		func = ref-> content ()-> to<IPtrFuncInfo> ();
	    }
	    
	    if (func-> isDelegate ()) {
		auto closureType = getField (loc.getLocus (), fn, "obj");
		auto fnPtr = getField (loc.getLocus (), fn, "ptr");
		args.insert (args.begin (), closureType.getTree ());
		return build_call_array_loc (loc.getLocus (),
					     ret-> toGeneric ().getTree (),
					     fnPtr.getTree (),
					     args.size (),
					     args.data ()
		);		
	    } else {	    
		return build_call_array_loc (loc.getLocus (),
					     ret-> toGeneric ().getTree (),
					     fn.getTree (),
					     args.size (),
					     args.data ()
		);
	    }
	}
	
    }
    
    IPtrFuncInfo::IPtrFuncInfo (bool isConst)
	: IInfoType (isConst)
    {}

    bool IPtrFuncInfo::isSameNoDTest (InfoType other) {
	if (auto ptr = other-> to <IPtrFuncInfo> ()) {
	    if (!this-> ret-> isSame (ptr-> ret)) return false;
	    if (this-> params.size () != ptr-> params.size ())
		return false;
	    for (auto it : Ymir::r (0, this-> params.size ())) {
		if (!ptr-> params [it]-> isSame (this-> params [it])) {
		    return false;
		}
	    }
	    return true;
	}
	return false;
    }
    
    bool IPtrFuncInfo::isSame (InfoType other) {
	if (auto ptr = other-> to <IPtrFuncInfo> ()) {
	    if (this-> isDelegate () != ptr-> isDelegate ()) return false;
	    if (!this-> ret-> isSame (ptr-> ret)) return false;
	    if (this-> params.size () != ptr-> params.size ())
		return false;
	    for (auto it : Ymir::r (0, this-> params.size ())) {
		if (!ptr-> params [it]-> isSame (this-> params [it])) {
		    return false;
		}
	    }
	    return true;
	}
	return false;
    }
    
    InfoType IPtrFuncInfo::BinaryOp (Word token, syntax::Expression right) {
	if (token == Token::EQUAL) return Affect (right);
	else if (token == Keys::IS) return Is (right);
	else if (token == Keys::NOT_IS) return NotIs (right);
	return NULL;
    }

    InfoType IPtrFuncInfo::BinaryOpRight (Word token, syntax::Expression left) {
	if (token == Token::EQUAL) return AffectRight (left);
	return NULL;
    }	

    ApplicationScore IPtrFuncInfo::CallOp (Word token, syntax::ParamList params) {
	if (params-> getParams ().size () != this-> params.size ()) 
	    return NULL;

	auto score = new (Z0) IApplicationScore (token);
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    InfoType info = this-> params [it];
	    auto type = params-> getParams() [it]-> info-> type-> CompOp (info);
	    if (type) type = type-> ConstVerif (info);
	    if (type) {
		score-> score += 1;
		score-> treat.push_back (type);
	    } else return NULL;
	}

	auto ret = this-> ret-> cloneConst ();
	ret-> multFoo = &PtrFuncUtils::InstCall;
	score-> dyn = true;
	score-> ret = ret;	
	return score;
    }

    InfoType IPtrFuncInfo::Affect (syntax::Expression right) {
	if (this-> isSame (right-> info-> type)) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &PtrUtils::InstAffect;
	    return ret;
	} else if (right-> info-> type-> is <INullInfo> ()) {
	    auto ret = this-> clone ();
	    if (this-> isDelegate ()) {
		ret-> binopFoo = &PtrUtils::InstAffect;
	    } else {
		ret-> binopFoo = &PtrFuncUtils::InstAffectDelegate;
	    }
	    return ret;
	} else if (right-> info-> type-> is <IFunctionInfo> ()) {
	    auto ret = right-> info-> type-> CompOp (this);
	    if (ret && ret-> isSame (this)) {
		ret-> nextBinop.push_back (ret-> binopFoo);		
		ret-> binopFoo = &PtrFuncUtils::InstAffectComp;
		return ret;
	    }
	}
	return NULL;
    }
    
    InfoType IPtrFuncInfo::AffectRight (syntax::Expression left) {
	if (left-> info-> type-> is <IUndefInfo> ()) {
	    auto ret = this-> clone ();
	    if (this-> isDelegate ()) {
		ret-> binopFoo = &PtrFuncUtils::InstAffectDelegate;
	    } else {
		ret-> binopFoo = &PtrUtils::InstAffect;
	    }
	    return ret;
	}
	return NULL;
    }
    
    InfoType IPtrFuncInfo::CompOp (InfoType other) {
	if (other-> isSame (this) || other-> is <IUndefInfo> ()) {
	    auto ptr = this-> clone ();
	    if (this-> isDelegate ()) 
		ptr-> binopFoo = &PtrFuncUtils::InstCastDelegate;
	    else 
		ptr-> binopFoo = &PtrUtils::InstCast;
	    
	    return ptr;
	}
	return NULL;
    }

    InfoType IPtrFuncInfo::Is (syntax::Expression right) {
	if (right-> info-> type-> isSame (this)) {
	    auto ret = new (Z0) IBoolInfo (true);
	    ret-> binopFoo = &FixedUtils::InstTest;
	    return ret;
	} else if (right-> info-> type-> is <INullInfo> ()) {
	    auto ret = new (Z0) IBoolInfo (true);
	    ret-> binopFoo = &FixedUtils::InstTest;
	    return ret;
	}
	return NULL;
    }
    
    InfoType IPtrFuncInfo::NotIs (syntax::Expression right) {
	if (right-> info-> type-> isSame (this)) {
	    auto ret = new (Z0) IBoolInfo (true);
	    ret-> binopFoo = &FixedUtils::InstTest;
	    return ret;
	} else if (right-> info-> type-> is <INullInfo> ()) {
	    auto ret = new (Z0) IBoolInfo (true);
	    ret-> binopFoo = &FixedUtils::InstTest;
	    return ret;
	}
	return NULL;
    }
    
    ulong IPtrFuncInfo::nbTemplates () {
	return this-> params.size () + 1;
    }
    
    InfoType IPtrFuncInfo::getTemplate (ulong i) {
	if (i < this-> params.size ()) return this-> params [i];
	else if (i == this-> params.size ())
	    return this-> ret;
	return NULL;
    }

    std::vector <InfoType> IPtrFuncInfo::getTemplate (ulong bef, ulong af) {
	if (bef < this-> params.size ()) {
	    std::vector <InfoType> ret;
	    for (auto it : Ymir::r (bef, this-> params.size () - af)) {
		ret.push_back (this-> params [it]);
	    }
	    return ret;
	} else if (bef == this-> params.size ()) {
	    return {this-> ret};
	}
	return {NULL};
    }
    
    InfoType IPtrFuncInfo::onClone () {
	auto aux = new (Z0) IPtrFuncInfo (this-> isConst ());
	for (auto it : this-> params) {
	    aux-> params.push_back (it-> clone ());
	}
	aux-> ret = this-> ret-> clone ();
	aux-> score = this-> score;
	aux-> _isDelegate = this-> _isDelegate; 
	return aux;
    }

    InfoType IPtrFuncInfo::DColonOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "typeid") {
	    return StringOf ();
	} else if (var-> token == "paramTuple") return NULL;
	else if (var-> token == "retType") return NULL;
	return NULL;
    }
 	
    std::string IPtrFuncInfo::innerTypeString () {
	Ymir::OutBuffer buf;
	if (this-> isDelegate ()) buf.write ("dg (");
	else buf.write ("fn (");
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    buf.write (this-> params [it]-> typeString ());
	    if (it < (int) this-> params.size () - 1)
		buf.write (",");
	}
	buf.write (")-> ", this-> ret-> typeString ());
	return buf.str ();
    }

    std::string IPtrFuncInfo::innerSimpleTypeString () {
	Ymir::OutBuffer buf;
	if (this-> isDelegate ()) buf.write ("DG");
	else buf.write ("PF");
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    buf.write (this-> params [it]-> simpleTypeString ());
	}
	buf.write ("Z", this-> ret-> typeString ());
	return buf.str ();
    }

    InfoType& IPtrFuncInfo::getType () {
	return this-> ret;
    }

    std::vector <InfoType>& IPtrFuncInfo::getParams () {
	return this-> params;
    }

    ApplicationScore& IPtrFuncInfo::getScore () {
	return this-> score;
    }

    bool & IPtrFuncInfo::isDelegate () {
	return this-> _isDelegate;
    }
    
    Ymir::Tree IPtrFuncInfo::toGeneric () {	
	std::vector<tree> fndecl_type_params;
	for (auto it : this->params) {
	    if (!it-> is <IStructInfo> ()) {
		fndecl_type_params.push_back (it-> toGeneric ().getTree ());
	    } else {
		fndecl_type_params.push_back (void_type_node);
	    }
	}

	tree ret = this-> ret-> toGeneric ().getTree ();
	auto ptr = build_pointer_type (
	    build_function_type_array (ret, fndecl_type_params.size (), fndecl_type_params.data ())
	);
	
	if (this-> isDelegate ()) {
	    return Ymir::makeStructType ("", 2,
					 get_identifier ("obj"),
					 build_pointer_type (
							     void_type_node
							     ),
					 get_identifier ("ptr"),
					 ptr
	    );	    
	} else return ptr;	
    }

    const char * IPtrFuncInfo::getId () {
	return IPtrFuncInfo::id ();
    }
    
}
