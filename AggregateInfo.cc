#include <ymir/semantic/object/AggregateInfo.hh>
#include <ymir/semantic/types/FunctionInfo.hh>
#include <ymir/ast/Var.hh>
#include <ymir/semantic/utils/StructUtils.hh>
#include <ymir/semantic/types/UndefInfo.hh>
#include <ymir/semantic/types/FixedInfo.hh>
#include <ymir/semantic/object/MethodInfo.hh>
#include <ymir/ast/ParamList.hh>
#include <ymir/semantic/types/PtrInfo.hh>
#include <ymir/semantic/types/VoidInfo.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/ast/TreeExpression.hh>
#include <ymir/utils/Mangler.hh>
#include <ymir/semantic/types/RefInfo.hh>

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

	Tree InstGetMethod (Word, InfoType, Expression left, Expression) {
	    return left-> toGeneric ();
	}

	Ymir::Tree InstUnref (Word locus, InfoType t, Expression left, Expression right) {
	    auto type = t-> cloneOnExitWithInfo ();
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);

	    auto loc = locus.getLocus ();
	    auto innerType = left-> info-> type-> to <IAggregateInfo> ()-> getImpl ();
	    
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

	
    }
    
    IAggregateCstInfo::IAggregateCstInfo (Word locId, Namespace space, std::string name, const std::vector <syntax::Expression> & tmps, const std::vector <syntax::Expression> & self, bool isUnion) :
	IInfoType (true),
	_space (space),
	_locId (locId),
	_name (name),
	_contrs ({}), _destr (NULL), _methods ({}), _staticMeth ({}),
	_tmps (tmps),
	_impl (self),
	_isUnion (isUnion)
	
    {}

    std::vector <FunctionInfo> & IAggregateCstInfo::getConstructors () {
	return this-> _contrs;
    }

    FunctionInfo & IAggregateCstInfo::getDestructor () {
	return this-> _destr;
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
        
    InfoType IAggregateCstInfo::TempOp (const std::vector <Expression> &) {
	if (this-> _impl.size () != 0) {
	    auto str = this-> constructImpl ();
	    if (str) {
		auto ret = new (Z0) IAggregateInfo (this, this-> _space, this-> _name, {}, this-> _isExternal);
		ret-> _impl = str;
		if (this-> _destr)
		    ret-> _destr = this-> _destr-> frame ();
		ret-> _staticMeth = this-> _staticMeth;
		ret-> _methods = this-> _methods;
		return ret;	    
	    } else {
		// if (!this-> _isFailure) {
		//     this-> _isFailure = true;	    
		//     Ymir::Error::cannotImpl (expr-> token, expr-> info-> type);
		// }
		return NULL;
	    }
	} else Ymir::Error::assert ("");
	return NULL;
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
	if (var-> token == "init") return Init ();
	if (var-> token == "sizeof") return SizeOf ();
	for (auto it : this-> _staticMeth) {
	    if (it-> name () == var-> token.getStr ()) {
		auto ret = it-> clone ();
		ret-> unopFoo = &AggregateUtils::InstGetStaticMeth;
		return ret;
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
    
    const char* IAggregateCstInfo::getId () {
	return IAggregateCstInfo::id ();
    }

    Word IAggregateCstInfo::getLocId () {
	return this-> _locId;	
    }

    InfoType IAggregateCstInfo::Init () {
	if (this-> _contrs.size () == 0) {
	    return NULL;
	} else {
	    std::vector <Frame> frames;
	    for (auto it : this-> _contrs)
		frames.push_back (it-> frame ());
	    auto ret = new (Z0) IFunctionInfo (this-> _space, "init", frames);
	    ret-> isConstr () = true;
	    return ret;
	}
    }

    TupleInfo IAggregateCstInfo::constructImpl () {
	auto type = this-> _impl [0]-> toType ();
	if (type == NULL) return NULL;

	auto str = type-> info-> type-> to <ITupleInfo> ();
	if (str == NULL) {
	    str = new (Z0) ITupleInfo (false, false, this-> _isUnion);
	    str-> addParam (type-> info-> type);
	}
	
	return str;		
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
				if (!ps-> getParams ()[it_]-> info-> type-> isSame (ps2-> getParams () [it_]-> info-> type)) {
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
			if (!this-> tmpsDone [it]-> info-> type-> isSame (ot-> tmpsDone [it]-> info-> type)) {
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
	return this-> _destr;
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
	ret-> isConst (this-> isConst ());
	
	return ret;
    }

    InfoType IAggregateInfo::BinaryOp (Word op, Expression right) {
	if (op == Token::EQUAL && right-> info-> type-> isSame (this)) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &StructUtils::InstAffect;
	    return ret;
	}
	return NULL;
    }

    InfoType IAggregateInfo::BinaryOpRight (Word op, Expression left) {
	if (op == Token::EQUAL && left-> info-> type-> is <IUndefInfo> ()) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &StructUtils::InstAffect;
	    return ret;
	}
	return NULL;
    }
    
    InfoType IAggregateInfo::DotOp (Var var) {
	auto ret = this-> _impl-> DotOpAggr (this-> _id-> getLocId (), this, var);
	if (ret == NULL) {
	    return this-> Method (var);
	} else {
	    ret-> isConst (this-> isConst ());
	    ret-> nextBinop.push_back (ret-> binopFoo);
	    ret-> binopFoo = &AggregateUtils::InstUnref;
	    return ret;
	}
    }

    InfoType IAggregateInfo::DotExpOp (Expression right) {
	auto ret = this-> _impl-> DotExpOp (right);
	if (ret != NULL) {
	    ret-> isConst (this-> isConst ());
	    ret-> nextBinop.push_back (ret-> binopFoo);
	    ret-> binopFoo = &AggregateUtils::InstUnref;
	}
	return ret;
    }
    
    InfoType IAggregateInfo::DColonOp (Var var) {
	if (var-> hasTemplate ()) return NULL;
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
	if (this-> isSame (other)) {
	    auto ret = this-> clone ();
	    ret-> isConst (this-> isConst ());
	    ret-> binopFoo = &StructUtils::InstCast;
	    return ret;
	} else if (other-> is <IUndefInfo> ()) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &StructUtils::InstCast;
	    return ret;
	} else if (auto ref = other-> to <IRefInfo> ()) {
	    if (this-> isSame (ref-> content ())) {
		auto ret = new (Z0) IRefInfo (this-> isConst (), ref-> content ()-> clone ());
		ret-> content ()-> isConst (this-> isConst ());
		ret-> binopFoo = &StructUtils::InstAddr;
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
    
    InfoType IAggregateInfo::Method (Var var) {
	std::vector <Frame> frames;
	std::vector <int> index;
	int i = 0;
	for (auto it : this-> _methods) {
	    if (it-> name () == var-> token.getStr ()) {
		frames.push_back (it-> frame ());
		index.push_back (i);
	    }
	    i ++;
	}
	
	if (frames.size () != 0) {
	    auto meth = new (Z0) IMethodInfo (this, var-> token.getStr (), frames, index);
	    meth-> binopFoo = AggregateUtils::InstGetMethod;
	    return meth;
	} else return NULL;
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
    
    Ymir::Tree IAggregateInfo::buildVtableEnum (Ymir::Tree vtype) {
	vec<constructor_elt, va_gc> * elms = NULL;
	int i = 0;
	for (auto it : this-> _methods) {
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
    
    std::vector <InfoType> IAggregateInfo::getTemplate (ulong, ulong) {
	return {};
    }

    const char * IAggregateInfo::getId () {
	return IAggregateInfo::id ();
    }
    
    
}
