#include <ymir/semantic/types/_.hh>
#include <ymir/ast/TreeExpression.hh>
#include <ymir/semantic/tree/Generic.hh>

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
		
	Ymir::Tree InstUnrefBin (Word locus, InfoType type, Expression left, Expression right) {
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);

	    auto inner = type-> toGeneric ();
	    
	    auto leftExp = left-> toGeneric ();
	    leftExp = getPointerUnref (locus.getLocus (), leftExp, inner, 0);
	    if (type-> binopFoo) {
		return type-> buildBinaryOp (
		    locus,
		    type,
		    new (GC) ITreeExpression (left-> token, left-> info-> type, leftExp),
		    right
		);
	    } else {
		return type-> buildMultOp (
		    locus,
		    type,
		    new (GC) ITreeExpression (left-> token, left-> info-> type, leftExp),
		    right
		);
	    }	    
	}

	Ymir::Tree InstUnrefUn (Word locus, InfoType type, Expression left) {
	    left-> info-> type-> binopFoo = getAndRemoveBack (left-> info-> type-> nextBinop);
	    auto store = getAndRemoveBack (left-> info-> type-> nextUnop);
	    if (!left-> info-> type-> nextUnop.empty ())
		left-> info-> type-> unopFoo = getAndRemoveBack (left-> info-> type-> nextUnop);
	    else left-> info-> type-> unopFoo = NULL;
	    
	    left-> info-> type-> multFoo = getAndRemoveBack (left-> info-> type-> nextMult);

	    auto inner = type-> toGeneric ();
	    
	    auto leftExp = left-> toGeneric ();
	    leftExp = getPointerUnref (locus.getLocus (), leftExp, inner, 0);
	    if (store) {
		return store (
		    locus,
		    type,
		    new (GC) ITreeExpression (left-> token, left-> info-> type, Ymir::Tree ())
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
	    aux = addUnref (aux);
	    if (refRight != NULL) aux = addUnref (aux);
	    return aux;
	}
	return NULL;
    }

    InfoType IRefInfo::BinaryOpRight (Word token, syntax::Expression left) {
	return NULL;
    }

    InfoType IRefInfo::AccessOp (Word token, syntax::ParamList params) {
	return NULL;
    }

    InfoType IRefInfo::DotOp (syntax::Var var) {
	return NULL;
    }

    InfoType IRefInfo::DotExpOp (syntax::Expression elem) {
	return NULL;
    }

    InfoType IRefInfo::DColonOp (syntax::Var var) {
	return NULL;
    }

    InfoType IRefInfo::UnaryOp (Word op) {
	auto aux = this-> _content-> UnaryOp (op);
	if (aux) return addUnref (aux);
	return NULL;
    }

    InfoType IRefInfo::CastOp (InfoType other) {}

    InfoType IRefInfo::CompOp (InfoType other) {}

    //InfoType IRefInfo::ApplyOp (std::vector <syntax::Var> vars) {}

    ApplicationScore IRefInfo::CallOp (Word op, syntax::ParamList params) {
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

    
    
}
