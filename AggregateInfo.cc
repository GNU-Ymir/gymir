#include <ymir/semantic/object/AggregateInfo.hh>
#include <ymir/ast/Function.hh>
#include <ymir/ast/TypeCreator.hh>
#include <ymir/ast/Var.hh>
#include <ymir/ast/Block.hh>
#include <ymir/semantic/object/MethodInfo.hh>
#include <ymir/ast/ParamList.hh>
#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/tree/_.hh>
#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/ast/TreeExpression.hh>
#include <ymir/utils/Mangler.hh>
#include <ymir/semantic/types/RefInfo.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/utils/StructUtils.hh>
#include <ymir/semantic/pack/ConstructFrame.hh>
#include <ymir/ast/TypedVar.hh>
#include <ymir/ast/Binary.hh>
#include <ymir/semantic/value/BoolValue.hh>
#include <set>


using namespace syntax;   
using namespace std;

namespace semantic {

    namespace AggregateUtils {
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
	
	Tree InstGetStaticMeth (Word, InfoType, Expression) {
	    return Tree ();
	}

	Tree InstAncestor (Word, InfoType, Expression elem, Expression) {
	    return elem-> toGeneric ();
	}
	
	Tree InstGetMethod (Word, InfoType, Expression left, Expression) {
	    return left-> toGeneric ();
	}

	Ymir::Tree InstAddr (Word locus, InfoType, Expression elem, Expression) {
	    return Ymir::getAddr (locus.getLocus (), elem-> toGeneric ());
	}
	
	Tree InstGetVtableCTE (Word, InfoType, Expression elem) {
	    auto rtype = elem-> info-> type ()-> to <IAggregateInfo> ();
	    auto vtable = rtype-> getVtable ();
	    return Ymir::getAddr (vtable);
	}

	Tree InstGetVtable (Word locus, InfoType, Expression elem, Expression) {
	    auto ltree = elem-> toGeneric ();
	    return getField (locus.getLocus (), ltree, Keys::VTABLE_FIELD);
	}
	
	Tree InstCompVtable (Word locus, InfoType, Expression left, Expression right) {
	    auto rtype = right-> info-> type ()-> to <IAggregateInfo> ();
	    auto vtable = rtype-> getVtable ();
	    auto ltree = left-> toGeneric ();
	    
	    auto ltable = getField (locus.getLocus (), ltree, Keys::VTABLE_FIELD);
	    tree_code code = OperatorUtils::toGeneric (locus);
	    return Ymir::buildTree (
		code, locus.getLocus (), boolean_type_node, ltable, vtable
	    );
	}

	Tree InstIsTypedStatic (Word locus, InfoType type, Expression left, Expression right) {
	    auto ltable = InstGetVtable (locus, type, left, NULL);

	    auto ptr_type = build_pointer_type (void_type_node);

	    // The table info is the first element of the vtable
	    auto ltype = Ymir::getPointerUnref (locus.getLocus (), ltable, ptr_type, 0);
	    
	    auto rtype = Ymir::getAddr (right-> info-> type ()-> genericTypeInfo ());
	    return callLib (locus.getLocus (), Runtime::COMPARE_TYPEINFO, boolean_type_node, {ltype, rtype});
	}

	Tree InstCastDyn (Word locus, InfoType type, Expression left, Expression) {
	    auto rtype = Ymir::getAddr (type-> genericTypeInfo ());
	    auto lelem = left-> toGeneric ();
	    auto innerType = type-> toGeneric ();
	    auto ptrType = build_pointer_type (innerType.getTree ());

	    Ymir::TreeStmtList list;
	    Ymir::Tree casted = Ymir::getExpr (list, callLib (locus.getLocus (), Runtime::DYNAMIC_CAST, ptrType, {Ymir::getAddr (lelem), rtype}));
	    
	    return Ymir::compoundExpr (locus.getLocus (),
				       list,
				       Ymir::getPointerUnref (locus.getLocus (), casted, innerType, 0));
	}
	
	Ymir::Tree InstCopyCstAff (Word loc, InfoType, Expression left, Expression right) {
	    auto rtree = right-> toGeneric ();
	    auto ltree = left-> toGeneric ();
	    Ymir::TreeStmtList list;
	    auto meth = right-> info-> type ()-> to <IAggregateInfo> ()-> cpyCstr ();
	    auto ptrl = Ymir::getAddr (loc.getLocus (), ltree).getTree ();
	    auto ptrr = Ymir::getAddr (loc.getLocus (), rtree).getTree ();
	    auto vtable = Ymir::getAddr (right-> info-> type ()-> to <IAggregateInfo> ()-> getVtable ());
	    auto vfield = Ymir::getField (loc.getLocus (), ltree, Keys::VTABLE_FIELD);
	    list.append (
		Ymir::buildTree (MODIFY_EXPR,
				 loc.getLocus (),
				 void_type_node,
				 vfield,
				 convert (vfield.getType ().getTree (), vtable.getTree ())
		)
	    );

	    auto fn = meth-> validate ()-> toGeneric ();
	    std::vector <tree> args = {ptrl, ptrr};
	    list.append (build_call_array_loc (loc.getLocus (),
					       void_type_node,
					       fn.getTree (),
					       2,
					       args.data ()));
	    
	    return Ymir::compoundExpr (loc.getLocus (), list.getTree (), ltree);	    
	}
	
	Ymir::Tree InstGetCpy (Word loc, InfoType type, Expression left, Expression) {
	    auto rtree = left-> toGeneric ();
	    Ymir::TreeStmtList list;
	    auto meth = type-> to <IAggregateInfo> ()-> cpyCstr ();
	    auto ltree = Ymir::makeAuxVar (loc.getLocus (), ISymbol::getLastTmp (), type-> toGeneric ());
	    auto ptrl = Ymir::getAddr (loc.getLocus (), ltree).getTree ();
	    auto ptrr = Ymir::getAddr (loc.getLocus (), rtree).getTree ();
	    auto vtable = Ymir::getAddr (type-> to <IAggregateInfo> ()-> getVtable ());
	    auto vfield = Ymir::getField (loc.getLocus (), ltree, Keys::VTABLE_FIELD);
	    list.append (
		Ymir::buildTree (MODIFY_EXPR,
				 loc.getLocus (),
				 void_type_node,
				 vfield,
				 convert (vfield.getType ().getTree (), vtable.getTree ())
		)
	    );

	    auto fn = meth-> validate ()-> toGeneric ();
	    std::vector <tree> args = {ptrl, ptrr};
	    list.append (build_call_array_loc (loc.getLocus (),
					       void_type_node,
					       fn.getTree (),
					       2,
					       args.data ()));
	    
	    return Ymir::compoundExpr (loc.getLocus (), list.getTree (), ltree);
	}
	
