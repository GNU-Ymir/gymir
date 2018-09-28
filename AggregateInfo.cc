#include <ymir/semantic/object/AggregateInfo.hh>
#include <ymir/ast/Function.hh>
#include <ymir/ast/TypeCreator.hh>
#include <ymir/ast/Var.hh>
#include <ymir/semantic/object/MethodInfo.hh>
#include <ymir/ast/ParamList.hh>
#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/ast/TreeExpression.hh>
#include <ymir/utils/Mangler.hh>
#include <ymir/semantic/types/RefInfo.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/utils/StructUtils.hh>
#include <ymir/semantic/pack/MethodFrame.hh>
#include <ymir/ast/TypedVar.hh>
#include <ymir/ast/Binary.hh>


using namespace syntax;   

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
	
	Ymir::Tree InstUnref (Word locus, InfoType t, Expression left, Expression right) {
	    auto type = t-> cloneOnExitWithInfo ();
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);

	    auto loc = locus.getLocus ();
	    auto innerType = left-> info-> type ()-> to <IAggregateInfo> ()-> getImpl ();
	    
	    Ymir::TreeStmtList list;
	    auto leftExp = Ymir::getExpr (list, left);
	    leftExp = getField (locus.getLocus (), leftExp, 1);
	    
	    return Ymir::compoundExpr (loc, list, type-> buildBinaryOp (
		locus,
		type,
		new (Z0) ITreeExpression (left-> token, innerType, leftExp),
		right
	    ));
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
    
    IAggregateCstInfo::IAggregateCstInfo (Word locId, Namespace space, std::string name, const std::vector <syntax::Expression> & tmps, const std::vector <syntax::Expression> & self, bool isUnion, bool isOver) :
	IInfoType (true),
	_space (space),
	_locId (locId),
	_name (name),
	_contrs ({}), _destr (NULL), _methods ({}), _staticMeth ({}),
	_tmps (tmps),
	_impl (self),
	_isUnion (isUnion),
	_isOver (isOver)
	
    {}

    std::vector <FunctionInfo> & IAggregateCstInfo::getConstructors () {
	return this-> _contrs;
    }

    std::vector <TypeAlias> & IAggregateCstInfo::getAlias () {
	return this-> _alias;
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

    Namespace IAggregateCstInfo::space () {
	return this-> _space;
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
	static std::map <std::string, AggregateInfo> validated;

	if (this-> _tmps.size () != 0) {
	    return NULL;
	    //return this-> getScore (tmps);
	}

	if (this-> _info) return this-> _info-> clone ();	
	if (this-> _tmps.size () != tmps.size ()) return NULL;

	auto name = Namespace (this-> _space, this-> _name).toString ();
	auto valid = validated.find (name);
	if (valid != validated.end ()) return valid-> second-> clone ();
	auto inside = inProgress.find (name);
	if (inside != inProgress.end ()) return inside-> second;
	
	auto info = new (Z0) IAggregateInfo (this, this-> _space, this-> _name, {}, this-> _isExternal);
	inProgress [name] = info;
	
	auto str = this-> constructImpl ();
	if (str == NULL) {
	    inProgress [name] = NULL;
	    return NULL;
	}
	
	if (recursiveGet (info, str)) {
	    inProgress [name] = NULL;
	    Ymir::Error::recursiveNoSize (this-> _impl [0]-> token);
	    return NULL;
	}
	    
	info-> _impl = str;
	if (this-> _destr)
	    info-> _destr = this-> _destr-> frame ();
	info-> _staticMeth = this-> _staticMeth;
	info-> _methods = this-> _methods;
	InfoType anc = NULL;
	if (this-> _anc) anc = this-> _anc-> TempOp ({});
	if (anc) info-> _anc = anc-> to <IAggregateInfo> ();
	
	info-> _allMethods = info-> getMethods ();
	info-> _allAlias = info-> getAllAlias ();
	this-> _info = info;	    
	
	// info-> setTmps (this-> tmpsDone); TODO
	inProgress.erase (name);	
	return info-> clone ();	    
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
	for (auto it : this-> _staticMeth) {
	    if (it-> name () == var-> token.getStr ()) {
		bool hasPrivate = false;
		if (it-> frame ()-> isInnerPrivate () && (!this-> isMine (it-> frame ()-> space ()) || (!this-> inPrivateContext ()))) {
		    hasPrivate = true;
		} else if (it-> frame ()-> isInnerProtected () && (!this-> isProtectedForMe (it-> frame ()-> space ()) || (!this-> inProtectedContext ()))) {
		    hasPrivate = true;
		}

		if (!hasPrivate) {
		    auto ret = it-> clone ();
		    ret-> unopFoo = &AggregateUtils::InstGetStaticMeth;
		    return ret;
		} else {
		    Ymir::Error::privateMemberWithinThisContext (this-> typeString (), var-> token);
		    return NULL;
		}
	    }	    		
	}
	return NULL;
    }
    
    std::string IAggregateCstInfo::typeString () {
	return Namespace (this-> _space, this-> _name).toString ();
    }

    std::string IAggregateCstInfo::innerTypeString () {
	return Namespace (this-> _space, this-> _name).toString ();
    }    
    
    std::string IAggregateCstInfo::innerSimpleTypeString () {
	return Namespace (this-> _space, this-> _name).toString ();
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

    TupleInfo IAggregateCstInfo::constructImpl () {
	auto last = Table::instance ().templateNamespace ();
	auto currentSpace = Table::instance ().space ();
	
	Table::instance ().setCurrentSpace (this-> _space);
	Table::instance ().templateNamespace () = currentSpace;
	
	auto type = this-> _impl [0]-> toType ();
	if (type == NULL) {
	    Table::instance ().setCurrentSpace (currentSpace);
	    Table::instance ().templateNamespace () = last;
	    return NULL;
	}

	if (this-> _isOver) {
	    if (this-> _anc == NULL) {
		auto agg = type-> info-> type ()-> to <IAggregateInfo> ();
		if (agg == NULL) {
		    Ymir::Error::cannotOverride (this-> _locId, type-> info-> type ());
		    Table::instance ().setCurrentSpace (currentSpace);
		    Table::instance ().templateNamespace () = last;
		    return NULL;
		}
		this-> _anc = agg-> _id;
	    }
	    Table::instance ().setCurrentSpace (currentSpace);
	    Table::instance ().templateNamespace () = last;
	    return this-> _anc-> constructImpl ();
	} else {	
	    auto str = type-> info-> type ()-> to <ITupleInfo> ();
	    if (str == NULL) {
		str = new (Z0) ITupleInfo (false, false, this-> _isUnion);
		str-> addParam (type-> info-> type ());
	    }
	    Table::instance ().setCurrentSpace (currentSpace);
	    Table::instance ().templateNamespace () = last;
	    return str;		
	}	
    }    

    bool IAggregateCstInfo::inProtectedContext () {
	auto space = Table::instance ().getCurrentSpace ();
	if (space.innerMods () [space.innerMods ().size () - 2] == this-> _name) {
	    return true;
	} else if (this-> _anc) {
	    auto ret = this-> _anc-> inPrivateContext ();
	    if (ret) return true;
	}
	
	std::vector <std::string> name (space.innerMods ().begin (), space.innerMods ().end () - 1);
	return FrameTable::instance ().isSuccessor (Namespace (name), this);		   	
    }
    
    bool IAggregateCstInfo::inPrivateContext () {
	auto space = Table::instance ().getCurrentSpace ();
	if (space.innerMods () [space.innerMods ().size () - 2] == this-> _name) {
	    return true;
	}
	return false;
    }


    bool IAggregateCstInfo::isMine (Namespace space) {
	return Namespace (this-> _space, this-> _name) == space;
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

    Frame IAggregateInfo::getDestructor () {
	if (this-> _destr || !this-> _anc)
	    return this-> _destr;
	else return this-> _anc-> getDestructor ();
    }   
    
    InfoType IAggregateInfo::ConstVerif (InfoType type) {
	if (auto aggr = type-> to <IAggregateInfo> ())
	    if (this-> _impl-> ConstVerif (aggr-> _impl))
		return this;
	return NULL;
    }

    InfoType IAggregateInfo::onClone () {
	auto ret = new (Z0) IAggregateInfo (this-> _id, this-> _space, this-> _name, this-> tmpsDone, this-> _isExternal);
	ret-> _impl = this-> _impl-> clone ()-> to <ITupleInfo> ();
	ret-> _destr = this-> _destr;
	ret-> _staticMeth = this-> _staticMeth;
	ret-> _methods = this-> _methods;
	ret-> _allMethods = this-> _allMethods;
	ret-> _allAlias = this-> _allAlias;
	if (this-> _anc)
	    ret-> _anc = this-> _anc;
	ret-> isConst (this-> isConst ());
	
	return ret;
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
	if (!var-> hasTemplate () && var-> token.getStr () == Keys::SUPER && this-> getAncestor () != NULL)  {
	    if (this-> inPrivateContext ())
		return Super ();
	    else { Ymir::Error::privateMemberWithinThisContext (this-> typeString (), var-> token); return NULL; }
	} if (var-> token.getStr () == Keys::DISPOSE) {
	    auto ret = new (Z0) IMethodInfo (this, var-> token.getStr (), {this-> getDestructor ()}, {0}, true);
	    ret-> binopFoo = &AggregateUtils::InstGetMethod;
	    return ret;
	}
	
	this-> _impl-> isConst (this-> isConst ());
	auto ret = this-> _impl-> DotOpAggr (this-> _id-> getLocId (), this, var);
	bool alias = false;
	if (ret == NULL && !this-> _hasExemption) {
	    ret = this-> AliasOp (var);
	    if (ret) {
		ret-> nextBinop.push_back (ret-> binopFoo);
		ret-> binopFoo = &AggregateUtils::InstUnref;
		return ret;
	    }
	}
	
	if (ret == NULL || (!this-> inProtectedContext () && !alias)) {	    
	    auto fin = this-> Method (var);
	    if (fin) return fin;
	    else if (ret) { Ymir::Error::privateMemberWithinThisContext (this-> typeString (), var-> token); return NULL; }
	    return NULL;
	} else {	    
	    ret-> isConst (this-> isConst ());
	    ret-> nextBinop.push_back (ret-> binopFoo);
	    ret-> binopFoo = &AggregateUtils::InstUnref;
	    return ret;
	}
    }

    InfoType IAggregateInfo::DotExpOp (Expression right) {
	this-> _impl-> isConst (this-> isConst ());
	auto ret = this-> _impl-> DotExpOp (right);
	if (ret != NULL) {
	    if (!this-> inPrivateContext ()) {
		Ymir::Error::privateMemberWithinThisContext (this-> typeString (), right-> token);
		return NULL;
	    } else {
		ret-> isConst (this-> isConst ());
		ret-> nextBinop.push_back (ret-> binopFoo);
		ret-> binopFoo = &AggregateUtils::InstUnref;
	    }
	}
	return ret;
    }
    
    InfoType IAggregateInfo::DColonOp (Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "init") return Init (var);
	if (var-> token == "sizeof") return SizeOf ();
	//if (var-> token == "name") return Name ();
	for (auto it : this-> _staticMeth) {
	    if (it-> name () == var-> token.getStr ()) {
		auto ret = it-> clone ();
		ret-> unopFoo = &AggregateUtils::InstGetStaticMeth;
		return ret;
	    }	    		
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

    std::string IAggregateInfo::getName () {
	return this-> _name;
    }

    Namespace & IAggregateInfo::getSpace () {
	return this-> _space;
    }

    std::string IAggregateInfo::innerTypeString () {
	return Namespace (this-> _space, this-> _name).toString ();
    }

    std::string IAggregateInfo::innerSimpleTypeString () {
	return Namespace (this-> _space, this-> _name + "A").toString ();
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
    
    InfoType IAggregateInfo::AliasOp (Var var) {
	bool hasPrivate = false;
	for (auto it : this-> _allAlias) {	    
	    if (it-> getIdent ().getStr () == var-> token.getStr ()) {
		if (it-> isPrivate () && (!this-> isMine (it-> space ()) || !this-> inPrivateContext ()))
		    hasPrivate = true;
		else if (it-> isProtected () && (!this-> isProtectedForMe (it-> space ()) || !this-> inProtectedContext ()))
		    hasPrivate = true;
		else {
		    auto ret = new (Z0) IAliasCstInfo (var-> token, this-> _space, it-> getValue ());

		    ret-> isConst (this-> isConst ());
		    if (it-> isConst ()) ret-> isConst (it-> isConst ());
		    return ret;
		}
	    }
	}
	if (hasPrivate)
	    Ymir::Error::privateMemberWithinThisContext (this-> typeString (), var-> token);
	return NULL;
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

    TupleInfo IAggregateInfo::getImpl () {
	return this-> _impl;
    }
    
    Ymir::Tree IAggregateInfo::buildVtableType () {
	int size = 0;
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

    std::vector <TypeAlias> IAggregateInfo::getAllAlias () {
	std::vector <TypeAlias> alias;
	if (this-> getAncestor ()) 
	    alias = this-> getAncestor ()-> _allAlias;
	auto clone = this-> clone ()-> to <IAggregateInfo> ();
	clone-> _hasExemption = true;
	
	for (auto it : this-> _id-> getAlias ()) {
	    auto name = it-> getIdent ().getStr ();
	    bool error = false;
	    for (auto anc : alias) {
		if (name == anc-> getIdent ().getStr ()) {
		    Ymir::Error::shadowingVar (it-> getIdent (), anc-> getIdent ());
		    error = true;
		    break;
		}
	    }
	    
	    if (!error) {
		auto ign = new (Z0) IExpression (it-> getIdent ());
		ign-> info = new (Z0) ISymbol (it-> getIdent (), ign, clone);
		auto expr = new (Z0) IEvaluatedExpr (ign);
		auto eval = new (Z0) IAliasCstInfo (it-> getIdent (), this-> _space, it-> getValue ());
		if (eval-> replace ({{Keys::SELF, expr}})-> expression () != NULL)
		    alias.push_back (it);
	    }
	}
	return alias;
    }
    
    std::vector <FunctionInfo> IAggregateInfo::getMethods () {
	if (!this-> getAncestor ()) return this-> _methods;
	auto method = this-> getAncestor ()-> _allMethods;
	std::vector <FunctionInfo> toAdd;
	for (auto mt : this-> _methods) {
	    bool changed = false;
	    for (auto & it : method) {
		auto name = it-> name ();
		if (mt-> name () == it-> name ()) {
		    auto lparams = mt-> frame ()-> to <IMethodFrame> ()-> getMethod ()-> getParams ();
		    auto rparams = it-> frame ()-> to <IMethodFrame> ()-> getMethod ()-> getParams ();
		    if (lparams.size ()  != rparams.size ()) continue;
		    for (auto it : Ymir::r (0, lparams.size ())) {
			if (lparams [it]-> is <ITypedVar> () && rparams [it]-> is <ITypedVar> ()) {
			    if (!lparams [it]-> to <ITypedVar> ()-> getType ()-> CompOp (rparams [it]-> to <ITypedVar> ()-> getType ()))
				continue;
			} else continue;
		    }

		    if (!mt-> isOver ()) {
			Ymir::Error::implicitOverride (
			    mt-> frame ()-> func ()-> getIdent (),
			    it-> frame ()-> func ()-> getIdent ()
			);
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
	int i = 0;
	
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
		auto vec = buildVtableEnum (vtype);
		vtable = declareVtable (vname, vtype, vec);
	    } else vtable = declareVtableExtern (vname, vtype);
	}
	return vtable;
    }
    
    Ymir::Tree IAggregateInfo::toGeneric () {
	auto tname = this-> simpleTypeString ();
	auto vname = Ymir::OutBuffer ("_YTV", Mangler::mangle_namespace (tname)).str ();
	auto ttype = IFinalFrame::getDeclaredType (tname.c_str ());
	if (ttype.isNull ()) {
	    std::vector <InfoType> types = {new (Z0) IPtrInfo (true, new (Z0) IVoidInfo ()), this-> _impl};
	    auto attrs = {Keys::VTABLE_FIELD};

	    ttype = Ymir::makeTuple (tname, types, attrs); 
	    IFinalFrame::declareType (tname, ttype);
	    
	    if (Ymir::getVtable (vname).isNull ()) {
		auto vtype = buildVtableType ();
		if (!this-> _isExternal) {
		    auto vec = buildVtableEnum (vtype);
		    declareVtable (vname, vtype, vec);
		} else declareVtableExtern (vname, vtype);
	    }
	}
	
	return ttype;
	return this-> _impl-> toGeneric ();
    }

    void IAggregateInfo::setTmps (const std::vector <Expression> & tmps) {
	this-> tmpsDone = tmps;
    }
    
    InfoType IAggregateInfo::getTemplate (ulong) {
	return NULL;
    }

    AggregateInfo IAggregateInfo::getAncestor () {
	return this-> _anc;
    }

    bool& IAggregateInfo::hasExemption () {
	return this-> _hasExemption;
    }
    
    std::vector <InfoType> IAggregateInfo::getTemplate (ulong, ulong) {
	return {};
    }

    const char * IAggregateInfo::getId () {
	return IAggregateInfo::id ();
    }
    
    bool IAggregateInfo::inPrivateContext () {
	if (this-> _hasExemption) return true;
	auto space = Table::instance ().getCurrentSpace ();
	if (space.innerMods () [space.innerMods ().size () - 2] == this-> _name) {
	    return true;
	}
	return false;
    }

    bool IAggregateInfo::inProtectedContext () {
	auto space = Table::instance ().getCurrentSpace ();
	if (space.innerMods () [space.innerMods ().size () - 2] == this-> _name) {
	    return true;
	} else if (this-> _anc) {
	    auto ret = this-> _anc-> inPrivateContext ();
	    if (ret) return true;
	}
	
	std::vector <std::string> name (space.innerMods ().begin (), space.innerMods ().end () - 1);
	return FrameTable::instance ().isSuccessor (Namespace (name), this-> _id);		   	
    }

    bool IAggregateInfo::isMine (Namespace space) {
	return Namespace (this-> _space, this-> _name) == space;
    }

    bool IAggregateInfo::isProtectedForMe (Namespace space) {
	if (isMine (space)) return true;
	if (this-> _anc) return this-> _anc-> isProtectedForMe (space);
	return false;
    }
    
    bool IAggregateInfo::hasCopyCstr () {
	for (auto it : this-> _id-> _contrs) {	    
	    if (it-> frame ()-> to<IMethodFrame> ()-> isCopy ()) {
		return true;
	    }
	}
	return false;
    }

    Frame IAggregateInfo::cpyCstr () {
	for (auto it : this-> _id-> _contrs) {
	    if (it-> frame ()-> to<IMethodFrame> ()-> isCopy ()) {
		return it-> frame ();
	    }
	}
	return NULL;
    }

}
