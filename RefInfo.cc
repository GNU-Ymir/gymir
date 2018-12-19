#include <ymir/semantic/types/_.hh>
#include <ymir/ast/TreeExpression.hh>
#include <ymir/semantic/tree/_.hh>
#include <ymir/semantic/utils/PtrUtils.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/object/AggregateInfo.hh>
#include <ymir/syntax/Keys.hh>

namespace semantic {

    namespace RefUtils {
	using namespace syntax;
	using namespace Ymir;

	template <typename T>
	T getAndRemoveBack (std::list <T> &list) {
	    if (list.size () != 0) {
		auto last = list.back ();	    
		list.pop_back ();
		return last;
	    } else {
		return NULL;
	    }
	}

	Ymir::Tree InstUnrefApply (Word locus, InfoType t, std::vector <Var> & vars, Block block, Expression iter) {
	    auto type = t-> cloneOnExitWithInfo ();
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);
	    type-> applyFoo = getAndRemoveBack (type-> nextApply);

	    auto loc = locus.getLocus ();
	    auto innerType = iter-> info-> type ()-> to <IRefInfo> ()-> content ();
	    auto inner = innerType-> toGeneric ();

	    Ymir::TreeStmtList list;
	    auto iterExp = Ymir::getExpr (list, iter);
	    iterExp = getPointerUnref (locus.getLocus (), iterExp, inner, 0);

	    return Ymir::compoundExpr (loc, list, type-> buildApplyOp (
		locus, type, vars, block,
		new (Z0) ITreeExpression (iter-> token, innerType, iterExp)
	    ));	    
	}
	
	Ymir::Tree InstUnrefBinRight (Word locus, InfoType t, Expression left, Expression right) {
	    auto type = t-> cloneOnExitWithInfo ();
	    
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);

	    auto loc = locus.getLocus ();
	    auto innerType = right-> info-> type ()-> to<IRefInfo> ()-> content ();
	    auto inner = innerType-> toGeneric ();	    

	    Ymir::TreeStmtList list;
	    auto rightExp = Ymir::getExpr (list, right);	    
	    rightExp = getPointerUnref (locus.getLocus (), rightExp, inner, 0);
	    