	Ymir::Tree InstCast (Word loc, InfoType type, Expression elem, Expression) {	    
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

	
	
    }
    
    IAggregateCstInfo::IAggregateCstInfo (Word locId, Namespace space, std::string name, const std::vector <syntax::Expression> & tmps, syntax::Expression over) :
	IInfoType (true),
	_space (space),
	_locId (locId),
	_name (name),
	_contrs ({}), _destr (NULL), _methods ({}), _staticMeth ({}),
	_tmps (tmps),
	_ancExpr (over),
	_templateSpace ("")
	
    {}

    std::vector <FunctionInfo> & IAggregateCstInfo::getConstructors () {
	return this-> _contrs;
    }

    std::vector <TypeAttr> & IAggregateCstInfo::getAttrs () {
	return this-> _attrs;
    }
    
    FunctionInfo & IAggregateCstInfo::getDestructor () {
	if (this-> _destr != NULL || !this-> _anc) {
	    return this-> _destr;
	} else {
	    return this-> _anc-> getDestructor ();
	}
    }

    std::vector <FunctionInfo> & IAggregateCstInfo::getMethods () {
	return this-> _methods;
    }

    std::vector <FunctionInfo> & IAggregateCstInfo::getStaticMethods () {
	return this-> _staticMeth;
    }

    std::vector <TypeAttr> & IAggregateCstInfo::getStaticVars () {
	return this-> _staticAttrs;
    }
    
    Block & IAggregateCstInfo::getStaticBlock () {
	return this-> _staticBlock;
    }

    Namespace IAggregateCstInfo::space () {
	return this-> _space;
    }

    Namespace& IAggregateCstInfo::templateSpace () {
	return this-> _templateSpace;
    }
    
    bool IAggregateCstInfo::isSame (InfoType other) {
	if (other == this) return true;
	if (auto ot = other-> to <IAggregateCstInfo> ()) {
	    if (ot-> _name == this-> _name && ot-> _space == this-> _space) {
		return true;
	    }
	}
	return false;
    }


    InfoType IAggregateCstInfo::onClone () {
	return this;
    }
    
    InfoType IAggregateCstInfo::TempOp (const std::vector <Expression> & tmps) {
	static std::map <std::string, AggregateInfo> inProgress;
	
	if (this-> _ancExpr) {
	    auto aggType = this-> _ancExpr-> toType ();
	    if (aggType == NULL) return NULL;
	    auto agg = aggType-> info-> type ()-> to <IAggregateInfo> ();
	    if (agg == NULL) {
		Ymir::Error::cannotOverride (this-> _locId, aggType-> info-> type ());
		return NULL;
	    } else this-> _anc = agg-> _id;
	}
	
	if (this-> _tmps.size () != 0) {
	    return this-> getScore (tmps);
	}

	if (this-> _tmps.size () != tmps.size ()) return NULL;

	auto name = Namespace (this-> _space, this-> _name).toString ();

	auto inside = inProgress.find (name);
	if (inside != inProgress.end ()) return inside-> second;
	
	auto last = Table::instance ().templateNamespace ();
	auto currentSpace = Table::instance ().space ();
	
	this-> _info = new (Z0) IAggregateInfo (this, this-> _space, this-> _name, {}, this-> _isExternal);
	inProgress [name] = this-> _info;

	AggregateInfo anc = NULL;
	if (this-> _anc) {
	    auto ancInfo = this-> _anc-> TempOp ({});
	    if (ancInfo == NULL) return NULL;
	    else anc = ancInfo-> to <IAggregateInfo> ();
	}
	
	if (this-> _anc && this-> _attrs.size () != 0 && !this-> _isDynamic) {
	    Ymir::Error::attributeInHeirStatic (this-> _attrs [0]-> getIdent ());
	    return NULL;
	}

	if (anc) {
	    for (auto it : anc-> getTypes ()) this-> _info-> getTypes ().push_back (it);
	    for (auto it : anc-> getAttrs ()) this-> _info-> getAttrs ().push_back (it);
	    for (auto it : anc-> getInnerProts ()) this-> _info-> getInnerProts ().push_back (it);
	    for (auto it : anc-> getAttrSpaces ()) this-> _info-> getAttrSpaces ().push_back (it);
	}

	auto typeSpace = Namespace (this-> _info-> innerTypeString ());
	for (auto it : Ymir::r (0, this-> _attrs.size ())) {
	    Table::instance ().setCurrentSpace (this-> _space);
	    Table::instance ().templateNamespace () = currentSpace;
	    
	    Type info = this-> _attrs [it]-> getType ()-> toType ();
	    if (info) {
		if (recursiveGet (this-> _info, info-> type ())) {
		    Table::instance ().setCurrentSpace (currentSpace);
		    Table::instance ().templateNamespace () = last;
		    Ymir::Error::recursiveNoSize (this-> _attrs [it]-> getIdent ());
		    inProgress [name] = NULL;
		    return NULL;
		}

		if (this-> _attrs [it]-> isConst ())
		    info-> type ()-> isConst (true);
		
		this-> _info-> getTypes ().push_back (info-> type ());
		this-> _info-> getAttrs ().push_back (this-> _attrs [it]-> getIdent ().getStr ());
		this-> _info-> getInnerProts ().push_back (this-> _attrs [it]-> getProtection ());
		this-> _info-> getAttrSpaces ().push_back (typeSpace);
	    } else {
		Table::instance ().setCurrentSpace (currentSpace);
		Table::instance ().templateNamespace () = last;
		inProgress [name] = NULL;
		return NULL;
	    }	    
	}

	Table::instance ().setCurrentSpace (currentSpace);
	Table::instance ().templateNamespace () = last;
		    
	if (this-> _destr)
	    this-> _info-> _destr = this-> _destr-> frame ();
	this-> _info-> _staticMeth = this-> _staticMeth;
	this-> _info-> _methods = this-> _methods;
	if (anc) this-> _info-> _anc = anc-> to <IAggregateInfo> ();
	
	this-> _info-> _allMethods = this-> _info-> getMethods ();

       
	inProgress.erase (name);	
	return this-> _info-> clone ();	    
    }

