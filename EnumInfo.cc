#include <ymir/semantic/types/_.hh>
#include <ymir/errors/Error.hh>
#include <ymir/ast/TreeExpression.hh>
#include <ymir/utils/Mangler.hh>

namespace semantic {

    using namespace syntax;

    namespace EnumUtils {

	using namespace Ymir;

	template <typename T>
	T getAndRemoveBack (std::list <T> &list) {
	    auto last = list.back ();
	    list.pop_back ();
	    return last;
	}
	
	Tree InstGet (Word locus, InfoType type, Expression elem) {
	    EnumInfo ecst = type-> realTo <IEnumInfo> ();
	    if (ecst-> getValue () == NULL) ecst-> getValue () = elem;
	    if (ecst-> getComp () != NULL) {
		if (ecst-> getComp ()-> unopFoo) {
		    return ecst-> getComp ()-> buildUnaryOp (
			locus,
			ecst-> getComp () ,
			ecst-> getValue ()
		    );
		} else {
		    return ecst-> getComp ()-> buildBinaryOp (
			locus,
			ecst-> getComp (),
			ecst-> getValue (),
			new (Z0) ITreeExpression (locus, ecst-> getComp (), Ymir::Tree ())
		    );
		}
	    } else {
		return ecst-> getValue ()-> toGeneric ();
	    }
	}

	Tree InstGet (Word locus, InfoType type, Expression elem, Expression) {
	    EnumInfo ecst = type-> realTo <IEnumInfo> ();
	    if (ecst-> getValue () == NULL) ecst-> getValue () = elem;
	    if (ecst-> getComp () != NULL) {
		if (ecst-> getComp ()-> unopFoo) {
		    return ecst-> getComp ()-> buildUnaryOp (
			locus,
			ecst-> getComp (),
			ecst-> getValue ()
		    );
		} else {
		    return ecst-> getComp ()-> buildBinaryOp (
			locus,
			ecst-> getComp (),
			ecst-> getValue (),
			new (Z0) ITreeExpression (locus, ecst-> getComp (), Ymir::Tree ())
		    );
		}
	    } else {
		return ecst-> getValue ()-> toGeneric ();
	    }
	}

	Tree InstUnrefBin (Word locus, InfoType type, Expression left, Expression right) {
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);

	    InfoType inner = type;
	    EnumInfo ecst = left-> info-> type-> to <IEnumInfo> ();
	    if (ecst) type = ecst-> getContent ();

	    inner-> binopFoo = type-> binopFoo;
	    inner-> unopFoo = type-> unopFoo;
	    inner-> multFoo = type-> multFoo;
	    
	    auto linfo = left-> info-> type-> to <IEnumInfo> ();
	    left-> info-> type = linfo-> content ();
	    left-> info-> value () = NULL;

	    left-> info-> type-> binopFoo = linfo-> binopFoo;
	    left-> info-> type-> unopFoo = linfo-> unopFoo;
	    left-> info-> type-> multFoo = linfo-> multFoo;
	    
	    if (type-> binopFoo) {
		return type-> buildBinaryOp (
		    locus, inner, left, right
		);
	    } else {
		return type-> buildMultOp (
		    locus, inner, left, right
		);
	    }
	}

	Tree InstUnrefBinRight (Word locus, InfoType type, Expression left, Expression right) {
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);

	    InfoType inner = type;
	    EnumInfo ecst = left-> info-> type-> to <IEnumInfo> ();
	    if (ecst) inner = ecst-> getContent ();
	    
	    auto rinfo = right-> info-> type-> to <IEnumInfo> ();
	    right-> info-> type = rinfo-> content ();
	    right-> info-> value () = NULL;

	    right-> info-> type-> binopFoo = rinfo-> binopFoo;
	    right-> info-> type-> unopFoo = rinfo-> unopFoo;
	    right-> info-> type-> multFoo = rinfo-> multFoo;
	    