	    return Ymir::compoundExpr (loc, list, type-> buildBinaryOp (
		locus,
		type,
		left,
		new (Z0)  ITreeExpression (right-> token, innerType, rightExp)
	    ));
	    
	}
		
	Ymir::Tree InstUnrefBin (Word locus, InfoType t, Expression left, Expression right) {
	    auto type = t-> cloneOnExitWithInfo ();
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);

	    auto loc = locus.getLocus ();
	    auto innerType = left-> info-> type ()-> to<IRefInfo> ()-> content ();
	    auto inner = innerType-> toGeneric ();	    
	    
	    Ymir::TreeStmtList list;
	    auto leftExp = Ymir::getExpr (list, left);
	    leftExp = getPointerUnref (locus.getLocus (), leftExp, inner, 0);
	    
	    if (type-> binopFoo) {
		return Ymir::compoundExpr (loc, list, type-> buildBinaryOp (
		    locus,
		    type,
		    new (Z0)  ITreeExpression (left-> token, innerType, leftExp),
		    right
		));
	    } else if (type-> unopFoo) {
		return Ymir::compoundExpr (loc, list, type-> buildUnaryOp (
		    locus,
		    type,
		    new (Z0)  ITreeExpression (left-> token, innerType, leftExp)
		));
	    }
	    return leftExp;
	}

	Ymir::Tree InstUnrefBinDouble (Word locus, InfoType t, Expression left, Expression right) {
	    auto type = t-> cloneOnExitWithInfo ();
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);

	    auto loc = locus.getLocus ();
	    Ymir::TreeStmtList list;
	    auto innerLeft = left-> info-> type ()-> to<IRefInfo> ()-> content ();
	    auto inner = innerLeft-> toGeneric ();	    
	    auto leftExp = Ymir::getExpr (list, left);
	    leftExp = getPointerUnref (locus.getLocus (), leftExp, inner, 0);
	    
	    auto innerRight = right-> info-> type ()-> to<IRefInfo> ()-> content ();
	    inner = innerRight-> toGeneric ();
	    auto rightExp = Ymir::getExpr (list, right);
	    rightExp = getPointerUnref (locus.getLocus (), rightExp, inner, 0);
	    

	    return Ymir::compoundExpr (loc, list, type-> buildBinaryOp (
		locus,
		type,
		new (Z0)  ITreeExpression (left-> token, innerLeft, leftExp),
		new (Z0)  ITreeExpression (right-> token, innerRight, rightExp)
	    ));
	}
	
	Ymir::Tree InstUnrefUn (Word locus, InfoType t, Expression left) {
	    auto type = t-> cloneOnExitWithInfo ();
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);
	    
	    auto inner = left-> info-> type ()-> to<IRefInfo> ()-> content ()-> toGeneric ();

	    Ymir::TreeStmtList list;	    
	    auto leftExp = Ymir::getExpr (list, left);
	    leftExp = getPointerUnref (locus.getLocus (), leftExp, inner, 0);
	    
	    if (type-> unopFoo) {
		return Ymir::compoundExpr (locus.getLocus (), list, type-> buildUnaryOp (
		    locus,
		    type,
		    new (Z0)  ITreeExpression (left-> token, left-> info-> type (), leftExp)
		));
	    } else if (type-> binopFoo) {
		return Ymir::compoundExpr (locus.getLocus (), list, type-> buildBinaryOp (
		    locus,
		    type,
		    new (Z0)  ITreeExpression (left-> token, left-> info-> type (), leftExp),
		    new (Z0)  ITreeExpression (locus, type, Ymir::Tree ())
		));
	    } else {
		return Ymir::compoundExpr (locus.getLocus (), list, leftExp);
	    }
	}


	Ymir::Tree InstUnrefMult (Word locus, InfoType t, Expression left, Expression right, ApplicationScore score) {
	    auto type = t-> cloneOnExitWithInfo ();
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);
	    
	    auto inner = left-> info-> type ()-> to<IRefInfo> ()-> content ()-> toGeneric ();

	    Ymir::TreeStmtList list;	    
	    auto leftExp = Ymir::getExpr (list, left);
	    leftExp = getPointerUnref (locus.getLocus (), leftExp, inner, 0);
	    

	    return Ymir::compoundExpr (locus.getLocus (), list, type-> buildMultOp (
		locus,
		type,
		new (Z0)  ITreeExpression (left-> token, left-> info-> type (), leftExp),
		right,
		score
	    ));	    
	}

    }
    
    IRefInfo::IRefInfo (bool) :
	IInfoType (false),
	_content (NULL)
    {}

    IRefInfo::IRefInfo (bool, InfoType content) :
	IInfoType (false),
	_content (content)
    {}
    
    bool IRefInfo::isSame (InfoType type) {
	if (auto ot = type-> to<IRefInfo> ()) {
	    return ot-> _content-> isSame (this-> _content);
	}
	return false;
    }

    InfoType IRefInfo::ConstVerif (InfoType type) {
	auto aux = type-> to <IRefInfo> ();
	if (aux) {
	    if ((this-> isConst () || this-> _content-> isConst ()) && (!aux-> _content-> isConst () && !aux-> isConst ()))
		return NULL;
		
	    auto content = this-> _content-> ConstVerif (aux-> _content);
	    if (content != NULL) {
		this-> _content = content;
		return this;
	    }		
	}
	return NULL;
    }

    bool IRefInfo::passingConst (InfoType other) {
	if (IInfoType::passingConst (other)) return true;
	else if (auto type = other-> to <IRefInfo> ()) {
	    return this-> _content-> passingConst (type-> _content);
	}
	return this-> _content-> passingConst (other);
    }
    
    InfoType IRefInfo::BinaryOp (Word token, syntax::Expression right) {
	InfoType aux = NULL, refRight = NULL;
	if (auto type = right-> info-> type ()-> to<IRefInfo> ()) {
	    aux = this-> _content-> BinaryOp (token, type-> _content);
	    refRight = type-> _content;	    
	} else aux = this-> _content-> BinaryOp (token, right);
	
	if (aux != NULL) {
	    if (refRight != NULL) aux = addUnrefDouble (aux);
	    else aux = addUnref (aux);
	    return aux;
	} else {
	    aux = right-> info-> type ()-> BinaryOpRight (token, this-> _content);
	    if (!aux) return NULL;
	    if (refRight != NULL) aux = addUnrefDouble (aux);
	    else aux = addUnref (aux);
	    return aux;
	}
	
	return NULL;
    }

    InfoType IRefInfo::BinaryOpRight (Word token, syntax::Expression left) {
	auto aux = this-> _content-> BinaryOpRight (token, left);
	if (aux != NULL) {
	    return addUnrefRight (aux);
	} else {
	    aux = left-> info-> type ()-> BinaryOp (token, this-> _content);    
	    if (aux != NULL) return addUnrefRight (aux);
	}
	return NULL;
    }

    InfoType IRefInfo::AccessOp (Word token, syntax::ParamList params, std::vector <InfoType> & treats) {
	auto aux = this-> _content-> AccessOp (token, params, treats);
	if (aux != NULL)
	    return addUnref (aux);
	return NULL;
    }

    InfoType IRefInfo::DotOp (syntax::Var var) {
	auto aux = this-> _content-> DotOp (var);
	if (aux != NULL)
	    return addUnref (aux);
	return NULL;
    }

    InfoType IRefInfo::DotExpOp (syntax::Expression elem) {
	auto aux = this-> _content-> DotExpOp (elem);
	if (aux != NULL)
	    return addUnref (aux);
	return NULL;
    }

    InfoType IRefInfo::DColonOp (syntax::Var var) {
	auto aux = this-> _content-> DColonOp (var);
	if (aux != NULL && var-> token != "typeinfo")
	    return addUnref (aux);
	else if (aux != NULL) return aux;
	return NULL;
    }

    InfoType IRefInfo::UnaryOp (Word op) {
	auto aux = this-> _content-> UnaryOp (op);
	if (aux) return addUnref (aux);
	return NULL;
    }

    InfoType IRefInfo::CastOp (InfoType other) {
	auto ptr = other-> to <IRefInfo> ();
	if (ptr && ptr-> _content-> isSame (this-> _content)) {
	    auto rf = this-> clone ();
	    rf-> binopFoo = &PtrUtils::InstAffect;
	    return rf;
	} else {
	    auto aux = this-> _content-> CastOp (other);
	    if (aux) return addUnref (aux);
	}
	return NULL;
    }

    InfoType IRefInfo::CompOp (InfoType other) {
	auto ptr = other-> to <IRefInfo> ();
	if (ptr && ptr-> _content-> isSame (this-> _content)) {
	    auto rf = this-> clone ();
	    rf-> binopFoo = &PtrUtils::InstCast;
	    return rf;
	} else {
	    auto aux = this-> _content-> CompOp (other);
	    if (aux) return addUnref (aux);
	}
	return NULL;
    }

    InfoType IRefInfo::ApplyOp (const std::vector <syntax::Var> & vars) {	
	auto ret = this-> _content-> ApplyOp (vars);
	if (ret) return addUnref (ret);
	return ret;
    }

    ApplicationScore IRefInfo::CallType (Word op, syntax::ParamList params) {
	auto ret = this-> _content-> CallType (op, params);
	return ret;
    }
    
    ApplicationScore IRefInfo::CallOp (Word op, syntax::ParamList params) {
	auto ret = this-> _content-> CallOp (op, params);
	if (ret && ret-> dyn) {
	    ret-> ret = addUnref (ret-> ret);
	}
	return ret;
    }

    bool IRefInfo::isConst () {
	return this-> _content-> isConst ();
    }

    void IRefInfo::isConst (bool set) {
	this-> _content-> isConst (set);
    }
        
    std::string IRefInfo::innerTypeString () {
	return std::string ("ref(") + this-> _content-> typeString () + ")";
    }

    std::string IRefInfo::innerSimpleTypeString () {
	return std::string ("R") + Mangler::mangle_type (this-> _content, this-> _content-> simpleTypeString ());
    }

    InfoType IRefInfo::onClone () {
	return new (Z0)  IRefInfo (this-> isConst (), this-> _content-> clone ());
    }

    const char* IRefInfo::getId () {
	return IRefInfo::id ();
    }

    InfoType IRefInfo::content () {
	return this-> _content;
    }

    Symbol& IRefInfo::symbol () {
	return this-> _content-> symbol ();
    }

    bool IRefInfo::isLvalue () {
	return this-> _content-> isLvalue ();
    }    
    
    Ymir::Tree IRefInfo::toGeneric () {
	if (this-> _content-> is <IStructInfo> ()) {
	    return build_pointer_type (
		void_type_node
	    );
	} else {
	    Ymir::Tree inner = this-> _content-> toGeneric ();
	    return build_pointer_type (
		inner.getTree ()
	    );
	}
    }

    Ymir::Tree IRefInfo::genericTypeInfo () {
	auto inner = this-> content ()-> genericTypeInfo ();

	auto type = Table::instance ().getTypeInfoType ()-> TempOp ({});
	auto typeTree = type-> toGeneric ();
	vec <constructor_elt, va_gc> * elms = NULL;
       	    
	auto ptr_info_type = Table::instance ().getTypeInfoType (Ymir::Runtime::REF_INFO)-> TempOp ({})-> to <IAggregateInfo> ();
	auto vtable = ptr_info_type-> getVtable ();
	    
	CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Keys::VTABLE_FIELD).getTree (), Ymir::getAddr (vtable).getTree ());	   
	CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Ymir::Runtime::VTABLE_FIELD_TYPEINFO).getTree (), build_int_cst_type (long_unsigned_type_node, 0));
	CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Ymir::Runtime::LEN_FIELD_TYPEINFO).getTree (), build_int_cst_type (long_unsigned_type_node, 0));
	CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Ymir::Runtime::C_O_A_TYPEINFO).getTree (), getAddr (inner).getTree ());
	
	auto name = Ymir::Runtime::TYPE_INFO_MODULE + "." + this-> innerSimpleTypeString () + Ymir::Runtime::TYPE_INFO_SUFFIX;
	auto glob = Ymir::declareGlobalWeak (name, typeTree, build_constructor (typeTree.getTree (), elms));

	return glob;
    }    
    
    InfoType IRefInfo::addUnref (InfoType elem) {
	bool binop = false, unop = false, mult = false;
	bool apply = false;
	if (elem-> binopFoo) {
	    elem-> nextBinop.push_back (elem-> binopFoo);
	    binop = true;
	}
	if (elem-> unopFoo) {
	    elem-> nextUnop.push_back (elem-> unopFoo);
	    unop = true;
	}

	if (elem-> multFoo) {
	    elem-> nextMult.push_back (elem-> multFoo);
	    mult = true;
	}

	if (elem-> applyFoo) {
	    elem-> nextApply.push_back (elem-> applyFoo);
	    apply = true;
	}
	
	if (binop) elem-> binopFoo = &RefUtils::InstUnrefBin;
	if (unop) elem-> unopFoo = &RefUtils::InstUnrefUn;
	if (mult) elem-> multFoo = &RefUtils::InstUnrefMult;
	if (apply) elem-> applyFoo = &RefUtils::InstUnrefApply;
	return elem;
    }
    
    InfoType IRefInfo::addUnrefDouble (InfoType elem) {
	bool binop = false, unop = false, mult = false;
	if (elem-> binopFoo) {
	    elem-> nextBinop.push_back (elem-> binopFoo);
	    binop = true;
	}
	if (elem-> unopFoo) {
	    elem-> nextUnop.push_back (elem-> unopFoo);
	    unop = true;
	}

	if (elem-> multFoo) {
	    elem-> nextMult.push_back (elem-> multFoo);
	    mult = true;
	}
	
	if (binop) elem-> binopFoo = &RefUtils::InstUnrefBinDouble;
	if (unop) elem-> unopFoo = &RefUtils::InstUnrefUn;
	if (mult) elem-> multFoo = &RefUtils::InstUnrefMult;
	return elem;
    }
    
    InfoType IRefInfo::addUnrefRight (InfoType elem) {
	bool binop = false, unop = false, mult = false;
	if (elem-> binopFoo) {
	    elem-> nextBinop.push_back (elem-> binopFoo);
	    binop = true;
	}
	if (elem-> unopFoo) {
	    elem-> nextUnop.push_back (elem-> unopFoo);
	    unop = true;
	}

	if (elem-> multFoo) {
	    elem-> nextMult.push_back (elem-> multFoo);
	    mult = true;
	}
	
	if (binop) elem-> binopFoo = &RefUtils::InstUnrefBinRight;
	if (unop) elem-> unopFoo = &RefUtils::InstUnrefUn;
	if (mult) elem-> multFoo = &RefUtils::InstUnrefMult;
	return elem;
    }

    
    IArrayRefInfo::IArrayRefInfo (bool isConst) :
	IInfoType (isConst),
	_content (NULL)
    {}

    IArrayRefInfo::IArrayRefInfo (bool isConst, InfoType content) :
	IInfoType (isConst),
	_content (content)
    {}

    Value & IArrayRefInfo::value () {
	return this-> _content-> value ();
    }
    
    InfoType IArrayRefInfo::getIntern () {
	return this-> _content;
    }
    
    bool IArrayRefInfo::isSame (InfoType type) {
	return this-> _content-> isSame (type);	
    }

    InfoType IArrayRefInfo::ConstVerif (InfoType type) {
	return this-> _content-> ConstVerif (type);	
    }

    bool IArrayRefInfo::passingConst (InfoType type) {
	return this-> _content-> passingConst (type);
    }
    
    InfoType IArrayRefInfo::BinaryOp (Word token, syntax::Expression right) {
	if (auto type = right-> info-> type ()-> to <IArrayRefInfo> ()) {
	    return this-> _content-> BinaryOp (token, type-> _content);
	} else 
	    return this-> _content-> BinaryOp (token, right);
    }

    InfoType IArrayRefInfo::BinaryOpRight (Word token, syntax::Expression left) {
	auto aux = this-> _content-> BinaryOpRight (token, left);
	if (aux != NULL) return aux;
	else 
	    return left-> info-> type ()-> BinaryOp (token, this-> _content);	
    }

    InfoType IArrayRefInfo::AccessOp (Word token, syntax::ParamList params, std::vector <InfoType> & treats) {
	return this-> _content-> AccessOp (token, params, treats);
    }

    InfoType IArrayRefInfo::DotOp (syntax::Var var) {
	return this-> _content-> DotOp (var);
    }

    InfoType IArrayRefInfo::DotExpOp (syntax::Expression elem) {
	return this-> _content-> DotExpOp (elem);
    }

    InfoType IArrayRefInfo::DColonOp (syntax::Var var) {
	return this-> _content-> DColonOp (var);
    }

    InfoType IArrayRefInfo::UnaryOp (Word op) {
	return this-> _content-> UnaryOp (op);
    }

    InfoType IArrayRefInfo::CastOp (InfoType other) {
	return this-> _content-> CastOp (other);
    }

    InfoType IArrayRefInfo::CompOp (InfoType other) {
	return this-> _content-> CompOp (other);
    }

    InfoType IArrayRefInfo::ApplyOp (const std::vector <syntax::Var> &vars) {
	return this-> _content-> ApplyOp (vars);
    }

    ApplicationScore IArrayRefInfo::CallType (Word op, syntax::ParamList params) {
	return this-> _content-> CallType (op, params);
    }
    
    ApplicationScore IArrayRefInfo::CallOp (Word op, syntax::ParamList params) {
	return this-> _content-> CallOp (op, params);
    }

    std::string IArrayRefInfo::typeString () {
	return this-> _content-> typeString ();
    }
    
    std::string IArrayRefInfo::innerTypeString () {
	return this-> _content-> innerTypeString ();
    }

    std::string IArrayRefInfo::simpleTypeString () {
	return this-> _content-> simpleTypeString ();
    }
    
    std::string IArrayRefInfo::innerSimpleTypeString () {
	return this-> _content-> innerSimpleTypeString ();
    }

    InfoType IArrayRefInfo::onClone () {
	return new (Z0) IArrayRefInfo (this-> isConst (), this-> _content-> clone ());
    }

    const char* IArrayRefInfo::getId () {
	return IArrayRefInfo::id ();
    }

    InfoType IArrayRefInfo::content () {
	return this-> _content;
    }

    InfoType IArrayRefInfo::getTemplate (ulong nb) {
	return this-> _content-> getTemplate (nb);
    }
    
    Ymir::Tree IArrayRefInfo::toGeneric () {
	return this-> _content-> toGeneric ();
    }

    Symbol& IArrayRefInfo::symbol () {
	return this-> _content-> symbol ();
    }

    bool IArrayRefInfo::isLvalue () {
	return this-> _content-> isLvalue ();
    }
    
    
}
