#include <ymir/semantic/types/_.hh>
#include <ymir/ast/ParamList.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/ast/TypedVar.hh>

namespace semantic {

    using namespace syntax;
    using namespace std;
    
    namespace StructUtils {
	using namespace Ymir;

	Tree InstCall (Word loc, InfoType ret, Expression, Expression paramsExp) {
	    ParamList params = paramsExp-> to <IParamList> ();
	    std::vector <tree> args = params-> toGenericParams (params-> getTreats ());
	    Ymir::TreeStmtList list;
	    StructInfo info = ret-> to <IStructInfo> ();
	    auto aux = Ymir::makeAuxVar (loc.getLocus (), ISymbol::getLastTmp (), ret-> toGeneric ());
	    
	    for (auto i : Ymir::r (0, info-> getAttribs ().size ())) {
		auto &it = info-> getAttribs () [i];
		auto attr = getField (loc.getLocus (), aux, it);
		list.append (buildTree (
		    MODIFY_EXPR, loc.getLocus (), attr.getType (), attr, args [i]
		));
	    }
	    
	    Ymir::getStackStmtList ().back ().append (list.getTree ());
	    return aux;
	}

	Tree InstCast (Word, InfoType, Expression elem, Expression) {
	    return elem-> toGeneric ();
	}

	Tree InstCastTuple (Word locus, InfoType type, Expression left, Expression) {
	    auto loc = locus.getLocus ();
	    auto info = type-> to<ITupleInfo> ();

	    auto rtree = left-> toGeneric ();
	    //auto rinfo = left-> info-> type-> to<IStructInfo> ();
	    //auto rtype = left-> info-> type-> toGeneric ();

	    Ymir::TreeStmtList list;
	    auto aux = Ymir::makeAuxVar (loc, ISymbol::getLastTmp (), info-> toGeneric ());
	    for (auto it : Ymir::r (0, info-> nbParams ())) {
		auto raux = getField (loc, rtree, it);
		auto laux = getField (loc, aux, it);
		list.append (Ymir::buildTree (
		    MODIFY_EXPR, loc, raux.getType (), laux, raux
		));
	    }
	    
	    getStackStmtList ().back ().append (list.getTree ());
	    return aux;
	}
	
	Tree InstGet (Word locus, InfoType, Expression left, Expression right) {
	    location_t loc = locus.getLocus ();
	    auto ltree = left-> toGeneric ();
	    auto val = right-> token.getStr ();
	    return getField (loc, ltree, val.c_str ());
	}
	
	Tree InstAffect (Word loc, InfoType, Expression left, Expression right) {
	    auto ltree = left-> toGeneric ();
	    auto rtree = right-> toGeneric ();
	    return buildTree (
		MODIFY_EXPR, loc.getLocus (), ltree.getType (), ltree, rtree
	    );
	}

	Ymir::Tree InstAddr (Word locus, InfoType, Expression elem, Expression) {
	    
	    return Ymir::getAddr (locus.getLocus (), elem-> toGeneric ());
	}		
    }
       
    IStructCstInfo::IStructCstInfo (Namespace space, string name, vector <Expression> &tmps) :
	IInfoType (true),
	space (space),
	name (name),
	tmps (tmps)       
    {}

    bool IStructCstInfo::isSame (InfoType) {
	return false;
    }

    InfoType IStructCstInfo::onClone () {
	return this;
    }
    
    InfoType IStructCstInfo::DotOp (syntax::Var) {
	return NULL;
    }
    