	    if (type-> binopFoo) {
		return type-> buildBinaryOp (
		    locus, inner, left, right
		);
	    } else {
		return type-> buildMultOp (
		    locus, inner, left, right
		);
	    }
	}

	Tree InstUnrefBinDouble (Word locus, InfoType type, Expression left, Expression right) {
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);

	    InfoType inner = type;
	    EnumInfo ecst = left-> info-> type-> to <IEnumInfo> ();
	    if (ecst) inner = ecst-> getContent ();

	    auto linfo = left-> info-> type-> to <IEnumInfo> ();
	    left-> info-> type = linfo-> content ();
	    left-> info-> value () = NULL;
	    
	    auto rinfo = right-> info-> type-> to <IEnumInfo> ();
	    right-> info-> type = rinfo-> content ();
	    right-> info-> value () = NULL;

	    left-> info-> type-> binopFoo = linfo-> binopFoo;
	    left-> info-> type-> unopFoo = linfo-> unopFoo;
	    left-> info-> type-> multFoo = linfo-> multFoo;
	    
	    right-> info-> type-> binopFoo = rinfo-> binopFoo;
	    right-> info-> type-> unopFoo = rinfo-> unopFoo;
	    right-> info-> type-> multFoo = rinfo-> multFoo;
	    if (type-> binopFoo) {
		return type-> buildBinaryOp (
		    locus, inner, left, right
		);
	    } else {
		return type-> buildMultOp (
		    locus, inner, left, right
		);
	    }
	}

	Tree InstUnrefUn (Word locus, InfoType type, Expression left) {
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);

	    InfoType inner = type;
	    EnumInfo ecst = left-> info-> type-> to <IEnumInfo> ();
	    if (ecst) type = ecst-> getContent ();

	    auto linfo = left-> info-> type-> to <IEnumInfo> ();
	    left-> info-> type = linfo-> content ();
	    left-> info-> value () = NULL;

	    left-> info-> type-> binopFoo = linfo-> binopFoo;
	    left-> info-> type-> unopFoo = linfo-> unopFoo;
	    left-> info-> type-> multFoo = linfo-> multFoo;
	    
	    if (type-> unopFoo) {
		return type-> buildUnaryOp (
		    locus, inner, left
		);
	    } else {
		return type-> buildBinaryOp (
		    locus, inner, left, new (Z0) ITreeExpression (locus, type, Ymir::Tree ())
		);
	    }
	}
	
	
    }
    
    IEnumCstInfo::IEnumCstInfo (Namespace space, std::string name, InfoType type) :
	IInfoType (true),
	_name (name),
	_space (space),
	type (type)
    {
	this-> _isType = true;
    }

    std::string IEnumCstInfo::name () {
	return this-> _name;
    }

    void IEnumCstInfo::addAttrib (std::string name, Expression value, InfoType comp) {
	this-> names.push_back (name);
	this-> values.push_back (value);
	this-> comps.push_back (comp);
    }

    InfoType IEnumCstInfo::DColonOp (syntax::Var var) {
	ulong i = 0;
	if (var-> hasTemplate ()) return NULL;
	for (auto it : this-> names) {
	    if (var-> token == it)
		return GetAttrib (i);
	    i++;
	}
	return NULL;
    }

    InfoType IEnumCstInfo::DotOp (syntax::Var) {
	// if (var-> hasTemplate ()) return NULL;
	// if (var-> token == "member") {
	//     auto info = new (Z0)  IArrayInfo (true, new (Z0)  IEnumInfo (true, this-> _name, this-> type-> clone ()));
	//     //TODO
	//     return info;
	// } else return NULL;
	return NULL;
    }

    InfoType IEnumCstInfo::create () {
	return new (Z0) IEnumInfo (false, this-> _space, this-> _name, this-> type-> cloneOnExit ());
    }

    InfoType IEnumCstInfo::TempOp (const std::vector<::syntax::Expression> & tmps) {
	if (tmps.size () != 0) return NULL;
	return create ();
    }
    
    std::string IEnumCstInfo::innerSimpleTypeString () {
	return Ymir::format ("%%", Mangler::mangle_type (this-> _space.toString () + "." + this-> _name), "E");
    }

    std::string IEnumCstInfo::innerTypeString () {
	if (this-> type) {
	    return Ymir::format ("%.%(%)", this-> _space.toString (), this-> _name.c_str (), this-> type-> innerTypeString ());
	} else {
	    return Ymir::format ("%.%(...)", this-> _space.toString (), this-> _name.c_str ());
	}
    }

    bool IEnumCstInfo::isSame (InfoType ot) {
	if (auto en = ot-> to<IEnumCstInfo> ()) {
	    return en-> _name == this-> _name && en-> _space == this-> _space;
	}
	return false;
    }

    std::vector <std::string> & IEnumCstInfo::getNames () {
	return this-> names;
    }

    std::vector <syntax::Expression> & IEnumCstInfo::getValues () {
	return this-> values;
    }

    std::vector <InfoType> & IEnumCstInfo::getComp () {
	return this-> comps;
    }    
    
    InfoType IEnumCstInfo::onClone () {
	return this;
    }

    const char* IEnumCstInfo::getId () {
	return IEnumCstInfo::id ();
    }

    InfoType IEnumCstInfo::GetAttrib (ulong nb) {
	auto type = new (Z0)  IEnumInfo (true, this-> _space, this-> _name, this-> type-> clone ());
	if (this-> values [nb]-> info-> isImmutable ()) {
	    type-> getContent ()-> value () = this-> values [nb]-> info-> value ();
	}
	//type-> toGet () = nb;
	type-> unopFoo = EnumUtils::InstGet;
	type-> getValue () = this-> values [nb];
	type-> getComp () = this-> comps [nb];
	return type;
    }

    IEnumInfo::IEnumInfo (bool isConst, Namespace space, std::string name, InfoType type) :
	IInfoType (isConst),
	_name (name),
	_space (space),
	_content (type)
    {
	this-> _content-> isConst (this-> isConst ());
    }

    std::string& IEnumInfo::name () {
	return this-> _name;
    }

    InfoType & IEnumInfo::getComp () {
	return this-> comp;
    }

    Expression & IEnumInfo::getValue () {
	return this-> value;
    }

    InfoType IEnumInfo::BinaryOp (Word op, syntax::Expression right) {
	InfoType aux = NULL, refRight = NULL;	
	if (auto type = right-> info-> type-> to<IEnumInfo> ()) {
	    aux = this-> _content-> BinaryOp (op, type-> _content);
	    refRight = type-> _content;
	} else aux = this-> _content-> BinaryOp (op, right);
	
	if (aux != NULL) {
	    if (refRight != NULL) aux = addUnrefDouble (aux);
	    else aux = addUnref (aux);
	    return aux;
	}
	return NULL;
    }

    InfoType IEnumInfo::BinaryOpRight (Word op, syntax::Expression left) {
	if (left-> info-> type-> is<IUndefInfo> ()) {
	    return this-> _content-> BinaryOpRight (op, left);
	}
	
	auto aux = this-> _content-> BinaryOpRight (op, left);
	if (aux != NULL) {
	    return addUnrefRight (aux);
	} else {
	    aux = left-> info-> type-> BinaryOp (op, this-> _content);
	    if (aux != NULL) return addUnrefRight (aux);	    
	}
	return NULL;	
    }

    InfoType IEnumInfo::AccessOp (Word op, syntax::ParamList params, std::vector <InfoType> & treats) {
	auto ret = this-> _content-> AccessOp (op, params, treats);
	if (ret) return addUnref (ret);
	return NULL;
    }

    InfoType IEnumInfo::DotOp (syntax::Var var) {
	auto ret = this-> _content-> DotOp (var);
	if (ret) return addUnref (ret);
	return NULL;
    }

    InfoType IEnumInfo::DotExpOp (syntax::Expression var) {
	auto ret = this-> _content-> DotExpOp (var);
	if (ret) return addUnref (ret);
	return NULL;
    }

    InfoType IEnumInfo::DColonOp (syntax::Var var) {
	auto ret = this-> _content-> DColonOp (var);
	if (ret) return addUnref (ret);
	return NULL;
    }

    InfoType IEnumInfo::UnaryOp (Word op) {
	auto ret = this-> _content-> UnaryOp (op);
	if (ret) return addUnref (ret);
	return NULL;
    }

    InfoType IEnumInfo::CompOp (InfoType other) {
	if (other-> is<IUndefInfo> () || this-> isSame (other)) {
	    auto rf = this-> clone ()-> to <IEnumInfo> ();
	    auto ret = this-> _content-> CompOp (this-> _content);	    
	    rf-> comp = ret;
	    rf-> binopFoo = EnumUtils::InstGet;
	    return rf;
	} else {
	    auto ret = this-> _content-> CompOp (other);
	    if (ret) return addUnref (ret);
	    return ret;
	}
    }

    InfoType IEnumInfo::CastOp (InfoType other) {
	auto ret = this-> _content-> CastOp (other);
	if (ret) return addUnref (ret);
	return NULL;
    }

    ApplicationScore IEnumInfo::CallOp (Word op, syntax::ParamList params) {
	auto ret = this-> _content-> CallOp (op, params);
	if (ret && ret-> dyn) {
	    ret-> left = addUnref (this-> _content-> cloneOnExit ());
	}
	return ret;
    }
    
    InfoType IEnumInfo::addUnref (InfoType elem) {
	elem-> nextBinop.push_back (elem-> binopFoo);
	elem-> nextUnop.push_back (elem-> unopFoo);
	elem-> nextMult.push_back (elem-> multFoo);

	elem-> binopFoo = &EnumUtils::InstUnrefBin;
	elem-> unopFoo = &EnumUtils::InstUnrefUn;
	elem-> multFoo = &EnumUtils::InstUnrefBin;
	return elem;
    }
    
    InfoType IEnumInfo::addUnrefDouble (InfoType elem) {
	elem-> nextBinop.push_back (elem-> binopFoo);
	elem-> nextUnop.push_back (elem-> unopFoo);
	elem-> nextMult.push_back (elem-> multFoo);

	elem-> binopFoo = &EnumUtils::InstUnrefBinDouble;
	elem-> unopFoo = &EnumUtils::InstUnrefUn;
	elem-> multFoo = &EnumUtils::InstUnrefBinDouble;
	return elem;
    }
    
    InfoType IEnumInfo::addUnrefRight (InfoType elem) {
	elem-> nextBinop.push_back (elem-> binopFoo);
	elem-> nextUnop.push_back (elem-> unopFoo);
	elem-> nextMult.push_back (elem-> multFoo);

	elem-> binopFoo = &EnumUtils::InstUnrefBinRight;
	elem-> unopFoo = &EnumUtils::InstUnrefUn;
	elem-> multFoo = &EnumUtils::InstUnrefBinRight;
	return elem;
    }    
    
    InfoType & IEnumInfo::getContent () {
	return this-> _content;
    }

    InfoType IEnumInfo::getIntern () {
	return this-> _content;
    }
    
    std::string IEnumInfo::innerSimpleTypeString () {
	return Ymir::format ("%%", Mangler::mangle_type (this-> _space.toString () + "." +this-> _name), "E");
    }

    std::string IEnumInfo::innerTypeString () {
	return Ymir::format ("%.%(%)", this-> _space.toString (), this-> _name.c_str (), this-> _content-> innerTypeString ());
    }

    const char* IEnumInfo::getId () {
	return IEnumInfo::id ();
    }

    bool IEnumInfo::isSame (InfoType other) {
	if (auto en = other-> to<IEnumInfo> ()) {
	    return (en-> _space == this-> _space && en-> _name == this-> _name && this-> _content-> isSame (en-> _content));
	} else
	    return this-> _content-> isSame (other);
	return false;
    }

    InfoType IEnumInfo::getTemplate (ulong i) {
	return this-> _content-> getTemplate (i);
    }
    
    InfoType IEnumInfo::onClone () {
	auto ret = new (Z0)  IEnumInfo (this-> isConst (), this-> _space, this-> _name, this-> _content-> clone ());
	ret-> value = this-> value;
	return ret;
    }

    Ymir::Tree IEnumInfo::toGeneric () {
	return this-> _content-> toGeneric ();
    }
    
    InfoType IEnumInfo::content () {
	return this-> _content;
    }
        
}
    
