#include <ymir/semantic/types/_.hh>
#include <ymir/ast/ParamList.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/ast/TypedVar.hh>
#include <ymir/semantic/value/StringValue.hh>
#include <ymir/semantic/pack/InternalFunction.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/utils/Mangler.hh>
#include <ymir/semantic/object/AggregateInfo.hh>

namespace semantic {

    using namespace syntax;
    using namespace std;
    
    namespace StructUtils {
	using namespace Ymir;

	Tree InstCall (Word loc, InfoType ret, Expression, Expression paramsExp, ApplicationScore) {
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
	    
	    return Ymir::compoundExpr (loc.getLocus (), list.getTree (), aux);
	}
	
	Tree InstCallUnion (Word loc, InfoType ret, Expression, Expression paramsExp, ApplicationScore) {
	    ParamList params = paramsExp-> to <IParamList> ();
	    std::vector <tree> args = params-> toGenericParams (params-> getTreats ());
	    Ymir::TreeStmtList list;
	    StructInfo info = ret-> to <IStructInfo> ();
	    auto aux = Ymir::makeAuxVar (loc.getLocus (), ISymbol::getLastTmp (), ret-> toGeneric ());
	    
	    for (auto i : Ymir::r (0, info-> getAttribs ().size ())) {
		auto &it = info-> getAttribs () [i];
		auto attr = getField (loc.getLocus (), aux, it);
		if ((Tree {args [0]}).getType () == attr.getType ()) {
		    list.append (buildTree (
			MODIFY_EXPR, loc.getLocus (), attr.getType (), attr, args [0]
		    ));
		    break;
		}
	    }
	    
	    return Ymir::compoundExpr (loc.getLocus (), list.getTree (), aux);
	}

	Tree InstCast (Word loc, InfoType type, Expression elem, Expression) {
	    auto toType = type->  toGeneric ();
	    auto rtree = elem-> toGeneric ();
	    if (toType != rtree.getType ()) {
		Ymir::TreeStmtList list;
		auto ltree = Ymir::makeAuxVar (loc.getLocus (), ISymbol::getLastTmp (), toType);
		auto ptrl = Ymir::getAddr (loc.getLocus (), ltree).getTree ();
		auto ptrr = Ymir::getAddr (loc.getLocus (), rtree).getTree ();
		tree tmemcopy = builtin_decl_explicit (BUILT_IN_MEMCPY);
		tree size = TYPE_SIZE_UNIT (ltree.getType ().getTree ());	    
		auto result = build_call_expr_loc (loc.getLocus (), tmemcopy, 3, ptrl, ptrr, size);
		list.append (result);
		
		return Ymir::compoundExpr (loc.getLocus (), list, ltree);
	    } else {
	    	return rtree;
	    }
	}

	Tree InstCastTuple (Word, InfoType, Expression left, Expression) {
	    return left-> toGeneric ();
	}

	Tree InstInit (Word locus, InfoType type, Expression) {
	    auto loc = locus.getLocus ();
	    auto ltree = Ymir::makeAuxVar (loc, ISymbol::getLastTmp (), type-> toGeneric ());
	    auto addr = Ymir::getAddr (loc, ltree).getTree ();
				  
	    auto size = TYPE_SIZE_UNIT (ltree.getType ().getTree ());
	    tree tmemset = builtin_decl_explicit (BUILT_IN_MEMSET);
	    
	    auto result = build_call_expr_loc (loc, tmemset, 3, addr, integer_zero_node, size);
	    return Ymir::compoundExpr (loc, result, ltree);
	}
	
	Tree InstGet (Word locus, InfoType, Expression left, Expression right) {
	    location_t loc = locus.getLocus ();
	    auto ltree = left-> toGeneric ();
	    auto val = right-> token.getStr ();
	    return getField (loc, ltree, val.c_str ()).getTree ();
	}
	
