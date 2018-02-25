#include <ymir/semantic/types/_.hh>
#include <ymir/ast/TreeExpression.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/utils/PtrUtils.hh>


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
	
	Ymir::Tree InstUnrefBinRight (Word locus, InfoType type, Expression left, Expression right) {
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);

	    auto innerType = right-> info-> type-> to<IRefInfo> ()-> content ();
	    auto inner = innerType-> toGeneric ();	    
	    auto rightExp = right-> toGeneric ();
	    rightExp = getPointerUnref (locus.getLocus (), rightExp, inner, 0);
	    
	    if (type-> binopFoo) {
		return type-> buildBinaryOp (
		    locus,
		    type,
		    left,
		    new (Z0)  ITreeExpression (right-> token, innerType, rightExp)
		);
	    } else {
		return type-> buildMultOp (
		    locus,
		    type,
		    left,
		    new (Z0)  ITreeExpression (right-> token, innerType, rightExp)
		);
	    }	    
	    
	}
		
	Ymir::Tree InstUnrefBin (Word locus, InfoType type, Expression left, Expression right) {
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);

	    auto innerType = left-> info-> type-> to<IRefInfo> ()-> content ();
	    auto inner = innerType-> toGeneric ();	    
	    auto leftExp = left-> toGeneric ();
	    leftExp = getPointerUnref (locus.getLocus (), leftExp, inner, 0);
	    
	    if (type-> binopFoo) {
		return type-> buildBinaryOp (
		    locus,
		    type,
		    new (Z0)  ITreeExpression (left-> token, innerType, leftExp),
		    right
		);
	    } else if (type-> multFoo) {
		return type-> buildMultOp (
		    locus,
		    type,
		    new (Z0)  ITreeExpression (left-> token, innerType, leftExp),
		    right
		);
	    } else if (type-> unopFoo) {
		return type-> buildUnaryOp (
		    locus,
		    type,
		    new (Z0)  ITreeExpression (left-> token, innerType, leftExp)
		);
	    }
	    return leftExp;
	}

	Ymir::Tree InstUnrefBinDouble (Word locus, InfoType type, Expression left, Expression right) {
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);

	    auto innerLeft = left-> info-> type-> to<IRefInfo> ()-> content ();
	    auto inner = innerLeft-> toGeneric ();
	    auto leftExp = left-> toGeneric ();
	    leftExp = getPointerUnref (locus.getLocus (), leftExp, inner, 0);

	    auto innerRight = right-> info-> type-> to<IRefInfo> ()-> content ();
	    inner = innerRight-> toGeneric ();
	    auto rightExp = right-> toGeneric ();
	    rightExp = getPointerUnref (locus.getLocus (), rightExp, inner, 0);
	    
	    if (type-> binopFoo) {
		return type-> buildBinaryOp (
		    locus,
		    type,
		    new (Z0)  ITreeExpression (left-> token, innerLeft, leftExp),
		    new (Z0)  ITreeExpression (right-> token, innerRight, rightExp)
		);
	    } else {
		return type-> buildMultOp (
		    locus,
		    type,
		    new (Z0)  ITreeExpression (left-> token, innerLeft, leftExp),
		    new (Z0)  ITreeExpression (right-> token, innerRight, rightExp)
		);
	    }	    
	}
	
	Ymir::Tree InstUnrefUn (Word locus, InfoType type, Expression left) {
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);
	    
	    auto inner = left-> info-> type-> to<IRefInfo> ()-> content ()-> toGeneric ();
	    
	    auto leftExp = left-> toGeneric ();
	    leftExp = getPointerUnref (locus.getLocus (), leftExp, inner, 0);

	    if (type-> unopFoo) {
		return type-> buildUnaryOp (
		    locus,
		    type,
		    new (Z0)  ITreeExpression (left-> token, left-> info-> type, leftExp)
		);
	    } else if (type-> binopFoo) {
		return type-> buildBinaryOp (
		    locus,
		    type,
		    new (Z0)  ITreeExpression (left-> token, left-> info-> type, leftExp),
		    new (Z0)  ITreeExpression (locus, type, Ymir::Tree ())
		);
	    } else {
		return leftExp;
	    }
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
	    auto content = this-> _content-> ConstVerif (aux-> _content);
	    if (content != NULL) {
		this-> _content = content;
		return this;
	    }		
	}
	return NULL;
    }

    InfoType IRefInfo::BinaryOp (Word token, syntax::Expression right) {
	InfoType aux = NULL, refRight = NULL;
	if (auto type = right-> info-> type-> to<IRefInfo> ()) {
	    aux = this-> _content-> BinaryOp (token, type-> _content);
	    refRight = type-> _content;	    
	} else aux = this-> _content-> BinaryOp (token, right);
	if (aux != NULL) {
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
	    aux = left-> info-> type-> BinaryOp (token, this-> _content);    
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
	if (aux != NULL)
	    return addUnref (aux);
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

    // InfoType IRefInfo::ApplyOp (std::vector <syntax::Var> vars) {	
    // }

    ApplicationScore IRefInfo::CallOp (Word op, syntax::ParamList params) {
	auto ret = this-> _content-> CallOp (op, params);
	if (ret && ret-> dyn) {
	    ret-> left = addUnref (this-> _content-> cloneOnExit ());
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
	return std::string ("ref(") + this-> _content-> innerTypeString () + ")";
    }

    std::string IRefInfo::innerSimpleTypeString () {
	return std::string ("R") + this-> _content-> simpleTypeString ();
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
    
    InfoType IRefInfo::addUnref (InfoType elem) {
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
	
	if (binop) elem-> binopFoo = &RefUtils::InstUnrefBin;
	if (unop) elem-> unopFoo = &RefUtils::InstUnrefUn;
	if (mult) elem-> multFoo = &RefUtils::InstUnrefBin;
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
	if (mult) elem-> multFoo = &RefUtils::InstUnrefBinDouble;
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
	if (mult) elem-> multFoo = &RefUtils::InstUnrefBinRight;
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

    InfoType IArrayRefInfo::getIntern () {
	return this-> _content;
    }
    
    bool IArrayRefInfo::isSame (InfoType type) {
	return this-> _content-> isSame (type);	
    }

    InfoType IArrayRefInfo::ConstVerif (InfoType type) {
	return this-> _content-> ConstVerif (type);	
    }

    InfoType IArrayRefInfo::BinaryOp (Word token, syntax::Expression right) {
	if (auto type = right-> info-> type-> to <IArrayRefInfo> ()) {
	    return this-> _content-> BinaryOp (token, type-> _content);
	} else 
	    return this-> _content-> BinaryOp (token, right);
    }

    InfoType IArrayRefInfo::BinaryOpRight (Word token, syntax::Expression left) {
	auto aux = this-> _content-> BinaryOpRight (token, left);
	if (aux != NULL) return aux;
	else 
	    return left-> info-> type-> BinaryOp (token, this-> _content);	
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

    //InfoType IArrayRefInfo::ApplyOp (std::vector <syntax::Var> vars) {}

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


    
}
