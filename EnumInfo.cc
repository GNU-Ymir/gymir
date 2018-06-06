#include <ymir/semantic/types/_.hh>
#include <ymir/errors/Error.hh>
#include <ymir/ast/TreeExpression.hh>
#include <ymir/utils/Mangler.hh>
#include <ymir/utils/Options.hh>
#include <ymir/semantic/tree/Generic.hh>

namespace semantic {

    using namespace syntax;

    namespace EnumUtils {

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

	Tree InstGet (Word locus, EnumCstInfo info, ulong i) {
	    if (info-> comps [i]) {
		if (info-> comps [i]-> unopFoo) {
		    return info-> comps [i]-> buildUnaryOp (
			locus, info-> comps [i], info-> values [i]
		    );
		} else {
		    return info-> comps [i]-> buildBinaryOp (
			locus, info-> comps [i], info-> values [i],
			new (Z0) ITreeExpression (locus, info-> comps [i], Ymir::Tree ())
		    );
		}
	    } else return info-> values [i]-> toGeneric ();
	}
	
	Tree InstMembers (Word locus, InfoType type, Expression elem) {
	    Ymir::TreeStmtList list;
	    ArrayInfo info = type-> to <IArrayInfo> ();
	    auto encst = elem-> info-> type-> to <IEnumCstInfo> ();
	    Ymir::Tree innerType = info-> content ()-> toGeneric ();
	    auto intExpr = new (Z0) IFixed (locus, FixedConst::ULONG);
	    intExpr-> setUValue (encst-> values.size ());
	    auto lenExpr = intExpr-> expression ();
	    auto len = lenExpr-> toGeneric ();
	    intExpr-> setUValue (0);
	    auto begin = intExpr-> toGeneric ();

	    Ymir::Tree range_type = build_range_type (integer_type_node, fold (begin.getTree ()), fold (len.getTree ()));
	    Ymir::Tree array_type = build_array_type (innerType.getTree (), range_type.getTree ());
	
	    Ymir::Tree aux = Ymir::makeAuxVar (locus.getLocus (),
					       ISymbol::getLastTmp (),
					       array_type
	    );

	    for (uint i = 0 ; i < encst-> values.size () ; i++) {
		auto ref = Ymir::getArrayRef (locus.getLocus (), aux, innerType, i);
		// auto left = new (Z0) ITreeExpression (locus, info-> content (), ref);
		auto right = EnumUtils::InstGet (locus, encst, i);
		if (ref.getType () == right.getType ()) {
		    list.append (Ymir::buildTree (
			MODIFY_EXPR, locus.getLocus (),
			void_type_node, ref, right
		    ));
		} else {
		    auto ptrl = Ymir::getAddr (locus.getLocus (), ref).getTree ();
		    auto ptrr = Ymir::getAddr (locus.getLocus (), right).getTree ();
		    tree tmemcpy = builtin_decl_explicit (BUILT_IN_MEMCPY);
		    tree size = TYPE_SIZE_UNIT (ref.getType ().getTree ());
		    auto res = build_call_expr (tmemcpy, 3, ptrl, ptrr, size);
		    list.append (res);
		}
	    }
	    return Ymir::compoundExpr (locus.getLocus (), list.getTree (), aux);
	}	
	
	Tree InstGet (Word locus, InfoType type, Expression elem, Expression) {
	    EnumInfo ecst = type-> to <IEnumInfo> ();
	    ecst-> getValue () = elem;
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

	Tree InstUnrefMult (Word locus, InfoType type, Expression left, Expression right, ApplicationScore score) {
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);

	    InfoType inner = type;
	    
	    auto linfo = left-> info-> type-> to <IEnumInfo> ();
	    if (linfo) {
		left-> info-> type = linfo-> content ();
		left-> info-> value () = NULL;

		left-> info-> type-> binopFoo = linfo-> binopFoo;
		left-> info-> type-> unopFoo = linfo-> unopFoo;
		left-> info-> type-> multFoo = linfo-> multFoo;
	    }

	    return type-> buildMultOp (
		locus, inner, left, right, score
	    );	    
	}
	
	Tree InstUnrefBin (Word locus, InfoType type, Expression left, Expression right) {
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);

	    InfoType inner = type;
	    
	    auto linfo = left-> info-> type-> to <IEnumInfo> ();
	    if (linfo) {
		left-> info-> type = linfo-> content ();
		left-> info-> value () = NULL;

		left-> info-> type-> binopFoo = linfo-> binopFoo;
		left-> info-> type-> unopFoo = linfo-> unopFoo;
		left-> info-> type-> multFoo = linfo-> multFoo;
	    }

	    return type-> buildBinaryOp (
		locus, inner, left, right
	    );	    
	}

	Tree InstUnrefBinRight (Word locus, InfoType type, Expression left, Expression right) {
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);

	    InfoType inner = type;
	    
	    auto rinfo = right-> info-> type-> to <IEnumInfo> ();
	    if (rinfo) {
		right-> info-> type = rinfo-> content ();
		right-> info-> value () = NULL;

		right-> info-> type-> binopFoo = rinfo-> binopFoo;
		right-> info-> type-> unopFoo = rinfo-> unopFoo;
		right-> info-> type-> multFoo = rinfo-> multFoo;
	    }
	    