	Tree InstAffect (Word loc, InfoType, Expression left, Expression right) {
	    Ymir::TreeStmtList list;	    
	    auto ltree = Ymir::getExpr (list, left);
	    auto rtree = Ymir::getExpr (list, right);
	    if (ltree.getType () != rtree.getType ()) {
		auto ptrl = Ymir::getAddr (loc.getLocus (), ltree).getTree ();
		auto ptrr = Ymir::getAddr (loc.getLocus (), rtree).getTree ();
		tree tmemcopy = builtin_decl_explicit (BUILT_IN_MEMCPY);
		tree size = TYPE_SIZE_UNIT (ltree.getType ().getTree ());	    
		auto result = build_call_expr_loc (loc.getLocus (), tmemcopy, 3, ptrl, ptrr, size);
		list.append (result);
		return Ymir::compoundExpr (loc.getLocus (), list, ltree);
	    } else {
		list.append (buildTree (
		    MODIFY_EXPR, loc.getLocus (), void_type_node, ltree, rtree 
		));
		return Ymir::compoundExpr (loc.getLocus (), list, ltree);
	    }
	}

	Ymir::Tree InstAddr (Word locus, InfoType, Expression elem, Expression) {
	    return Ymir::getAddr (locus.getLocus (), elem-> toGeneric ());
	}

	Ymir::Tree InstSizeOf (Word, InfoType, Expression elem) {	    
	    return TYPE_SIZE_UNIT (elem-> info-> type ()-> toGeneric ().getTree ());
	}
	
	Ymir::Tree InstSizeOfCst (Word, InfoType, Expression elem) {
	    StructCstInfo cst = elem-> info-> type ()-> to <IStructCstInfo> ();
	    auto type = cst-> TempOp ({})-> toGeneric ().getTree ();
	    return TYPE_SIZE_UNIT (type);
	}

    }
       
    IStructCstInfo::IStructCstInfo (Word locId, Namespace space, string name, vector <Expression> &tmps, vector <Word> attrs, bool isUnion) :
	IInfoType (true),	
	space (space),
	_locId (locId),
	name (name),
	tmps (tmps),
	_udas (attrs),
	_isUnion (isUnion)
    {}