    InfoType IAggregateCstInfo::getScore (const std::vector <syntax::Expression> & tmps) {
	auto res = TemplateSolver::instance ().solve (this-> _tmps, tmps);
	if (!res.valid || !TemplateSolver::instance ().isSolved (this-> _tmps, res))
	    return NULL;
	
	auto tmpsDone = TemplateSolver::instance ().solved (this-> _tmps, res);
	auto ret = findAlreadySolve (tmpsDone);
	if (!ret) {
	    auto creator = this-> _creator-> templateDeclReplace (res.elements)-> to <ITypeCreator> ();
	    ret = creator-> declare (this-> _space, tmpsDone)-> to <IAggregateCstInfo> ();
	    addAlreadySolve (tmpsDone, ret);
	}
	
	return ret;
    }

    InfoType IAggregateCstInfo::findAlreadySolve (std::vector <syntax::Expression>& tmpsDone) {
	for (auto it_ : Ymir::r (0, this-> _solvedTmps.size ())) {
	    auto tmps = this-> _solvedTmps [it_];
	    if (tmpsDone.size () != tmps.size ()) continue;
	    bool success = true;
	    for (auto it : Ymir::r (0, tmpsDone.size ())) {
		if (auto ps = tmpsDone  [it]-> to <IParamList> ()) {
		    if (auto ps2 = tmps [it]-> to <IParamList> ()) {
			if (ps-> getParams ().size () != ps2-> getParams ().size ()) {
			    success = false;
			    break;
			}
			for (auto it_ : Ymir::r (0, ps-> getParams ().size ())) {
			    if (!ps-> getParams ()[it_]-> info-> type ()-> isSame (ps2-> getParams () [it_]-> info-> type ())) {
				success = false;
				break;
			    }
			}
		    } else {
			success = false;
			break;	   
		    }
		} else {
		    if (tmpsDone [it]-> is <IParamList> ()) {
			success = false; break;
		    }
		    if (!tmpsDone [it]-> info-> type ()-> isSame (tmps [it]-> info-> type ())) {
			success = false; break;
		    }
		}		
	    }
	    if (success) return this-> _solved [it_];
	}
	return NULL;
    }

    void IAggregateCstInfo::addAlreadySolve (std::vector <syntax::Expression> & tmpsDone, InfoType type) {
	this-> _solvedTmps.push_back (tmpsDone);
	this-> _solved.push_back (type);
    }
    
