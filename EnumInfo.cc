#include <ymir/semantic/types/_.hh>
#include <ymir/errors/Error.hh>
#include <ymir/ast/TreeExpression.hh>
#include <ymir/utils/Mangler.hh>

namespace semantic {

    using namespace syntax;

    namespace EnumUtils {

	using namespace Ymir;
	
	Tree InstGet (Word locus, InfoType type, Expression elem) {
	    EnumInfo ecst = type-> to <IEnumInfo> ();
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
	    EnumInfo ecst = type-> to <IEnumInfo> ();
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
	if (auto type = right-> info-> type-> to<IEnumInfo> ()) {
	    return this-> _content-> BinaryOp (op, type-> _content);
	} else return this-> _content-> BinaryOp (op, right);
    }

    InfoType IEnumInfo::BinaryOpRight (Word op, syntax::Expression left) {
	if (left-> info-> type-> is<IUndefInfo> ()) {
	    return this-> _content-> BinaryOpRight (op, left);
	} else
	    return left-> info-> type-> BinaryOp (op, this-> _content);
    }

    InfoType IEnumInfo::AccessOp (Word op, syntax::ParamList params, std::vector <InfoType> & treats) {
	return this-> _content-> AccessOp (op, params, treats);
    }

    InfoType IEnumInfo::DotOp (syntax::Var var) {
	return this-> _content-> DotOp (var);
    }

    InfoType IEnumInfo::DotExpOp (syntax::Expression var) {
	return this-> _content-> DotExpOp (var);
    }

    InfoType IEnumInfo::DColonOp (syntax::Var var) {
	return this-> _content-> DColonOp (var);
    }

    InfoType IEnumInfo::UnaryOp (Word op) {
	return this-> _content-> UnaryOp (op);
    }

    InfoType IEnumInfo::CompOp (InfoType other) {
	if (other-> is<IUndefInfo> () || this-> isSame (other)) {
	    auto rf = this-> clone ()-> to <IEnumInfo> ();
	    auto ret = this-> _content-> CompOp (this-> _content);	    
	    rf-> comp = ret;
	    rf-> binopFoo = EnumUtils::InstGet;
	    return rf;
	} else {	    
	    return this-> _content-> CompOp (other);
	}
    }

    InfoType IEnumInfo::CastOp (InfoType other) {
	return this-> _content-> CastOp (other);
    }

    InfoType IEnumInfo::ApplyOp (const std::vector<syntax::Var> & other) {
	return this-> _content-> ApplyOp (other);
    }

    InfoType & IEnumInfo::getContent () {
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
    