    bool IStructCstInfo::isSame (InfoType other) {
	if (auto ot = other-> to<IStructCstInfo> ()) {
	    if (ot-> name == this-> name && ot-> space == this-> space) {
		if (this-> tmpsDone.size () != ot-> tmpsDone.size ()) return false;
		for (auto it : Ymir::r (0, this-> tmpsDone.size ())) {
		    if (!this-> tmpsDone [it]-> info-> type ()-> isSame (ot-> tmpsDone [it]-> info-> type ()))
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
		if (!this-> tmpsDone [it]-> info-> type ()-> isSame (ot-> tmpsDone [it]-> info-> type ()))
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
	if (var-> token == "typeid") {
	    auto ret = this-> TempOp ({});
	    if (ret == NULL) return NULL;
	    return ret-> StringOf ();
	}
	if (var-> token == "init") return Init ();
	if (var-> token == "sizeof") return SizeOf ();
	if (var-> token == "name") return Name ();
	return NULL;
    }

    InfoType IStructInfo::Name () {
	auto str = new (Z0) IStringInfo (true);
	str-> value () = new (Z0)  IStringValue (this-> name.c_str ());
	return str;
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
	    auto ret = new (Z0) IFixedInfo (true, FixedConst::UINT);
	    ret-> unopFoo = StructUtils::InstSizeOfCst;
	    return ret;
	}
	return NULL;
    }

    ApplicationScore IStructCstInfo::CallOpUnion (Word token, syntax::ParamList params) {
	if (params-> getParams ().size () != 1) return NULL;

	std::vector <InfoType> types;
	std::vector <std::string> attribs;
	bool done = false;
	auto last = Table::instance ().templateNamespace ();
	auto currentSpace = Table::instance ().space ();
	
	auto score = new (Z0) IApplicationScore (token);
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    Table::instance ().setCurrentSpace (this-> space);
	    Table::instance ().templateNamespace () = currentSpace;
	    InfoType info = this-> params [it]-> getType ();
	    if (info == NULL) return NULL;
	    types.push_back (info);
	    attribs.push_back (this-> params [it]-> token.getStr ());

	    if (!done) {
		auto type = params-> getParams () [0]-> info-> type ()-> CompOp (info);
		if (type) type = type-> ConstVerif (info);
		if (type) {
		    type-> isConst (info-> isConst ());
		    score-> score += 1;
		    score-> treat.push_back (type);
		    done = true;
		} 
	    }	    
	}

	Table::instance ().setCurrentSpace (currentSpace);
	Table::instance ().templateNamespace () = last;
	if (!done) return NULL;
	auto ret = new (Z0) IStructInfo (this, this-> space, this-> name, this-> _udas, true);
	ret-> isConst (false);
	ret-> setTypes (types);
	ret-> setAttribs (attribs);
	ret-> setTmps (this-> tmpsDone);
	
	ret-> multFoo = &StructUtils::InstCallUnion;	
	score-> dyn = true;
	score-> ret = ret;
	return score;
    }
        
    ApplicationScore IStructCstInfo::CallType (Word token, syntax::ParamList params) {
	if (this-> tmps.size () != 0) {
	    return NULL;
	}
	
	if (this-> _isUnion) return CallOpUnion (token, params);
	if (params-> getParams ().size () != this-> params.size ())
	    return NULL;

	std::vector <InfoType> types;
	std::vector <std::string> attribs;

	auto last = Table::instance ().templateNamespace ();
	auto currentSpace = Table::instance ().space ();
	auto score = new (Z0) IApplicationScore (token);
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    Table::instance ().setCurrentSpace (this-> space);
	    Table::instance ().templateNamespace () = currentSpace;
	    InfoType info = this-> params [it]-> getType ();
	    if (info == NULL) return NULL;
	    types.push_back (info);
	    attribs.push_back (this-> params [it]-> token.getStr ());

	    auto type = params-> getParams () [it]-> info-> type ()-> CompOp (info);
	    if (type) type = type-> ConstVerif (info);
	    if (type) {
		type-> isConst (info-> isConst ());
		score-> score += 1;
		score-> treat.push_back (type);
	    } else {
		Table::instance ().setCurrentSpace (currentSpace);
		Table::instance ().templateNamespace () = last;		
		return NULL;
	    }
	}

	Table::instance ().setCurrentSpace (currentSpace);
	Table::instance ().templateNamespace () = last;
	auto ret = new (Z0) IStructInfo (this, this-> space, this-> name, this-> _udas, this-> _isUnion);
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
	static std::map <std::string, StructInfo> inProgress;
	static std::map <std::string, StructInfo> validated;
	
	if (this-> tmps.size () != 0) {
	    return this-> getScore (tmps);
	}

	if (this-> tmps.size () != tmps.size ()) return NULL;
	
	auto name = Namespace (this-> space, this-> onlyNameTypeString ()).toString ();
	auto valid = validated.find (name);
	
	if (valid != validated.end ()) return valid-> second-> clone ();	
	auto inside = inProgress.find (name);
	if (inside != inProgress.end ()) return inside-> second;
	
	std::vector <InfoType> types;
	std::vector <std::string> attribs;
	auto last = Table::instance ().templateNamespace ();
	auto currentSpace = Table::instance ().space ();
	this-> _info = new (Z0) IStructInfo (this, this-> space, this-> name, this-> _udas, this-> _isUnion);
	inProgress [name] = this-> _info;
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    Table::instance ().setCurrentSpace (this-> space);
	    Table::instance ().templateNamespace () = currentSpace;
	    InfoType info = this-> params [it]-> getType ();
	    if (info) {
		if (recursiveGet (this-> _info, info)) {
		    Table::instance ().setCurrentSpace (currentSpace);
		    Table::instance ().templateNamespace () = last;
		    Ymir::Error::recursiveNoSize (this-> params [it]-> token);
		    inProgress [name] = NULL;
		    return NULL;
		}
		types.push_back (info);
		attribs.push_back (this-> params [it]-> token.getStr ());
	    } else {
		Table::instance ().setCurrentSpace (currentSpace);
		Table::instance ().templateNamespace () = last;
		inProgress [name] = NULL;
		return NULL;
	    }
	}
	    
	Table::instance ().setCurrentSpace (currentSpace);
	Table::instance ().templateNamespace () = last;
	this-> _info-> isConst (false);
	this-> _info-> setTypes (types);
	this-> _info-> setAttribs (attribs);	    
	this-> _info-> setTmps (this-> tmpsDone);
	
