#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/utils/PtrUtils.hh>
#include <ymir/ast/ParamList.hh>

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
	
	Tree InstAffectComp (Word loc, InfoType type, Expression left, Expression right) {
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);

	    auto ltree = left-> toGeneric ();
	    auto rtree = type-> buildBinaryOp (
		loc, type, left, right
	    );

	    return buildTree (MODIFY_EXPR, loc.getLocus (), ltree.getType (), ltree, rtree);	    
	}

	Tree InstCall (Word loc, InfoType ret, Expression left, Expression paramsExp) {
	    ParamList params = paramsExp-> to <IParamList> ();
	    auto fn = left-> toGeneric ();
	    std::vector <tree> args = params-> toGenericParams (params-> getTreats ());	    
	    return build_call_array_loc (loc.getLocus (),
					 ret-> toGeneric ().getTree (),
					 fn.getTree (),
					 args.size (),
					 args.data ()
	    );
	}
	
    }
    
    IPtrFuncInfo::IPtrFuncInfo (bool isConst)
	: IInfoType (isConst)
    {}

    bool IPtrFuncInfo::isSame (InfoType other) {
	if (auto ptr = other-> to <IPtrFuncInfo> ()) {
	    if (!this-> ret-> isSame (ptr-> ret)) return false;
	    if (this-> params.size () != ptr-> params.size ())
		return false;
	    for (auto it : Ymir::r (0, this-> params.size ())) {
		if (!ptr-> params [it]-> isSame (this-> params [it]))
		    return false;
	    }
	    return true;
	}
	return false;
    }
    
    InfoType IPtrFuncInfo::BinaryOp (Word token, syntax::Expression right) {
	if (token == Token::EQUAL) return Affect (right);
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
	    ret-> binopFoo = &PtrUtils::InstAffect;
	    return ret;
	} else if (right-> info-> type-> is <IFunctionInfo> ()) {
	    auto ret = right-> info-> type-> CompOp (this);
	    if (ret) {
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
	    ret-> binopFoo = &PtrUtils::InstAffect;
	    return ret;
	}
	return NULL;
    }
    
    InfoType IPtrFuncInfo::CompOp (InfoType other) {
	if (other-> isSame (this) || other-> is <IUndefInfo> ()) {
	    auto ptr = this-> clone ();
	    ptr-> binopFoo = &PtrUtils::InstCast;
	    return ptr;
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
    
    InfoType IPtrFuncInfo::onClone () {
	auto aux = new (Z0) IPtrFuncInfo (this-> isConst ());
	for (auto it : this-> params) {
	    aux-> params.push_back (it-> clone ());
	}
	aux-> ret = this-> ret-> clone ();
	aux-> score = this-> score;
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
	Ymir::OutBuffer buf ("fn(");
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    buf.write (this-> params [it]-> typeString ());
	    if (it < (int) this-> params.size () - 1)
		buf.write (",");
	}
	buf.write (")->", this-> ret-> typeString ());
	return buf.str ();
    }

    std::string IPtrFuncInfo::innerSimpleTypeString () {
	Ymir::OutBuffer buf ("PF");
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
	return build_pointer_type (
	    build_function_type_array (ret, fndecl_type_params.size (), fndecl_type_params.data ())
	);	
    }

    const char * IPtrFuncInfo::getId () {
	return IPtrFuncInfo::id ();
    }
    
}
