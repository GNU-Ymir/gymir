#include <ymir/semantic/types/_.hh>
#include <ymir/ast/ParamList.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/ast/TypedVar.hh>
#include <ymir/semantic/value/StringValue.hh>
#include <ymir/semantic/pack/InternalFunction.hh>

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

	Tree InstInit (Word locus, InfoType type, Expression) {
	    auto loc = locus.getLocus ();
	    auto ltree = Ymir::makeAuxVar (loc, ISymbol::getLastTmp (), type-> toGeneric ());
	    auto addr = Ymir::getAddr (loc, ltree);
	    tree memsetArgs [] = {addr.getTree (),
				  build_int_cst_type (long_unsigned_type_node, 0),
				  TYPE_SIZE_UNIT (ltree.getType ().getTree ())};

	    Ymir::getStackStmtList ().back ().append (build_call_array_loc (loc, void_type_node, InternalFunction::getYMemset ().getTree (), 3, memsetArgs));
	    return ltree;
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

	Ymir::Tree InstSizeOf (Word, InfoType, Expression elem) {	    
	    return TYPE_SIZE_UNIT (elem-> info-> type-> toGeneric ().getTree ());
	}
	
	Ymir::Tree InstSizeOfCst (Word, InfoType, Expression elem) {
	    StructCstInfo cst = elem-> info-> type-> to <IStructCstInfo> ();
	    auto type = cst-> TempOp ({})-> toGeneric ().getTree ();
	    return TYPE_SIZE_UNIT (type);
	}

    }
       
    IStructCstInfo::IStructCstInfo (Namespace space, string name, vector <Expression> &tmps) :
	IInfoType (true),
	space (space),
	name (name),
	tmps (tmps)       
    {}

    bool IStructCstInfo::isSame (InfoType other) {
	if (auto ot = other-> to<IStructCstInfo> ()) {
	    if (ot-> name == this-> name && ot-> space == this-> space) {
		if (this-> tmpsDone.size () != ot-> tmpsDone.size ()) return false;
		for (auto it : Ymir::r (0, this-> tmpsDone.size ())) {
		    if (!this-> tmpsDone [it]-> info-> type-> isSame (ot-> tmpsDone [it]-> info-> type))
			return false;
		}
		return true;
	    }
	}
	return false;
    }

    bool IStructCstInfo::isInstance (StructInfo ot) {
	if (ot-> name == this-> name && ot-> space == this-> space) {
	    if (this-> tmpsDone.size () != ot-> tmpsDone.size ()) return false;
	    for (auto it : Ymir::r (0, this-> tmpsDone.size ())) {
		if (!this-> tmpsDone [it]-> info-> type-> isSame (ot-> tmpsDone [it]-> info-> type))
		    return false;
	    }
	    return true;
	}
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
	if (var-> token == "init") return Init ();
	if (var-> token == "sizeof") return SizeOf ();
	return NULL;
    }

    InfoType IStructCstInfo::Init () {
	std::vector <syntax::Expression> exp;
	auto ret = this-> TempOp (exp);
	if (ret == NULL) return NULL;
	ret-> unopFoo = StructUtils::InstInit;
	return ret;
    }
    
    InfoType IStructCstInfo::SizeOf () {
	if (this-> TempOp ({}) != NULL) {
	    auto ret = new (Z0) IFixedInfo (true, FixedConst::UBYTE);
	    ret-> unopFoo = StructUtils::InstSizeOfCst;
	    return ret;
	}
	return NULL;
    }

    
    ApplicationScore IStructCstInfo::CallOp (Word token, syntax::ParamList params) {
	if (this-> tmps.size () != 0) {
	    return NULL;
	}
 
	if (params-> getParams ().size () != this-> params.size ())
	    return NULL;

	std::vector <InfoType> types;
	std::vector <std::string> attribs;
	
	auto score = new (Z0) IApplicationScore (token);
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    InfoType info = this-> params [it]-> getType ();
	    if (info == NULL) return NULL;
	    types.push_back (info);
	    attribs.push_back (this-> params [it]-> token.getStr ());
	    
	    auto type = params-> getParams () [it]-> info-> type-> CompOp (info);
	    if (type) type = type-> ConstVerif (info);
	    if (type) {
		type-> isConst (info-> isConst ());
		score-> score += 1;
		score-> treat.push_back (type);
	    } else return NULL;
	}

	auto ret = new (Z0) IStructInfo (this-> space, this-> name);
	ret-> isConst (false);
	ret-> setTypes (types);
	ret-> setAttribs (attribs);
	ret-> setTmps (this-> tmpsDone);
	    
	ret-> multFoo = &StructUtils::InstCall;
	score-> dyn = true;
	score-> ret = ret;
	
	return score;
    }

    ApplicationScore IStructCstInfo::CallOp (Word token, const std::vector <InfoType>& params) {
	if (this-> tmps.size () != 0) {
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
	ret-> isConst (false);
	ret-> setTypes (types);
	ret-> setAttribs (attribs);
	ret-> setTmps (this-> tmpsDone);
	
	ret-> multFoo = &StructUtils::InstCall;
	score-> dyn = true;
	score-> ret = ret;
	
	return score;
    }

    InfoType IStructCstInfo::TempOp (const std::vector <syntax::Expression> & tmps) {
	if (this-> tmps.size () != 0) {
	    return this-> getScore (tmps);
	}

	if (this-> tmps.size () != tmps.size ()) return NULL;
	std::vector <InfoType> types;
	std::vector <std::string> attribs;
	static std::map <std::string, StructInfo> dones;
	auto name = this-> onlyNameTypeString ();
	auto inside = dones.find (name);
	if (inside == dones.end ()) {
	    this-> _info = new (Z0) IStructInfo (this-> space, this-> name);
	    dones [name] = this-> _info;
	    for (auto it : Ymir::r (0, this-> params.size ())) {
		InfoType info = this-> params [it]-> getType ();
		if (info) {
		    if (recursiveGet (this-> _info, info)) {
			Ymir::Error::recursiveNoSize (this-> params [it]-> token);
			return NULL;
		    }
		    types.push_back (info);
		    attribs.push_back (this-> params [it]-> token.getStr ());
		} else return NULL;
	    }

	    dones.erase (name);
	    this-> _info-> isConst (false);
	    this-> _info-> setTypes (types);
	    this-> _info-> setAttribs (attribs);	    
	    this-> _info-> setTmps (this-> tmpsDone);	    
	    return this-> _info-> clone ();
	} else {
	    return inside-> second;//-> clone ();
	}		
    }

    bool IStructCstInfo::recursiveGet (InfoType who, InfoType where) {
	if (who-> isSame (where)) return true;
	if (auto str = where-> to <IStructInfo> ()) {
	    for (auto it : str-> getTypes ()) {
		if (recursiveGet (who, it)) return true;
	    }
	    return false;
	} else if (auto cstr = where-> to <IStructCstInfo> ()) {
	    return recursiveGet (who, cstr-> TempOp ({}));
	} else if (auto arr = where-> to <IArrayInfo> ()) {
	    if (arr-> isStatic ()) {
		return recursiveGet (who, arr-> content ());
	    }
	}
	return false;
    }
    
    template <typename K, typename V>
    vector <V> getValues (map <K, V> dico) {
	vector <V> vec;
	for (auto it : dico)
	    vec.push_back (it.second);
	return vec;
    }
    
    InfoType IStructCstInfo::getScore (const std::vector <syntax::Expression> & tmps) {
	auto res = TemplateSolver::instance ().solve (this-> tmps, tmps);
	if (!res.valid || !TemplateSolver::instance ().isSolved (this-> tmps, res))
	    return NULL;
	
	std::vector <syntax::TypedVar> params;
	for (auto it : this-> params) {
	    params.push_back (it-> templateExpReplace (res.elements)-> to <ITypedVar> ());
	}

	std::vector <syntax::Expression> ignore;
	auto ret = new (Z0) IStructCstInfo (this-> space, this-> name, ignore);
	ret-> params = params;	
	ret-> tmpsDone = getValues (res.elements);
	for (auto &it : ret-> tmpsDone) {
	    it = it-> templateExpReplace ({});
	}
	
	return ret;	
    }

    std::string IStructCstInfo::onlyNameTypeString () {
	Ymir::OutBuffer buf (this-> space.toString (), ".", this-> name, "!(");
	for (auto it : Ymir::r (0, this-> tmpsDone.size ())) {
	    if (auto ps = this-> tmpsDone [it]-> to <IParamList> ()) {
		buf.write ("{");
		for (auto it_ : Ymir::r (0, ps-> getParams ().size ())) {
		    buf.write (ps-> getParams () [it_]-> info-> typeString ());
		    if (it_ < (int) ps-> getParams ().size () - 1)
			buf.write (", ");
		}
		buf.write ("}");
	    } else {
		buf.write (this-> tmpsDone [it]-> info-> typeString ().c_str ());
	    }

	    if (it < (int) this-> tmpsDone.size () - 1)
		buf.write (", ");	    
	}
	buf.write (")");
	return buf.str ();
    }
    
    std::string IStructCstInfo::typeString () {
	Ymir::OutBuffer buf ("typeof ", this-> space.toString (), ".", this-> name, "{");
	for (auto i : Ymir::r (0, this-> params.size ())) {
	    auto it = this-> params [i];
	    auto t = it-> getType ();
	    if (t != NULL) 
		buf.write (t-> innerTypeString ());	
	    else buf.write ("undef");		    
	    if (i < (int) this-> params.size () - 1)
		buf.write (", ");		
	}
	buf.write ("}");
	return buf.str ();
    }
    
    std::string IStructCstInfo::innerTypeString () {
	static std::map <std::string, bool> inner;
	Ymir::OutBuffer buf (this-> space.toString (), ".", this-> name);
	auto inside = inner.find (buf.str ());
	if (inside == inner.end () || !inside-> second) {
	    std::string name = buf.str ();	    
	    inner [name] = true;
	    buf.write ("{");
	    for (auto i : Ymir::r (0, this-> params.size ())) {
		auto it = this-> params [i];
		auto t = it-> getType ();
		if (t)
		    buf.write (t-> innerTypeString ());
		else buf.write ("undef");		
		if (i < (int) this-> params.size () - 1)
		    buf.write (", ");
	    }
	    buf.write ("}");
	    inner [name] = false;
	}
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
		if (this-> tmpsDone.size () != ot-> tmpsDone.size ()) return false;
		for (auto it : Ymir::r (0, this-> tmpsDone.size ())) {
		    if (auto ps = this-> tmpsDone [it]-> to <IParamList> ()) {
			if (auto ps2 = ot-> tmpsDone [it]-> to <IParamList> ()) {
			    if (ps-> getParams ().size () != ps2-> getParams ().size ()) return false;
			    for (auto it_ : Ymir::r (0, ps-> getParams ().size ())) {
				if (!ps-> getParams ()[it_]-> info-> type-> isSame (ps2-> getParams () [it_]-> info-> type))
				    return false;
			    }
			} else return false;
		    } else {
			if (ot-> tmpsDone [it]-> is <IParamList> ()) return false;
			if (!this-> tmpsDone [it]-> info-> type-> isSame (ot-> tmpsDone [it]-> info-> type))
			    return false;
		    }
		}
		return true;
	    }
	}
	return false;
    }

    InfoType IStructInfo::ConstVerif (InfoType other) {
	if (this-> isConst () && !other-> isConst ()) return NULL;
	return this;
    }
    
    InfoType IStructInfo::onClone () {
	auto ret = new (Z0) IStructInfo (this-> space, this-> name);
	ret-> setAttribs (this-> attrs);
	for (auto it : this-> types) {
	    ret-> types.push_back (it);
	}
	
	ret-> tmpsDone = this-> tmpsDone;
	ret-> isConst (this-> isConst ());
	return ret;
    }

    ApplicationScore IStructInfo::CallOp (Word token, syntax::ParamList params) {
	if (params-> getParams ().size () != this-> types.size ())
	    return NULL;

	std::vector <InfoType> types;
	std::vector <std::string> attribs;
	
	auto score = new (Z0) IApplicationScore (token);
	for (auto it : Ymir::r (0, this-> attrs.size ())) {
	    InfoType info = this-> types [it];
	    if (info == NULL) return NULL;
	    types.push_back (info);
	    attribs.push_back (this-> attrs [it]);
	    
	    auto type = params-> getParams () [it]-> info-> type-> CompOp (info);
	    if (type) type = type-> ConstVerif (info);
	    if (type) {
		type-> isConst (info-> isConst ());
		score-> score += 1;
		score-> treat.push_back (type);
	    } else return NULL;
	}

	auto ret = new (Z0) IStructInfo (this-> space, this-> name);
	ret-> isConst (this-> isConst ());
	ret-> setTypes (types);
	ret-> setAttribs (attribs);
	ret-> setTmps (this-> tmpsDone);
	    
	ret-> multFoo = &StructUtils::InstCall;
	score-> dyn = true;
	score-> ret = ret;

	return score;
    }
    
    InfoType IStructInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	for (auto it : Ymir::r (0, this-> attrs.size ())) {
	    if (var-> token == this-> attrs [it]) {
		auto ret = this-> types [it]-> clone ();
		if (this-> isConst ()) {
		    ret-> isConst (true);
		} else ret-> isConst (this-> types [it]-> isConst ());
		ret = new (Z0) IArrayRefInfo (this-> isConst (), ret);
		ret-> binopFoo = StructUtils::InstGet;
		return ret;
	    }
	}
	
	if (var-> token == "tupleof") {
	    auto ret = new (Z0) ITupleInfo (false);
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
	if (var-> token == "init") return Init ();
	if (var-> token == "sizeof") return SizeOf ();
	return NULL;
    }

    InfoType IStructInfo::Init () {
	auto ret = this-> clone ();
	ret-> unopFoo = StructUtils::InstInit;
	return ret;
    }

    InfoType IStructInfo::SizeOf () {
	auto ret = new (Z0) IFixedInfo (true, FixedConst::UBYTE);
	ret-> unopFoo = StructUtils::InstSizeOf;
	return ret;
    }
    
    InfoType IStructInfo::CompOp (InfoType other) {
	if (this-> isSame (other)) {
	    auto ret = other-> clone ();
	    ret-> isConst (this-> isConst ());
	    ret-> binopFoo = &StructUtils::InstCast;
	    return ret;
	} else if (other-> is <IUndefInfo> ()) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &StructUtils::InstCast;
	    
	    return ret;
	} else if (auto ref = other-> to<IRefInfo> ()) {
	    if (!this-> isConst () && this-> isSame (ref-> content ())) {
		auto ret = new (Z0)  IRefInfo (false, ref-> content ()-> clone ());
		ret-> content ()-> isConst (this-> isConst ());
		ret-> binopFoo = &StructUtils::InstAddr;
		return ret;
	    }
	}
	return NULL;
    }

    InfoType IStructInfo::UnaryOp (Word op) {
	if (op == Token::AND) {
	    auto ret = new (Z0) IPtrInfo (this-> isConst (), this-> clone ());
	    ret-> binopFoo = &StructUtils::InstAddr;
	    return ret;
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
    
    std::string IStructInfo::onlyNameTypeString () {
	Ymir::OutBuffer buf (this-> space.toString (), ".", this-> name, "!(");
	for (auto it : Ymir::r (0, this-> tmpsDone.size ())) {
	    if (auto ps = this-> tmpsDone [it]-> to <IParamList> ()) {
		buf.write ("{");
		for (auto it_ : Ymir::r (0, ps-> getParams ().size ())) {
		    buf.write (ps-> getParams () [it_]-> info-> typeString ());
		    if (it_ < (int) ps-> getParams ().size () - 1)
			buf.write (", ");
		}
		buf.write ("}");
	    } else {
		buf.write (this-> tmpsDone [it]-> info-> typeString ().c_str ());
	    }
	    
	    if (it < (int) this-> tmpsDone.size () - 1)
		buf.write (", ");	    
	}
	buf.write (")");
	return buf.str ();
    }

    std::string IStructInfo::innerTypeString () {
	return onlyNameTypeString ();
    }

    Namespace& IStructInfo::getSpace () {
	return this-> space;
    }
    
    std::string IStructInfo::getName () {
	return this-> name;
    }
    
    std::string IStructInfo::innerSimpleTypeString () {
	return this-> onlyNameTypeString ();
    }

    void IStructInfo::setTypes (std::vector <InfoType> types) {
	this-> types = types;
    }
    
    void IStructInfo::setAttribs (std::vector <std::string> names) {
	this-> attrs = names;
    }

    void IStructInfo::setTmps (std::vector <syntax::Expression> tmps) {
	this-> tmpsDone = tmps;
    }

    InfoType IStructInfo::getTemplate (ulong nb) {	
	if (nb < this-> tmpsDone.size ()) {
	    if (auto ps = this-> tmpsDone [nb]-> to <IParamList> ()) {
		return ps-> getParams ()[0]-> info-> type;
	    }
	    return this-> tmpsDone [nb]-> info-> type;
	} else {
	    nb = nb - this-> tmpsDone.size ();
	    if (nb < this-> tmpsDone.size ()) {
		if (auto ps = this-> tmpsDone [nb]-> to <IParamList> ()) {
		    if (nb < ps-> getParams ().size ())
			return ps-> getParams () [nb]-> info-> type;
		}
	    }
	}
	return NULL;
    }
    
    std::vector <std::string> & IStructInfo::getAttribs () {
	return this-> attrs;
    }

    std::vector <InfoType> & IStructInfo::getTypes () {
	return this-> types;
    }

    InfoType IStructInfo::StringOf () {
	auto str = new (Z0) IStringInfo (true);
	str-> value () = new (Z0)  IStringValue (this-> onlyNameTypeString ().c_str ());
	return str;
    }
    
    Ymir::Tree IStructInfo::toGeneric () {
	auto name = this-> onlyNameTypeString ();	
	auto str_type_node = IFinalFrame::getDeclaredType (name.c_str ());	
	if (str_type_node.isNull ()) {
	    str_type_node = Ymir::makeTuple (name, this-> types, this-> attrs);
	    IFinalFrame::declareType (name, str_type_node);
	}
	return str_type_node;
    }

    Ymir::Tree IStructCstInfo::toGeneric () {
	return void_type_node;
    }
    
    const char * IStructInfo::getId () {
	return IStructInfo::id ();
    }
        

    
}