    bool IAggregateCstInfo::recursiveGet (InfoType who, InfoType where) {
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
	    for (auto it : agg-> getTypes ()) {
		if (recursiveGet (who, it)) return true;
	    }
	    return false;
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
    
    InfoType IAggregateCstInfo::SizeOf () {
	if (this-> TempOp ({}) != NULL) {
	    auto ret = new (Z0) IFixedInfo (true, FixedConst::UINT);
	    ret-> unopFoo = StructUtils::InstSizeOfCst;
	    return ret;
	}
	
	return NULL;
    }

    InfoType IAggregateCstInfo::DColonOp (Var var) {
	if (var-> token == "init") return Init (var);
	if (var-> token == "sizeof") return SizeOf ();
	if (var-> token == "typeid"){
	    if (auto str = this-> TempOp ({}))		
		return str-> StringOf ();
	    return NULL;
	}
	std::vector <Frame> frames;
	std::vector <Word> hasPrivate;
	for (auto it : this-> _staticMeth) {
	    if (it-> name () == var-> token.getStr ()) {
		bool isPrivate = false;
		if (it-> frame ()-> isInnerPrivate () && (!this-> isMine (it-> frame ()-> space ()) || (!this-> inPrivateContext ()))) {
		    isPrivate = true;
		} else if (it-> frame ()-> isInnerProtected () && (!this-> isProtectedForMe (it-> frame ()-> space ()) || (!this-> inProtectedContext ()))) {
		    isPrivate = true;
		}

		if (!isPrivate) {
		    frames.push_back (it-> frame ());
		} else {
		    hasPrivate.push_back (it-> frame ()-> ident ());
		}
	    }	    		
	}

	if (frames.size () == 0 && hasPrivate.size () != 0) {
	    for (auto it : hasPrivate)
		Ymir::Error::privateMemberWithinThisContext (this-> typeString (), it);
	    return NULL;
	} else if (frames.size () != 0) {
	    auto ret = new (Z0) IFunctionInfo (this-> _space, var-> token.getStr (), frames);
	    ret-> unopFoo = &AggregateUtils::InstGetStaticMeth;
	    return ret;
	}
	
	return NULL;
    }
    
    std::string IAggregateCstInfo::typeString () {
	if (this-> _tmpsDone.size () == 0) 
	    return Namespace (this-> _space, this-> _name).toString ();
	else {
	    Ymir::OutBuffer buf (Namespace(this-> _space, this-> _name).toString ());
	    buf.write ("!(");
	    int i = 0;
	    for (auto it : this-> _tmpsDone) {
		if (i != 0) buf.write (", ");
		if (it-> info) {
		    if (it-> info-> isImmutable ()) buf.write (it-> info-> value ()-> toString ());
		    else if (auto tu = it-> info-> type ()-> to <ITupleInfo> ()) {
			if (tu-> isFake ()) {
			    buf.write ("{");
			    int i = 0;
			    for (auto it : tu-> getParams ()) {
				if (i != 0) buf.write (", ");
				buf.write (it-> value ()-> toString ());
				i += 1;
			    }
			    buf.write ("}");
			} else buf.write (it-> info-> typeString ());
		    } else buf.write ( it-> info-> typeString ());
		}
	    }
	    buf.write (").", this-> _templateSpace.toString ());
	    return buf.str ();
	}
    }

    std::string IAggregateCstInfo::simpleTypeString () {
	if (this-> _tmpsDone.size () == 0) 
	    return Namespace (this-> _space, this-> _name + "A").toString ();
	else {
	    Ymir::OutBuffer buf (Namespace(this-> _space, this-> _name).toString ());
	    buf.write ("!(");
	    int i = 0;
	    for (auto it : this-> _tmpsDone) {
		if (i != 0) buf.write (", ");
		if (it-> info) {
		    if (it-> info-> isImmutable ()) buf.write (it-> info-> value ()-> toString ());
		    else if (auto tu = it-> info-> type ()-> to <ITupleInfo> ()) {
			if (tu-> isFake ()) {
			    buf.write ("{");
			    int i = 0;
			    for (auto it : tu-> getParams ()) {
				if (i != 0) buf.write (", ");
				buf.write (it-> value ()-> toString ());
				i += 1;
			    }
			    buf.write ("}");
			} else buf.write (it-> info-> simpleTypeString ());
		    } else buf.write ( it-> info-> simpleTypeString ());
		}
	    }
	    buf.write (")A.", this-> _templateSpace.toString ());
	    return buf.str ();
	}
    }
    
    std::string IAggregateCstInfo::innerTypeString () {
	return simpleTypeString ();
    }    
    
    std::string IAggregateCstInfo::innerSimpleTypeString () {
	if (this-> _tmpsDone.size () == 0) {
	    return Namespace (this-> _space, this-> _name + "A").toString ();   
	} else {
	    Ymir::OutBuffer buf (Namespace(this-> _space, this-> _name).toString ());
	    buf.write (Mangler::mangle_template_list (this-> _tmpsDone));
	    buf.write ("A.", this-> _templateSpace.toString ());
	    return buf.str ();
	}
    }

    Ymir::Tree IAggregateCstInfo::toGeneric () {
	Ymir::Error::assert ("TODO");
	return Ymir::Tree ();
    }

    bool IAggregateCstInfo::isType () {
	return true;
    }

    bool& IAggregateCstInfo::isExtern () {
	return this-> _isExternal;
    }
    
    std::string IAggregateCstInfo::name () {
	return this-> _name;
    }

    bool IAggregateCstInfo::isSuccessor (AggregateCstInfo info) {
	if (this-> _anc) {
	    if (this-> _anc-> isSame (info)) return true;
	    else return this-> _anc-> isSuccessor (info);
	} else return false;
    }

    TypeCreator& IAggregateCstInfo::creator () {
	return this-> _creator;
    }

    std::vector <Expression> & IAggregateCstInfo::tmpsDone () {
	return this-> _tmpsDone;
    }
    
    const char* IAggregateCstInfo::getId () {
	return IAggregateCstInfo::id ();
    }

    Word IAggregateCstInfo::getLocId () {
	return this-> _locId;	
    }
    
    InfoType IAggregateCstInfo::Init (Var var) {
	if (this-> _contrs.size () == 0) {
	    return NULL;
	} else {
	    std::vector <Frame> frames;
	    bool hasPrivate = false;
	    for (auto it : this-> _contrs) {
		if (it-> frame ()-> isInnerPrivate () && !this-> inPrivateContext ()) {
		    hasPrivate = true;
		} else if (it-> frame ()-> isInnerProtected () && !this-> inProtectedContext ()) {
		    hasPrivate = true;
		} else 
		    frames.push_back (it-> frame ());
	    }
	    
	    if (hasPrivate) {
		Ymir::Error::privateMemberWithinThisContext (this-> typeString (), var-> token);
		return NULL;
	    } else {	    
		auto ret = new (Z0) IFunctionInfo (this-> _space, "init", frames);
		ret-> isConstr () = true;
		return ret;
	    }
	}
    }

    bool IAggregateCstInfo::inProtectedContext () {
	auto space = Table::instance ().getCurrentSpace ();
	string name_ = "";
	auto myspace = Namespace (this-> innerTypeString ());
	if (myspace.isSubOf (space)) return true;
	else if (this-> _anc) {
	    auto ret = this-> _anc-> inPrivateContext ();
	    if (ret) return true;
	}
	
	return false;
    }
    
    bool IAggregateCstInfo::inPrivateContext () {
	auto space = Table::instance ().getCurrentSpace ();
	auto myspace = Namespace (this-> innerTypeString ());
	return myspace.isSubOf (space);
    }


    bool IAggregateCstInfo::isMine (Namespace space) {
	return Namespace (this-> typeString ()) == space;
    }

    bool IAggregateCstInfo::isProtectedForMe (Namespace space) {
	if (isMine (space)) return true;
	if (this-> _anc) return this-> _anc-> isProtectedForMe (space);
	return false;
    }

    
    IAggregateInfo::IAggregateInfo (AggregateCstInfo from, Namespace space, std::string name, const std::vector <syntax::Expression> & tmpsDone, bool isExtern) :
	IInfoType (false),
	_space (space),
	_name (name),
	tmpsDone (tmpsDone),
	_id (from),
	_isExternal (isExtern)
    {}

    bool IAggregateInfo::isSame (InfoType other) {
	if (auto ot = other-> to <IAggregateInfo> ()) {
	    if (this-> _space == ot-> _space && this-> _name == ot-> _name && this-> _id-> getLocId ().isSame (ot-> _id-> getLocId ())) {
		if (this-> _id-> tmpsDone ().size () != ot-> _id-> tmpsDone ().size ()) {
		    return false;		    
		}
		
		for (auto it : Ymir::r (0, this-> _id-> tmpsDone ().size ())) {
		    if (auto ps = this-> _id-> tmpsDone () [it]-> to <IParamList> ()) {
			if (auto ps2 = ot-> _id-> tmpsDone () [it]-> to <IParamList> ()) {
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
			if (ot-> _id-> tmpsDone () [it]-> is <IParamList> ()) {
			    return false;
			}
			if (!this-> _id-> tmpsDone () [it]-> info-> type ()-> isSame (ot-> _id-> tmpsDone () [it]-> info-> type ())) {
			    return false;
			}
		    }
		}		    
		return true;
	    }
	}
	return false;
    }

    Frame IAggregateInfo::getDestructor () {
	if (this-> _destr || !this-> _anc)
	    return this-> _destr;
	else return this-> _anc-> getDestructor ();
    }   
    
    InfoType IAggregateInfo::ConstVerif (InfoType type) {
	while (auto ref = type-> to <IRefInfo> ())
	    type = ref-> content ();
	
	if (auto aggr = type-> to <IAggregateInfo> ()) {
	    if (!this-> isSame (aggr)) return NULL;
	    static std::set <InfoType> dones;
	    if (dones.find (this) == dones.end ()) {
		dones.insert (this);
		for (auto it : Ymir::r (0, this-> _types.size ())) {
		    if (!this-> _types [it]-> ConstVerif (aggr-> _types [it])) {
			if (!aggr-> needKeepConst ()) {
			    dones.erase (this);
			    return NULL;
			}
		    }
		}
		dones.erase (this);
	    }
	    return this;
	}
	return NULL;
    }

    bool IAggregateInfo::needKeepConst () {
	if (this-> isConst ()) {
	    static std::vector <InfoType> dones;
	    if (std::find (dones.begin (), dones.end (), this) == dones.end ()) {
		dones.push_back (this);
		for (auto it : this-> _types) {
		    if (it-> cloneConst ()-> needKeepConst ()) return true;
		}
		dones.erase (std::find (dones.begin (), dones.end (), this));
	    }
	}
	return false;
    }

    InfoType IAggregateInfo::onClone () {
	static std::map <InfoType, InfoType> dones;
	if (dones.find (this) == dones.end ()) {
	    auto ret = new (Z0) IAggregateInfo (this-> _id, this-> _space, this-> _name, this-> tmpsDone, this-> _isExternal);
	    dones [this] = ret;

	    ret-> _destr = this-> _destr;
	    ret-> _staticMeth = this-> _staticMeth;
	    ret-> _methods = this-> _methods;
	    ret-> _allMethods = this-> _allMethods;

	    for (auto it : Ymir::r (0, this-> _types.size ())) {
		ret-> _attrs.push_back (this-> _attrs [it]);
		ret-> _types.push_back (this-> _types [it]);
		ret-> _prots.push_back (this-> _prots [it]);
		ret-> _attrSpaces.push_back (this-> _attrSpaces [it]);
	    }
	    
	    if (this-> _anc)
		ret-> _anc = this-> _anc;
	    ret-> isConst (this-> isConst ());

	    dones.erase (this);	    
	    return ret;
	} else {
	    return dones [this];
	}
	
    }

    InfoType IAggregateInfo::BinaryOp (Word op, Expression right) {
	if (op == Token::EQUAL && right-> info-> type ()-> isSame (this)) {
	    auto ret = this-> clone ();
	    if (this-> hasCopyCstr ()) 
	    	ret-> binopFoo = &AggregateUtils::InstCopyCstAff;
	    else 
		ret-> binopFoo = &StructUtils::InstAffect;	    
	    return ret;
	}
	return NULL;
    }

    InfoType IAggregateInfo::BinaryOpRight (Word op, Expression left) {
	if (op == Token::EQUAL && left-> info-> type ()-> is <IUndefInfo> ()) {	    
	    auto ret = this-> clone ();
	    if (this-> hasCopyCstr ())
		ret-> binopFoo = &AggregateUtils::InstCopyCstAff;
	    else 
		ret-> binopFoo = &StructUtils::InstAffect;
	    return ret;
	} else if (op == Token::EQUAL && left-> info-> type ()-> is <IAggregateInfo> ()) {
	    if (left-> info-> type ()-> isSame (this)) {
		auto ret = left-> info-> type ()-> clone ();
		if (this-> hasCopyCstr ())
		    ret-> binopFoo = &AggregateUtils::InstCopyCstAff;
		else
		    ret-> binopFoo = &StructUtils::InstAffect;
		return ret;
	    } else if (this-> getAncestor () != NULL) {
		return this-> getAncestor ()-> BinaryOpRightCpy (op, left, this-> hasCopyCstr ());
	    }
	}
	return NULL;
    }

    InfoType IAggregateInfo::BinaryOpRightCpy (Word op, Expression left, bool hasCpy) {
	if (op == Token::EQUAL && left-> info-> type ()-> is <IUndefInfo> ()) {	    
	    auto ret = this-> clone ();
	    if (this-> hasCopyCstr ())
		ret-> binopFoo = &AggregateUtils::InstCopyCstAff;
	    else 
		ret-> binopFoo = &StructUtils::InstAffect;
	    return ret;
	} else if (op == Token::EQUAL && left-> info-> type ()-> is <IAggregateInfo> ()) {
	    if (left-> info-> type ()-> isSame (this)) {
		auto ret = left-> info-> type ()-> clone ();
		if (hasCpy)
		    ret-> binopFoo = &AggregateUtils::InstCopyCstAff;
		else
		    ret-> binopFoo = &StructUtils::InstAffect;
		return ret;
	    } else if (this-> getAncestor () != NULL) {
		return this-> getAncestor ()-> BinaryOpRightCpy (op, left, this-> hasCopyCstr ());
	    }
	}
	return NULL;
    }
    
    InfoType IAggregateInfo::DotOp (Var var) {
	if (var-> token == Keys::VTABLE_FIELD) {
	    auto ret = new (Z0) IPtrInfo (true, new (Z0) IVoidInfo ());
	    ret-> binopFoo = AggregateUtils::InstGetVtable;
	    return ret;
	}

	bool hasPrivate = false;
	for (auto it : Ymir::r (0, this-> _attrs.size ())) {
	    if (var-> token == this-> _attrs [it]) {
		if (this-> _prots [it] == InnerProtection::PRIVATE && (!this-> isMine (this-> _attrSpaces [it]) || !this-> inPrivateContext ())) {
		    hasPrivate = true;
		} else if (this-> _prots [it] == InnerProtection::PROTECTED && (!this-> isProtectedForMe (this-> _attrSpaces [it]) || !this-> inProtectedContext ())) {
		    hasPrivate = true;
		} else {		
		    auto ret = this-> _types [it]-> clone ();
		    if (this-> isConst ()) {
			ret-> isConst (true);
		    } else ret-> isConst (this-> _types [it]-> isConst ());
		    ret = new (Z0) IArrayRefInfo (this-> isConst (), ret);
		    ret-> binopFoo = &StructUtils::InstGet;
		    return ret;
		}
	    }
	}
	
	if (!var-> hasTemplate () && var-> token.getStr () == Keys::SUPER && this-> getAncestor () != NULL)  {
	    if (this-> inPrivateContext ())
		return Super ();
	    else { Ymir::Error::privateMemberWithinThisContext (this-> typeString (), var-> token); return NULL; }
	} if (var-> token.getStr () == Keys::DISPOSE) {
	    auto ret = new (Z0) IMethodInfo (this, var-> token.getStr (), {this-> getDestructor ()}, {0}, true);
	    ret-> binopFoo = &AggregateUtils::InstGetMethod;
	    return ret;
	}
	
	auto fin = this-> Method (var);
	if (fin) return fin;
	else if (hasPrivate) {
	    Ymir::Error::privateMemberWithinThisContext (this-> typeString (), var-> token);
	}
	
	return NULL;
    }

    
    InfoType IAggregateInfo::DColonOp (Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "init") return Init (var);
	if (var-> token == "sizeof") return SizeOf ();
	if (var-> token == "typeid") return StringOf ();
	if (var-> token == Keys::VTABLE_FIELD) {
	    auto ret = new (Z0) IPtrInfo (true, new (Z0) IVoidInfo ());
	    ret-> unopFoo = AggregateUtils::InstGetVtableCTE;
	    return ret;
	} else if (var-> token == "typeinfo") return TypeInfo ();
	//if (var-> token == "name") return Name ();
	std::vector <Frame> frames;
	for (auto it : this-> _staticMeth) {
	    if (it-> name () == var-> token.getStr ()) {
		frames.push_back (it-> frame ());		
	    }	    		
	}
	
	if (frames.size () != 0) {
	    auto ret = new (Z0) IFunctionInfo (this-> _space, var-> token.getStr (), frames);
	    ret-> unopFoo = &AggregateUtils::InstGetStaticMeth;
	    return ret;
	}
	return NULL;
    }

    InfoType IAggregateInfo::CastOp (InfoType other) {
	if (this-> isSame (other)) {
	    return this;
	} else if (auto ret = this-> CompOp (other)) {
	    return ret;
	} else if (other-> is <IAggregateInfo> () && (other-> CompOp (this))) {
	    auto ret = other-> clone ();
	    ret-> isConst (this-> isConst ());
	    ret-> binopFoo = &AggregateUtils::InstCastDyn;
	    return ret;
	}
	return NULL;
    }

    InfoType IAggregateInfo::CompOp (InfoType other) {
	if (this-> isSame (other) || other-> is <IUndefInfo> ()) {
	    if (this-> hasCopyCstr ()) {
		auto ret = this-> clone ();
		//auto meth = new (Z0) IMethodInfo (this, Keys::COPY, {this-> cpyCstr ()}, {0}, false);
		ret-> isConst (this-> isConst ());
		ret-> binopFoo = AggregateUtils::InstGetCpy;
		return ret;
	    } else {
		auto ret = this-> clone ();
		ret-> isConst (this-> isConst ());
		ret-> binopFoo = &AggregateUtils::InstCast;
		return ret;
	    }
	} else if (auto ref = other-> to <IRefInfo> ()) {
	    if (this-> isSame (ref-> content ())) {
		auto ret = new (Z0) IRefInfo (this-> isConst (), ref-> content ()-> clone ());
		ret-> content ()-> isConst (this-> isConst ());
		ret-> binopFoo = &StructUtils::InstAddr;
		return ret;
	    }
	}

	if (this-> getAncestor ()) {
	    if (auto ret = this-> getAncestor ()-> CompOp (other)) {
		return ret;
	    }
	}
	return NULL;
    }

    InfoType IAggregateInfo::isTyped (AggregateInfo other) {
	if (this-> CompOp (other)) {
	    auto ret = new (Z0) IBoolInfo (true);
	    ret-> value () = new (Z0) IBoolValue (true);
	    return ret;
	} else if (other-> CompOp (this)) {
	    auto ret = new (Z0) IBoolInfo (true);
	    ret-> binopFoo = &AggregateUtils::InstIsTypedStatic;
	    return ret;
	} else return NULL;
    }
    
    InfoType IAggregateInfo::UnaryOp (Word op) {
	if (op == Token::AND && this-> isLvalue ()) {
	    auto ret = new (Z0) IPtrInfo (this-> isConst (), this-> clone ());
	    ret-> binopFoo = &AggregateUtils::InstAddr;
	    return ret;
	}
	return NULL;
    }
    
    std::string IAggregateInfo::getName () {
	return this-> _name;
    }

    Namespace & IAggregateInfo::getSpace () {
	return this-> _space;
    }

    std::string IAggregateInfo::innerTypeString () {
	if (this-> _id-> tmpsDone ().size () == 0) {
	    return Namespace (this-> _space, this-> _name).toString ();   
	} else {
	    Ymir::OutBuffer buf (Namespace(this-> _space, this-> _name).toString ());
	    buf.write ("!(");
	    int i = 0;
	    for (auto it : this-> _id-> tmpsDone ()) {
		if (i != 0) buf.write (", ");
		if (it-> info) {
		    if (it-> info-> isImmutable ()) buf.write (it-> info-> value ()-> toString ());
		    else if (auto tu = it-> info-> type ()-> to <ITupleInfo> ()) {
			if (tu-> isFake ())
			    for (auto it : tu-> getParams ())
				buf.write (it-> value ()-> toString ());
			else buf.write (it-> info-> typeString ());
		    } else buf.write ( it-> info-> typeString ());
		}
	    }
	    buf.write (").", this-> _id-> templateSpace ().toString ());
	    return buf.str ();
	}
    }

    std::string IAggregateInfo::innerSimpleTypeString () {
	if (this-> _id-> tmpsDone ().size () == 0) {
	    return Namespace (this-> _space, this-> _name + "A").toString ();   
	} else {
	    Ymir::OutBuffer buf (Namespace(this-> _space, this-> _name).toString ());
	    buf.write (Mangler::mangle_template_list (this-> _id-> tmpsDone ()));
	    buf.write ("A.", this-> _id-> templateSpace ().toString ());
	    return buf.str ();
	}
    }

    InfoType IAggregateInfo::SizeOf () {
	auto ret = new (Z0) IFixedInfo (true, FixedConst::UINT);
	ret-> unopFoo = StructUtils::InstSizeOf;
	return ret;
    }

    InfoType IAggregateInfo::Super () {
	auto ret = this-> _anc-> clone ()-> to <IAggregateInfo> ();
	ret-> binopFoo = AggregateUtils::InstAncestor;
	ret-> _static = true;
	return ret;
    }
    

    InfoType IAggregateInfo::Method (Var var) {
	std::vector <Frame> frames;
	std::vector <int> index;
	int i = 0;
	bool hasPrivate = false;
	
	for (auto it : this-> _allMethods) {
	    if (it-> name () == var-> token.getStr ()) {
		if (it-> frame ()-> isInnerPrivate () && (!this-> isMine (it-> frame ()-> space ()) || !this-> inPrivateContext ())) {
		    hasPrivate = true;
		} else if (it-> frame ()-> isInnerProtected () && (!this-> isProtectedForMe (it-> frame ()-> space ()) || !this-> inProtectedContext ())) {
		    hasPrivate = true;
		} else {
		    frames.push_back (it-> frame ());
		    index.push_back (i);
		}
	    }
	    
	    if (it-> isVirtual ())
		i ++;
	}

	if (frames.size () != 0) {
	    auto meth = new (Z0) IMethodInfo (this, var-> token.getStr (), frames, index, this-> _static);
	    meth-> binopFoo = AggregateUtils::InstGetMethod;	    
	    return meth;
	} else if (hasPrivate) {
	    Ymir::Error::privateMemberWithinThisContext (this-> typeString (), var-> token);
	}
	return NULL;
    }

    InfoType IAggregateInfo::Init (Var var) {
	if (this-> _id-> _contrs.size () == 0) {
	    return NULL;
	} else {
	    std::vector <Frame> frames;
	    bool hasPrivate = false;
	    for (auto it : this-> _id-> _contrs)
		if (it-> frame ()-> isInnerPrivate () && !this-> inPrivateContext ()) {
		    hasPrivate = true;
		} else if (it-> frame ()-> isInnerProtected () && !this-> inProtectedContext ()) {
		    hasPrivate = true;
		} else 
		    frames.push_back (it-> frame ());
	    
	    if (hasPrivate) {
		Ymir::Error::privateMemberWithinThisContext (this-> typeString (), var-> token);
		return NULL;
	    } else {	    
		auto ret = new (Z0) IFunctionInfo (this-> _id-> _space, "init", frames);
		ret-> isConstr () = true;
		return ret;
	    }
	}
    }    
    
    Ymir::Tree IAggregateInfo::buildVtableType () {
	int size = 1; // The last element is a pointer to the typeinfo
	for (auto it : this-> _methods) {
	    if (it-> isVirtual ())
		size ++;
	}

	auto innerType = new (Z0) IPtrInfo (true, new (Z0) IVoidInfo ());
	auto intExpr = new (Z0)  IFixed (Word::eof (), FixedConst::ULONG);
	intExpr-> setUValue (size);
	auto lenExpr = intExpr-> toGeneric ();			
	intExpr-> setUValue (0);
	auto begin = intExpr-> toGeneric ();
	intExpr-> setUValue (size);
	
	auto range_type = build_range_type (integer_type_node, fold (begin.getTree ()), fold (lenExpr.getTree ()));
	auto array_type = build_array_type (innerType-> toGeneric ().getTree (), range_type);
	return array_type;
    }
    
    
    std::vector <FunctionInfo> IAggregateInfo::getAllMethods () {
	return this-> _allMethods;
    }

    std::vector <InfoType> & IAggregateInfo::getTypes () {
	return this-> _types;
    }

    std::vector <std::string> & IAggregateInfo::getAttrs () {
	return this-> _attrs;
    }

    std::vector <syntax::InnerProtection> & IAggregateInfo::getInnerProts () {
	return this-> _prots;
    }

    std::vector <Namespace> & IAggregateInfo::getAttrSpaces () {
	return this-> _attrSpaces;
    }
    
    
    std::vector <FunctionInfo> IAggregateInfo::getMethods () {
	if (!this-> getAncestor ()) return this-> _methods;
	auto method = this-> getAncestor ()-> _allMethods;
	std::vector <FunctionInfo> toAdd;
	for (auto mt : this-> _methods) {
	    if (mt-> frame ()-> func ()-> getTemplates ().size () != 0 && mt-> isOver ()) {
		Ymir::Error::overTemplateMethod (mt-> frame ()-> func ()-> getIdent ());
		continue;
	    }
	    
	    bool changed = false;
	    auto from = Table::instance ().globalNamespace ();
	    for (auto & it : method) {
		auto name = it-> name ();
		if (mt-> name () == it-> name ()) {
		    auto lparams = mt-> frame ()-> func ()-> getParams ();
		    auto rparams = it-> frame ()-> func ()-> getParams ();
		    if (lparams.size ()  != rparams.size ()) continue;
		    bool valid = true;
		    
		    Table::instance ().setCurrentSpace (Namespace (it-> frame ()-> space (), it-> name ()));
		    for (auto it : Ymir::r (1, lparams.size ())) {
			if (lparams [it]-> is <ITypedVar> () && rparams [it]-> is <ITypedVar> ()) {
			    auto ltype = lparams [it]-> to <ITypedVar> ()-> getType ();
			    auto rtype = rparams [it]-> to <ITypedVar> ()-> getType ();
			    if (ltype == NULL || rtype == NULL || !ltype-> CompOp (rtype)) {
				valid = false;
				break;
			    }
			} else {
			    valid = false;
			    break;
			}
		    }		    
		    Table::instance ().setCurrentSpace (from);
		    
		    if (!valid) continue;
		    if (!mt-> isOver ()) {
			Ymir::Error::implicitOverride (
			    mt-> frame ()-> func ()-> getIdent (),
			    it-> frame ()-> func ()-> getIdent ()
			);
		    }

		    if (it-> frame ()-> func ()-> getTemplates ().size () != 0) {
			Ymir::Error::overTemplateMethod (mt-> frame ()-> func ()-> getIdent (), it-> frame ()-> func ()-> getIdent ());
			break;
		    }
		    
		    if (it-> frame ()-> isInnerPrivate ()) {
			Ymir::Error::overPrivateMethod (it-> frame ()-> func ()-> getIdent (), mt-> frame ()-> func ()-> getIdent ());
			break;
		    } else if (mt-> frame ()-> isInnerProtected ()) {
			Ymir::Error::overPrivateMethod (mt-> frame ()-> func ()-> getIdent ());
			break;
		    }
		    		    
		    changed = true;
		    it = mt;
		    break;
		}
	    }
	    	    
	    if (!changed) {
		if (mt-> isOver ()) {
		    Ymir::Error::noOverride (mt-> frame ()-> func ()-> getIdent ());
		}
		
		toAdd.push_back (mt);
	    }
	}
	method.insert (method.end (), toAdd.begin (), toAdd.end ());
	return method;   
    }
    
    Ymir::Tree IAggregateInfo::buildVtableEnum (Ymir::Tree vtype) {
	vec<constructor_elt, va_gc> * elms = NULL;
	int i = 1;

	CONSTRUCTOR_APPEND_ELT (elms, size_int (0), Ymir::getAddr (this-> genericTypeInfo ()).getTree ());	
	auto methods = this-> _allMethods;
	for (auto it : methods) {
	    if (it-> isVirtual ()) {
		CONSTRUCTOR_APPEND_ELT (elms, size_int (i), it-> frame ()-> validate ()-> toGeneric ().getTree ());
		i ++;
	    }
	}

	return build_constructor (vtype.getTree (), elms);
    }
    
    Ymir::Tree IAggregateInfo::getVtable () {
	auto tname = this-> simpleTypeString ();
	auto vname = Ymir::OutBuffer ("_YTV", Mangler::mangle_namespace (tname)).str ();
	auto vtable = Ymir::getVtable (vname);

	if (vtable.isNull ()) {
	    auto vtype = buildVtableType ();
	    if (!this-> _isExternal) {
		// The vtable is declared, recursive access will succeed
		vtable = declareVtable (vname, vtype); 
		auto vec = buildVtableEnum (vtype); // This construction can create recursive access
		DECL_INITIAL (vtable.getTree ()) = vec.getTree ();
	    } else vtable = declareVtableExtern (vname, vtype);
	} else if (!this-> _isExternal && DECL_EXTERNAL (vtable.getTree ()) == 1) {
	    DECL_EXTERNAL (vtable.getTree ()) = 0; // Prevent recursive infinite loop
	    auto vtype = vtable.getType ();
	    auto vec = buildVtableEnum (vtype); // Will recursivly call getVtable 
	    DECL_INITIAL (vtable.getTree ()) = vec.getTree ();
	}
	return vtable;
    }
    
    Ymir::Tree IAggregateInfo::toGeneric () {
	auto tname = this-> simpleTypeString ();
	auto vname = Ymir::OutBuffer ("_YTV", Mangler::mangle_namespace (tname)).str ();
	auto ttype = IFinalFrame::getDeclaredType (tname.c_str ());
	if (ttype.isNull ()) {
	    std::vector <InfoType> types = {new (Z0) IPtrInfo (true, new (Z0) IVoidInfo ())};
	    std::vector <std::string> attrs = {Keys::VTABLE_FIELD};

	    for (auto it : Ymir::r (0, this-> _types.size ())) {
		types.push_back (this-> _types [it]);
		attrs.push_back (this-> _attrs [it]);
	    }
			    
	    ttype = Ymir::makeTuple (tname, types, attrs); 
	    IFinalFrame::declareType (tname, ttype);

	    // Declare the vtable
	    this-> getVtable ();
	}
	
	return ttype;
    }

    Ymir::Tree IAggregateInfo::genericTypeInfo () {
	auto innerGlob = this-> getVtable ();		
	auto type = Table::instance ().getTypeInfoType ()-> TempOp ({});
	auto typeTree = type-> toGeneric ();
	
	vec <constructor_elt, va_gc> * elms = NULL;
	auto struct_info_type = Table::instance ().getTypeInfoType (Ymir::Runtime::AGGREGATE_INFO)-> TempOp ({})-> to <IAggregateInfo> ();
	auto vtable = struct_info_type-> getVtable ();
	    
	CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Keys::VTABLE_FIELD).getTree (), Ymir::getAddr (vtable).getTree ());	
	CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Ymir::Runtime::VTABLE_FIELD_TYPEINFO).getTree (), Ymir::getAddr (innerGlob).getTree ());
	CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Ymir::Runtime::LEN_FIELD_TYPEINFO).getTree (), build_int_cst_type (long_unsigned_type_node, 0));
	if (this-> _anc) {
	    auto ancTree = this-> _anc-> genericTypeInfo ();
	    CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Ymir::Runtime::C_O_A_TYPEINFO).getTree (), Ymir::getAddr (ancTree).getTree ());
	} else {
	    CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Ymir::Runtime::C_O_A_TYPEINFO).getTree (), build_int_cst_type (long_unsigned_type_node, 0));
	}

	auto name = Ymir::Runtime::TYPE_INFO_MODULE + "." + this-> simpleTypeString () + Ymir::Runtime::TYPE_INFO_SUFFIX;
	auto glob = Ymir::declareGlobalWeak (name, typeTree, build_constructor (typeTree.getTree (), elms));

	return glob;
    }	

    Ymir::Tree IAggregateInfo::genericConstructor () {
	vec<constructor_elt, va_gc> * elms = NULL;
	auto vtable = this-> getVtable ();
	auto vtype = this-> toGeneric ();
	auto fields = Ymir::getFieldDecls (vtype);
	CONSTRUCTOR_APPEND_ELT (elms, fields [0].getTree (), Ymir::getAddr (vtable).getTree ());

	for (auto it : Ymir::r (0, this-> _attrs.size ())) {
	    CONSTRUCTOR_APPEND_ELT (elms, fields [it + 1].getTree (), this-> _types [it]-> genericConstructor ().getTree ());
	}
	
	return build_constructor (vtype.getTree (), elms);
    }
    
    void IAggregateInfo::setTmps (const std::vector <Expression> & tmps) {
	this-> tmpsDone = tmps;
    }
    
    InfoType IAggregateInfo::getTemplate (ulong nb) {
	ulong current = 0;
	for (auto it : Ymir::r (0, this-> _id-> _tmpsDone.size ())) {
	    if (auto par = this-> _id-> _tmpsDone [it]-> to <IParamList> ()) {
		if (current <= nb && nb < current + par-> getParams ().size ()) {
		    auto index = nb - current;
		    return par-> getParamTypes () [index]-> clone ();
		} else {
		    current += par-> getParams ().size ();
		}		
	    } else {
		if (current == nb)
		    return this-> _id-> _tmpsDone [current]-> info-> type ();
		current ++;
	    }	    
	}
	return NULL;
    }

    AggregateInfo IAggregateInfo::getAncestor () {
	return this-> _anc;
    }
    
    std::vector <InfoType> IAggregateInfo::getTemplate (ulong bef, ulong af) {
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

    const char * IAggregateInfo::getId () {
	return IAggregateInfo::id ();
    }
    
    bool IAggregateInfo::inPrivateContext () {
	auto space = Table::instance ().getCurrentSpace ();
	auto myspace = Namespace (this-> innerTypeString ());
	return myspace.isSubOf (space);
    }

    bool IAggregateInfo::inProtectedContext () {
	auto space = Table::instance ().getCurrentSpace ();
	auto myspace = Namespace (this-> innerTypeString ());
	if (myspace.isSubOf (space)) return true;
	else if (this-> _anc) {
	    auto ret = this-> _anc-> inProtectedContext ();
	    if (ret) return true;
	}
	return false;
    }

    bool IAggregateInfo::isMine (Namespace space) {
	return Namespace (this-> innerTypeString ()) == space;
    }

    bool IAggregateInfo::isProtectedForMe (Namespace space) {
	if (isMine (space)) return true;
	if (this-> _anc) return this-> _anc-> isProtectedForMe (space);
	return false;
    }
    
    bool IAggregateInfo::hasCopyCstr () {
	for (auto it : this-> _id-> _contrs) {	    
	    if (it-> frame ()-> is <IConstructFrame> () && it-> frame ()-> to<IConstructFrame> ()-> isCopy ()) {
		return true;
	    }
	}
	return false;
    }

    Frame IAggregateInfo::cpyCstr () {
	for (auto it : this-> _id-> _contrs) {
	    if (it-> frame ()-> is <IConstructFrame> () && it-> frame ()-> to<IConstructFrame> ()-> isCopy ()) {
		return it-> frame ();
	    }
	}
	return NULL;
    }

}