    InfoType IStructCstInfo::DColonOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "typeid") return StringOf ();
	return NULL;
    }
    
    ApplicationScore IStructCstInfo::CallOp (Word token, syntax::ParamList params) {
	if (this-> tmps.size () != 0) {
	    Ymir::Error::assert ("TODO");
	    return NULL;
	}
	if (params-> getParams ().size () != this-> params.size ())
	    return NULL;

	std::vector <InfoType> types;
	std::vector <std::string> attribs;
	
	auto score = new (Z0) IApplicationScore (token);
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    InfoType info = this-> params [it]-> getType ();
	    types.push_back (info);
	    attribs.push_back (this-> params [it]-> token.getStr ());
	    
	    auto type = params-> getParams () [it]-> info-> type-> CompOp (info);
	    //if (type) type = ConstVerif (info);
	    if (type) {
		type-> isConst (info-> isConst ());
		score-> score += 1;
		score-> treat.push_back (type);
	    } else return NULL;
	}

	auto ret = new (Z0) IStructInfo (this-> space, this-> name);
	ret-> setTypes (types);
	ret-> setAttribs (attribs); 
	
	ret-> multFoo = &StructUtils::InstCall;
	score-> dyn = true;
	score-> ret = ret;
	
	return score;
    }

    ApplicationScore IStructCstInfo::CallOp (Word token, const std::vector <InfoType>& params) {
	if (this-> tmps.size () != 0) {
	    Ymir::Error::assert ("TODO");
	    return NULL;
	}
	if (params.size () != this-> params.size ())
	    return NULL;

	std::vector <InfoType> types;
	std::vector <std::string> attribs;
	
	auto score = new (Z0) IApplicationScore (token);
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    InfoType info = this-> params [it]-> getType ();
	    types.push_back (info);
	    attribs.push_back (this-> params [it]-> token.getStr ());
	    
	    auto type = params [it]-> CompOp (info);
	    //if (type) type = ConstVerif (info);
	    if (type) {
		type-> isConst (info-> isConst ());
		score-> score += 1;
		score-> treat.push_back (type);
	    } else return NULL;
	}

	auto ret = new (Z0) IStructInfo (this-> space, this-> name);
	ret-> setTypes (types);
	ret-> setAttribs (attribs); 
	
	ret-> multFoo = &StructUtils::InstCall;
	score-> dyn = true;
	score-> ret = ret;
	
	return score;
    }

    InfoType IStructCstInfo::TempOp (const std::vector <syntax::Expression> & tmps) {
	if (this-> tmps.size () != 0) {
	    Ymir::Error::assert ("TODO");
	    return NULL;
	}

	if (this-> tmps.size () != tmps.size ()) return NULL;
	std::vector <InfoType> types;
	std::vector <std::string> attribs;

	for (auto it : Ymir::r (0, this-> params.size ())) {
	    InfoType info = this-> params [it]-> getType ();
	    types.push_back (info);
	    attribs.push_back (this-> params [it]-> token.getStr ());
	}
	
	auto ret = new (Z0) IStructInfo (this-> space, this-> name);
	ret-> setTypes (types);
	ret-> setAttribs (attribs);
	return ret;
    }
    
    std::string IStructCstInfo::innerTypeString () {
	Ymir::OutBuffer buf ("typeof ", this-> space.toString (), ".", this-> name, "{");
	for (auto i : Ymir::r (0, this-> params.size ())) {
	    auto it = this-> params [i];
	    buf.write (it-> getType ()-> innerTypeString ());
	    if (i < (int) this-> params.size () - 1)
		buf.write (", ");
	}
	buf.write ("}");
	return buf.str ();
    }

    std::string IStructCstInfo::innerSimpleTypeString () {
	return "";
    }

    bool IStructCstInfo::isType () {
	return true;
    }

    void IStructCstInfo::isPublic (bool isPublic) {
	this-> _isPublic = isPublic;
    }

    void IStructCstInfo::addAttrib (syntax::TypedVar param) {
	this-> params.push_back (param);
    }
    
    const char * IStructCstInfo::getId () {
	return IStructCstInfo::id ();
    }
        
    IStructInfo::IStructInfo (Namespace space, std::string name) :
	IInfoType (true),
	space (space),
	name (name)
    {}

    bool IStructInfo::isSame (InfoType other) {
	if (auto ot = other-> to <IStructInfo> ()) {
	    if (ot-> name == this-> name && ot-> space == this-> space) {
		for (auto it : Ymir::r (0, this-> types.size ())) {
		    if (!this-> types [it]-> isSame (ot-> types [it]))
			return false;
		}
		return true;
	    }
	}
	return false;
    }

    InfoType IStructInfo::onClone () {
	auto ret = new (Z0) IStructInfo (this-> space, this-> name);
	ret-> setAttribs (this-> attrs);
	for (auto it : this-> types) {
	    ret-> types.push_back (it-> clone ());
	}
	return ret;
    }

    
    InfoType IStructInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	for (auto it : Ymir::r (0, this-> attrs.size ())) {
	    if (var-> token == this-> attrs [it]) {
		auto ret = this-> types [it]-> clone ();
		ret-> binopFoo = StructUtils::InstGet;
		return ret;
	    }
	}
	
	if (var-> token == "tupleof") {
	    auto ret = new ITupleInfo (false);
	    for (auto it : this-> types)
		ret-> addParam (it-> clone ());
	    ret-> binopFoo = StructUtils::InstCastTuple;
	    return ret;
	}
	return NULL;
    }

    InfoType IStructInfo::DColonOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "typeid") return StringOf ();
	return NULL;
    }
    
    InfoType IStructInfo::CompOp (InfoType other) {
	if (this-> isSame (other) || other-> is <IUndefInfo> ()) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &StructUtils::InstCast;
	    return ret;
	} else if (auto ref = other-> to<IRefInfo> ()) {
	    if (!this-> isConst () && this-> isSame (ref-> content ())) {
		auto ret = new (Z0)  IRefInfo (false, this-> clone ());
		ret-> binopFoo = &StructUtils::InstAddr;
		return ret;
	    }
	}
	return NULL;
    }

    InfoType IStructInfo::BinaryOp (Word op, Expression right) {
	if (op == Token::EQUAL && right-> info-> type-> isSame (this)) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &StructUtils::InstAffect;
	    return ret;	    
	}
	return NULL;
    }
    
    InfoType IStructInfo::BinaryOpRight (Word op, Expression left) {
	if (op == Token::EQUAL && left-> info-> type-> is <IUndefInfo> ()) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &StructUtils::InstAffect;
	    return ret;	    
	}
	return NULL;
    }
        
    std::string IStructInfo::innerTypeString () {
	Ymir::OutBuffer buf (this-> space.toString (), ".", this-> name, "{");
	for (auto i : Ymir::r (0, this-> types.size ())) {
	    auto it = this-> types [i];
	    buf.write (it-> innerTypeString ());
	    if (i < (int) this-> types.size () - 1)
		buf.write (", ");
	}
	buf.write ("}");
	return buf.str ();
    }

    std::string IStructInfo::innerSimpleTypeString () {
	Ymir::OutBuffer buf (this-> space.toString (), ".", this-> name);
	for (auto i : Ymir::r (0, this-> types.size ())) {
	    auto it = this-> types [i];
	    buf.write (it-> innerSimpleTypeString ());
	}
	return buf.str ();
    }

    void IStructInfo::setTypes (std::vector <InfoType> types) {
	this-> types = types;
    }
    
    void IStructInfo::setAttribs (std::vector <std::string> names) {
	this-> attrs = names;
    }

    std::vector <std::string> & IStructInfo::getAttribs () {
	return this-> attrs;
    }
    
    Ymir::Tree IStructInfo::toGeneric () {
	auto name = this-> innerTypeString ();
	auto str_type_node = IFinalFrame::getDeclaredType (name.c_str ());
	if (str_type_node.isNull ()) {
	    str_type_node = Ymir::makeTuple (name, this-> types, this-> attrs);
	    IFinalFrame::declareType (name, str_type_node);
	}
	return str_type_node;
    }
    
    const char * IStructInfo::getId () {
	return IStructInfo::id ();
    }
        

    
}