	    return type-> buildBinaryOp (
		locus, inner, left, right
	    );
	}

	Tree InstUnrefBinDouble (Word locus, InfoType type, Expression left, Expression right) {
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);

	    InfoType inner = type;
	    
	    auto rinfo = right-> info-> type-> to <IEnumInfo> ();
	    if (rinfo) {
		right-> info-> type = rinfo-> content ();
		right-> info-> value () = NULL;

		right-> info-> type-> binopFoo = rinfo-> binopFoo;
		right-> info-> type-> unopFoo = rinfo-> unopFoo;
		right-> info-> type-> multFoo = rinfo-> multFoo;
	    }

	    auto linfo = left-> info-> type-> to <IEnumInfo> ();
	    if (linfo) {
		left-> info-> type = linfo-> content ();
		left-> info-> value () = NULL;
		left-> info-> type-> binopFoo = linfo-> binopFoo;
		left-> info-> type-> unopFoo = linfo-> unopFoo;
		left-> info-> type-> multFoo = linfo-> multFoo;
	    }

	    return type-> buildBinaryOp (
		locus, inner, left, right
	    );
	}

	Tree InstUnrefUn (Word locus, InfoType type, Expression left) {
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);

	    InfoType inner = type;

	    auto linfo = left-> info-> type-> to <IEnumInfo> ();
	    if (linfo) { 
		left-> info-> type = linfo-> content ();
		left-> info-> value () = NULL;
		
		left-> info-> type-> binopFoo = linfo-> binopFoo;
		left-> info-> type-> unopFoo = linfo-> unopFoo;
		left-> info-> type-> multFoo = linfo-> multFoo;
	    }
	    
	    if (type-> unopFoo) {
		return type-> buildUnaryOp (
		    locus, inner, left
		);
	    } else if (type-> binopFoo) {
		return type-> buildBinaryOp (
		    locus, inner, left, new (Z0) ITreeExpression (locus, type, Ymir::Tree ())
		);
	    } else {
		return left-> toGeneric ();
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
	
	if (var-> token == "members")
	    return GetMembers ();
	
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

    InfoType IEnumCstInfo::GetMembers () {
	auto type = new (Z0) IArrayInfo (true, this-> type-> cloneConst ());
	type-> isStatic (true, this-> values.size ());
	type-> unopFoo = EnumUtils::InstMembers;
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
	return this-> _content-> BinaryOp (op, right);	       
    }

    InfoType IEnumInfo::BinaryOpRight (Word op, syntax::Expression left) {
	if (left-> info-> type-> is<IUndefInfo> ()) {
	    auto ret = this-> clone ()-> to <IEnumInfo> ();
	    ret-> _content = this-> _content-> BinaryOpRight (op, left);
	    ret-> binopFoo = ret-> content ()-> binopFoo;
	    return ret;
	}
	
	auto aux = this-> _content-> BinaryOpRight (op, left);
	if (aux != NULL) {
	    return aux;
	} else {
	    return left-> info-> type-> BinaryOp (op, this-> _content);
	}
	return NULL;	
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

    ApplicationScore IEnumInfo::CallOp (Word op, syntax::ParamList params) {
	return this-> _content-> CallOp (op, params);
    }
    
    InfoType IEnumInfo::addUnref (InfoType elem) {
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

	if (binop) elem-> binopFoo = &EnumUtils::InstUnrefBin;
	if (unop) elem-> unopFoo = &EnumUtils::InstUnrefUn;
	if (mult) elem-> multFoo = &EnumUtils::InstUnrefMult;
	return elem;
    }
    
    InfoType IEnumInfo::addUnrefDouble (InfoType elem) {
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
	
	if (binop) elem-> binopFoo = &EnumUtils::InstUnrefBinDouble;
	if (unop) elem-> unopFoo = &EnumUtils::InstUnrefUn;
	if (mult) elem-> multFoo = &EnumUtils::InstUnrefMult;
	return elem;
    }
    
    InfoType IEnumInfo::addUnrefRight (InfoType elem) {
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
	
	if (binop) elem-> binopFoo = &EnumUtils::InstUnrefBinRight;
	if (unop) elem-> unopFoo = &EnumUtils::InstUnrefUn;
	if (mult) elem-> multFoo = &EnumUtils::InstUnrefMult;
	return elem;
    }    
    
    InfoType & IEnumInfo::getContent () {
	return this-> _content;
    }

    InfoType IEnumInfo::getIntern () {
	return this-> _content;
    }

    bool IEnumInfo::allowInternalUnref () {
	return Options::instance ().itsLintTime ();
    }
    
    std::string IEnumInfo::innerSimpleTypeString () {
	auto name = Mangler::mangle_type (this-> _space.toString () + "." + this-> _name);
	return Ymir::format ("%%%", name.length () + 1, name, "E");
    }

    std::string IEnumInfo::innerTypeString () {
	return Ymir::format ("%.%(%)", this-> _space.toString (), this-> _name.c_str (), this-> _content-> innerTypeString ());
    }

    const char* IEnumInfo::getId () {
	return IEnumInfo::id ();
    }

    bool IEnumInfo::isSame (InfoType other) {
	if (auto en = other-> to <IEnumInfo> ()) {
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
    