	inProgress.erase (name);
	// validated [name] = this-> _info;
	return this-> _info-> clone ();
    }

    bool IStructCstInfo::recursiveGet (InfoType who, InfoType where) {
	if (who-> isSame (where)) return true;
	if (auto str = where-> to <IStructInfo> ()) {
	    for (auto it : str-> getTypes ()) {
		if (recursiveGet (who, it)) return true;
	    }
	    return false;
	} else if (auto tu = where-> to <ITupleInfo> ()) {
	    for (auto it : tu-> getParams ()) {
		if (recursiveGet (who, it)) return true;
	    }
	    return false;
	} else if (auto agg = where-> to <IAggregateInfo> ()) {
	    return recursiveGet (who, agg-> getImpl ());
	} else if (auto cstr = where-> to <IStructCstInfo> ()) {
	    return recursiveGet (who, cstr-> TempOp ({}));
	} else if (auto cagg = where-> to <IAggregateCstInfo> ()) {
	    return recursiveGet (who, cagg-> TempOp ({}));
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
	auto ret = new (Z0) IStructCstInfo (this-> _locId, this-> space, this-> name, ignore, this-> _udas, this-> _isUnion);
	ret-> params = params;	
	//ret-> tmpsDone = getValues (res.elements, this-> tmps);
	std::vector <syntax::Expression> tmpsDone = TemplateSolver::instance ().solved (this-> tmps, res);
	ret-> tmpsDone = tmpsDone;
	
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
	auto last = Table::instance ().templateNamespace ();
	auto currentSpace = Table::instance ().space ();
	for (auto i : Ymir::r (0, this-> params.size ())) {
	    Table::instance ().setCurrentSpace (this-> space);
	    Table::instance ().templateNamespace () = currentSpace;
	    auto it = this-> params [i];
	    auto t = it-> getType ();
	    if (t != NULL) 
		buf.write (t-> innerTypeString ());	
	    else buf.write ("undef");		    
	    if (i < (int) this-> params.size () - 1)
		buf.write (", ");		
	}
	buf.write ("}");
	Table::instance ().setCurrentSpace (currentSpace);
	Table::instance ().templateNamespace () = last;
	return buf.str ();
    }
    
    std::string IStructCstInfo::innerTypeString () {
	static std::map <std::string, bool> inner;
	auto last = Table::instance ().templateNamespace ();
	auto currentSpace = Table::instance ().space ();
	Ymir::OutBuffer buf (this-> space.toString (), ".", this-> name);
	auto inside = inner.find (buf.str ());
	if (inside == inner.end () || !inside-> second) {
	    std::string name = buf.str ();	    
	    inner [name] = true;
	    buf.write ("{");
	    for (auto i : Ymir::r (0, this-> params.size ())) {
		Table::instance ().setCurrentSpace (this-> space);
		Table::instance ().templateNamespace () = currentSpace;
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
	Table::instance ().setCurrentSpace (currentSpace);
	Table::instance ().templateNamespace () = last;
	return buf.str ();
    }

    std::string IStructCstInfo::innerSimpleTypeString () {
	return "";
    }

    std::vector<Word> & IStructCstInfo::udas () {
	return this-> _udas;
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

    Word IStructCstInfo::getLocId () {
	return this-> _locId;
    }
    
    IStructInfo::IStructInfo (StructCstInfo id, Namespace space, std::string name, vector <Word> udas, bool isUnion) :
	IInfoType (true),
	space (space),
	name (name),
	_id (id),
	_udas (udas),
	_isUnion (isUnion)
    {}

    bool IStructInfo::isSame (InfoType other) {
	if (auto ot = other-> to <IStructInfo> ()) {
	    if (this-> space == ot-> space && this-> name == ot-> name && this-> _id-> getLocId ().isSame (ot-> _id-> getLocId ())) {		    
		if (this-> tmpsDone.size () != ot-> tmpsDone.size ()) {
		    return false;
		}
		    
		for (auto it : Ymir::r (0, this-> tmpsDone.size ())) {
		    if (auto ps = this-> tmpsDone [it]-> to <IParamList> ()) {
			if (auto ps2 = ot-> tmpsDone [it]-> to <IParamList> ()) {
			    if (ps-> getParams ().size () != ps2-> getParams ().size ()) {
				return false;
			    }
			    for (auto it_ : Ymir::r (0, ps-> getParams ().size ())) {
				if (!ps-> getParams ()[it_]-> info-> type ()-> isSame (ps2-> getParams () [it_]-> info-> type ())) {
				    return false;
				}
			    }
			} else {
			    return false;
			}
		    } else {
			if (ot-> tmpsDone [it]-> is <IParamList> ()) {
			    return false;
			}
			if (!this-> tmpsDone [it]-> info-> type ()-> isSame (ot-> tmpsDone [it]-> info-> type ())) {
			    return false;
			}
		    }
		}		    
		return true;
	    }
	}
	return false;
    }    
    
    InfoType IStructInfo::ConstVerif (InfoType other) {	
	auto str = other-> to <IStructInfo> ();
	if (str == NULL || !str-> isSame (this)) return NULL;
	
	static std::vector <InfoType> dones;
	if (std::find (dones.begin (), dones.end (), this) == dones.end ()) {
	    dones.push_back (this);
	    for (auto it : Ymir::r (0, this-> types.size ())) {
		if (!this-> types [it]-> ConstVerif (str-> types [it])) {
		    if (!str-> needKeepConst ())
			return NULL;
		}
	    }
	} 
	return this;
    }

    bool IStructInfo::passingConst (InfoType other) {
	if (IInfoType::passingConst (other)) return true;
	else if (auto type = other-> to <IStructInfo> ()) {
	    static std::vector <InfoType> dones;
	    if (std::find (dones.begin (), dones.end (), this) == dones.end ()) {
		dones.push_back (this);
		for (auto it : Ymir::r (0, this-> types.size ())) {
		    if (this-> types [it]-> passingConst (type-> types [it]))
			return true;
		}
		dones.erase (std::find (dones.begin (), dones.end (), this));
	    }
	}
	return false;
    }
    
    InfoType IStructInfo::onClone () {
	auto ret = new (Z0) IStructInfo (this-> _id, this-> space, this-> name, this-> _udas, this-> _isUnion);
	ret-> setAttribs (this-> attrs);
	for (auto it : this-> types) {
	    ret-> types.push_back (it);
	}
	
	ret-> tmpsDone = this-> tmpsDone;
	ret-> isConst (this-> isConst ());
	return ret;
    }

    ApplicationScore IStructInfo::CallType (Word token, syntax::ParamList params) {
	if (params-> getParams ().size () != this-> types.size ())
	    return NULL;
	
	if (!this-> isType ()) return NULL;

	std::vector <InfoType> types;
	std::vector <std::string> attribs;
	
	auto score = new (Z0) IApplicationScore (token);
	for (auto it : Ymir::r (0, this-> attrs.size ())) {
	    InfoType info = this-> types [it];
	    if (info == NULL) return NULL;
	    types.push_back (info);
	    attribs.push_back (this-> attrs [it]);

	    info = info-> clone ();
	    if (this-> isConst ()) info-> isConst (true);
	    auto type = params-> getParams () [it]-> info-> type ()-> CompOp (info);	    
	    if (type) type = type-> ConstVerif (info);
	    if (type) {
		type-> isConst (info-> isConst ());
		score-> score += 1;
		score-> treat.push_back (type);
	    } else return NULL;
	}

	auto ret = new (Z0) IStructInfo (this-> _id, this-> space, this-> name, this-> _udas, this-> _isUnion);
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
	if (var-> token == "name") return Name ();
	return NULL;
    }

    InfoType IStructCstInfo::Name () {
	auto str = new (Z0) IStringInfo (true);
	str-> value () = new (Z0)  IStringValue (this-> name.c_str ());
	return str;
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
	    auto ret = this-> clone ();
	    ret-> isConst (this-> isConst ());
	    ret-> binopFoo = &StructUtils::InstCast;
	    return ret;
	} else if (other-> is <IUndefInfo> ()) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &StructUtils::InstCast;	    
	    return ret;
	} else if (auto ref = other-> to<IRefInfo> ()) {
	    if (this-> isLvalue () && this-> isSame (ref-> content ())) {
		auto ret = new (Z0)  IRefInfo (false, ref-> content ()-> clone ());
		ret-> content ()-> isConst (this-> isConst ());
		ret-> binopFoo = &StructUtils::InstAddr;
		return ret;
	    }
	}
	return NULL;
    }

    InfoType IStructInfo::UnaryOp (Word op) {
	if (op == Token::AND && this-> isLvalue ()) {
	    auto ret = new (Z0) IPtrInfo (this-> isConst (), this-> clone ());
	    ret-> binopFoo = &StructUtils::InstAddr;
	    return ret;
	}
	return NULL;
    }
    
    InfoType IStructInfo::BinaryOp (Word op, Expression right) {
	if (op == Token::EQUAL && right-> info-> type ()-> isSame (this)) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &StructUtils::InstAffect;
	    return ret;	    
	}
	return NULL;
    }
    
    InfoType IStructInfo::BinaryOpRight (Word op, Expression left) {
	if (op == Token::EQUAL && left-> info-> type ()-> is <IUndefInfo> ()) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &StructUtils::InstAffect;
	    return ret;	    
	}
	return NULL;
    }
    
    std::string IStructInfo::onlyNameTypeString (bool simple) {
	Ymir::OutBuffer buf (this-> space.toString (), ".", this-> name);
	if (this-> tmpsDone.size () != 0) buf.write ("!(");
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
	if (this-> tmpsDone.size () != 0) buf.write (")");
	if (simple)
	    buf.write ("S");
	return buf.str ();
    }

    std::string IStructInfo::innerTypeString () {
	static std::map <InfoType, bool> dones;
	if (dones.find (this) == dones.end ()) {
	    dones [this] = true;
	    auto ret = this-> onlyNameTypeString (false);
	    dones.erase (this);
	    return ret;
	} else return "_";
    }

    vector <Word> & IStructInfo::udas () {
	return this-> _udas;
    }
    
    Namespace& IStructInfo::getSpace () {
	return this-> space;
    }
    
    std::string IStructInfo::getName () {
	return this-> name;
    }
    
    std::string IStructInfo::innerSimpleTypeString () {
	std::string buf = this-> onlyNameTypeString ();
	return Ymir::OutBuffer (buf.length (), buf).str ();
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
	ulong current = 0;
	for (auto it : Ymir::r (0,  this-> tmpsDone.size ())) {
	    if (auto par = this-> tmpsDone [it]-> to <IParamList> ()) {
		if (current <= nb && nb < current + par-> getParams ().size ()) {
		    auto index = nb - current;
		    return par-> getParamTypes () [index]-> clone ();
		} else {
		    current += par-> getParams ().size ();
		}
	    } else {
		if (current == nb)
		    return this-> tmpsDone [current]-> info-> type ();
		current ++;
	    }
	}
	return NULL;
    }

    std::vector <InfoType> IStructInfo::getTemplate (ulong bef, ulong af) {
	std::vector <InfoType> types;
	ulong it = bef;
	while (true) {
	    auto tmp = this-> getTemplate (it);		
	    if (tmp == NULL) break;
	    types.push_back (tmp);		
	    it ++;
	}
	
	if ((int) types.size () - (int) af < 0) return {NULL};
	types.resize (types.size () - af);	
	return types;    
    }
    
    std::vector <std::string> & IStructInfo::getAttribs () {
	return this-> attrs;
    }

    std::vector <InfoType> & IStructInfo::getTypes () {
	return this-> types;
    }

    InfoType IStructInfo::typeOfAttrib (const std::string & name) {
	for (int i = 0 ; i < (int) this-> attrs.size () ; i++) {
	    if (this-> attrs [i] == name) return this-> types [i];
	}
	return NULL;
    }

    bool IStructInfo::needKeepConst () {
	if (this-> isConst ()) {
	    static std::vector <InfoType> dones;
	    if (std::find (dones.begin (), dones.end (), this) == dones.end ()) {
		dones.push_back (this);
		for (auto it : this-> types) {
		    if (it-> cloneConst ()-> needKeepConst ()) return true;
		}
		dones.erase (std::find (dones.begin (), dones.end (), this));
	    }
	}
	return false;
    }
    
    bool IStructInfo::has (std::string attr) {
	for (auto it : this-> _udas)
	    if (it == attr) return true;
	return false;
    }
    
    InfoType IStructInfo::StringOf () {
	auto str = new (Z0) IStringInfo (true);
	str-> value () = new (Z0)  IStringValue (this-> onlyNameTypeString ().c_str ());
	return str;
    }
    
    Ymir::Tree IStructInfo::toGeneric () {
	IInfoType::printConst (false);
	auto name = this-> innerTypeString ();
	IInfoType::printConst (true);
	auto str_type_node = IFinalFrame::getDeclaredType (name.c_str ());
	if (str_type_node.isNull ()) {
	    if (this-> types.size () != 0) {
		if (this-> _isUnion)
		    str_type_node = Ymir::makeUnion (name, this-> types, this-> attrs);
		else str_type_node = Ymir::makeTuple (name, this-> types, this-> attrs, this-> has (Keys::PACKED));
		IFinalFrame::declareType (name, str_type_node);	    
	    } else str_type_node = Ymir::makeTuple (name, {new (Z0) ICharInfo (true)}, {"_"});
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
