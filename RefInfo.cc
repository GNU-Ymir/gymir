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
	    auto last = list.back ();
	    list.pop_back ();
	    return last;
	}

	Ymir::Tree InstUnrefBinRight (Word locus, InfoType type, Expression left, Expression right) {
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);
	    
	    auto inner = right-> info-> type-> to<IRefInfo> ()-> content ()-> toGeneric ();	    
	    auto rightExp = right-> toGeneric ();
	    rightExp = getPointerUnref (locus.getLocus (), rightExp, inner, 0);
	    
	    if (type-> binopFoo) {
		return type-> buildBinaryOp (
		    locus,
		    type,
		    left,
		    new (GC) ITreeExpression (right-> token, right-> info-> type, rightExp)
		);
	    } else {
		return type-> buildMultOp (
		    locus,
		    type,
		    left,
		    new (GC) ITreeExpression (right-> token, right-> info-> type, rightExp)
		);
	    }	    
	    
	}
		
	Ymir::Tree InstUnrefBin (Word locus, InfoType type, Expression left, Expression right) {
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);

	    auto inner = left-> info-> type-> to<IRefInfo> ()-> content ()-> toGeneric ();
	    
	    auto leftExp = left-> toGeneric ();
	    leftExp = getPointerUnref (locus.getLocus (), leftExp, inner, 0);
	    
	    if (type-> binopFoo) {
		return type-> buildBinaryOp (
		    locus,
		    type,
		    new (GC) ITreeExpression (left-> token, left-> info-> type, leftExp),
		    right
		);
	    } else if (type-> multFoo) {
		return type-> buildMultOp (
		    locus,
		    type,
		    new (GC) ITreeExpression (left-> token, left-> info-> type, leftExp),
		    right
		);
	    } else if (type-> unopFoo) {
		return type-> buildUnaryOp (
		    locus,
		    type,
		    new (GC) ITreeExpression (left-> token, left-> info-> type, leftExp)
		);
	    }
	    return leftExp;
	}

	Ymir::Tree InstUnrefBinDouble (Word locus, InfoType type, Expression left, Expression right) {
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);

	    auto inner = left-> info-> type-> to<IRefInfo> ()-> content ()-> toGeneric ();
	    auto leftExp = left-> toGeneric ();
	    leftExp = getPointerUnref (locus.getLocus (), leftExp, inner, 0);

	    inner = right-> info-> type-> to<IRefInfo> ()-> content ()-> toGeneric ();
	    auto rightExp = right-> toGeneric ();
	    rightExp = getPointerUnref (locus.getLocus (), rightExp, inner, 0);
	    
	    if (type-> binopFoo) {
		return type-> buildBinaryOp (
		    locus,
		    type,
		    new (GC) ITreeExpression (left-> token, left-> info-> type, leftExp),
		    new (GC) ITreeExpression (right-> token, right-> info-> type, rightExp)
		);
	    } else {
		return type-> buildMultOp (
		    locus,
		    type,
		    new (GC) ITreeExpression (left-> token, left-> info-> type, leftExp),
		    new (GC) ITreeExpression (right-> token, right-> info-> type, rightExp)
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
		    new (GC) ITreeExpression (left-> token, left-> info-> type, leftExp)
		);
	    } else if (type-> binopFoo) {
		return type-> buildBinaryOp (
		    locus,
		    type,
		    new (GC) ITreeExpression (left-> token, left-> info-> type, leftExp),
		    new (GC) ITreeExpression (locus, type, Ymir::Tree ())
		);
	    } else {
		return leftExp;
	    }
	}
	
    }
    
    IRefInfo::IRefInfo (bool isConst) :
	IInfoType (isConst),
	_content (NULL)
    {}

    IRefInfo::IRefInfo (bool isConst, InfoType content) :
	IInfoType (isConst),
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
	if (other-> is <IUndefInfo> () || (ptr && ptr-> _content-> isSame (this-> _content))) {
	    auto rf = this-> clone ();
	    rf-> binopFoo = &PtrUtils::InstAffect;
	    return rf;
	} else {
	    auto aux = this-> _content-> CastOp (other);
	    if (aux) return addUnref (aux);
	}
	return NULL;
    }

    //InfoType IRefInfo::ApplyOp (std::vector <syntax::Var> vars) {}

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

    std::string IRefInfo::simpleTypeString () {
	return std::string ("R") + this-> _content-> simpleTypeString ();
    }

    InfoType IRefInfo::clone () {
	return new IRefInfo (this-> isConst (), this-> _content-> clone ());
    }

    const char* IRefInfo::getId () {
	return IRefInfo::id ();
    }

    InfoType IRefInfo::content () {
	return this-> _content;
    }

    Ymir::Tree IRefInfo::toGeneric () {
	Ymir::Tree inner = this-> _content-> toGeneric ();
	return build_pointer_type (
	    inner.getTree ()
	);
    }
    
    InfoType IRefInfo::addUnref (InfoType elem) {
	elem-> nextBinop.push_back (elem-> binopFoo);
	elem-> nextUnop.push_back (elem-> unopFoo);
	elem-> nextMult.push_back (elem-> multFoo);

	elem-> binopFoo = &RefUtils::InstUnrefBin;
	elem-> unopFoo = &RefUtils::InstUnrefUn;
	elem-> multFoo = &RefUtils::InstUnrefBin;
	return elem;
    }
    
    InfoType IRefInfo::addUnrefDouble (InfoType elem) {
	elem-> nextBinop.push_back (elem-> binopFoo);
	elem-> nextUnop.push_back (elem-> unopFoo);
	elem-> nextMult.push_back (elem-> multFoo);

	elem-> binopFoo = &RefUtils::InstUnrefBinDouble;
	elem-> unopFoo = &RefUtils::InstUnrefUn;
	elem-> multFoo = &RefUtils::InstUnrefBinDouble;
	return elem;
    }
    
    InfoType IRefInfo::addUnrefRight (InfoType elem) {
	elem-> nextBinop.push_back (elem-> binopFoo);
	elem-> nextUnop.push_back (elem-> unopFoo);
	elem-> nextMult.push_back (elem-> multFoo);

	elem-> binopFoo = &RefUtils::InstUnrefBinRight;
	elem-> unopFoo = &RefUtils::InstUnrefUn;
	elem-> multFoo = &RefUtils::InstUnrefBinRight;
	return elem;
    }

    
    
}
