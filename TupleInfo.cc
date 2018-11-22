#include <ymir/semantic/types/_.hh>
#include <ymir/utils/Range.hh>
#include <ymir/ast/Expression.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/utils/TupleUtils.hh>
#include <ymir/ast/Var.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/ast/TreeExpression.hh>
#include <ymir/semantic/value/_.hh>
#include <ymir/semantic/object/AggregateInfo.hh>
using namespace syntax;

namespace semantic {

    ITupleInfo::ITupleInfo (bool isConst) :
	IInfoType (isConst),
	_isFake (false)
    {}

    ITupleInfo::ITupleInfo (bool isConst, bool isFake, bool isUnion) :
	IInfoType (isConst),
	_isFake (isFake),
	_isUnion (isUnion)       
    {}
    
    void ITupleInfo::setFake () {
	this-> _isFake = true;
    }
    
    bool ITupleInfo::isFake () {
	return this-> _isFake;
    }

    bool ITupleInfo::isUnion () {
	return this-> _isUnion;
    }
    
    InfoType ITupleInfo::asNoFake () {
	auto ret = this-> cloneOnExitWithInfo ();
	if (this-> binopFoo && this-> binopFoo == TupleUtils::InstCastFake) {
	    ret-> binopFoo = TupleUtils::InstCast;
	}
	return ret;
    }

    
    ulong ITupleInfo::nbParams () {
	return this-> params.size ();
    }

    InfoType ITupleInfo::ConstVerif (InfoType other) {
	if (auto tuple = other-> to <ITupleInfo> ()) {
	    for (auto it : Ymir::r (0, this-> params.size ())) {
		auto res = this-> params [it]-> ConstVerif (tuple-> params [it]);
		if (res == NULL) return NULL;
	    }
	    return this;
	}
	return NULL;
    }

    bool ITupleInfo::passingConst (InfoType other) {
	if (IInfoType::passingConst (other)) return true;
	else if (auto tuple = other-> to <ITupleInfo> ()) {
	    for (auto it : Ymir::r (0, this-> params.size ())) {
		if (this-> params [it]-> passingConst (tuple-> params [it]))
		    return true;		
	    }
	}
	return false;
    }
    
    bool ITupleInfo::isSame (InfoType other) {
	if (auto tu = other-> to <ITupleInfo> ()) {
	    if (this-> _isUnion != tu-> _isUnion) return false;
	    if (tu-> params.size () != this-> params.size ()) return false;
	    for (auto it : Ymir::r (0, this-> params.size ())) {
		if (!(tu-> params [it]-> isSame (this-> params [it])))
		    return false;
	    }

	    return true;
	}
	return false;
    }
    
    InfoType ITupleInfo::create (Word, const std::vector<syntax::Expression> & templates) {
	auto tuple = new (Z0)  ITupleInfo (false);
	for (auto it : Ymir::r (0, templates.size ())) {
	    if (auto ot = templates [it]-> info-> type ()-> to <IStructCstInfo> ()) {
		auto type = ot-> TempOp ({});
		if (type == NULL) return NULL;
		tuple-> params.push_back (type);
	    } else {
		tuple-> params.push_back (templates [it]-> info-> type ());
	    }
	}
	return tuple;
    }
    
    InfoType ITupleInfo::BinaryOp (Word op, syntax::Expression right) {
	if (op == Token::EQUAL) return Affect (op, right);
	return NULL;
    }

    InfoType ITupleInfo::BinaryOpRight (Word op, syntax::Expression left) {
	if (op == Token::EQUAL) return AffectRight (op, left);
	return NULL;
    }

    InfoType ITupleInfo::CompOp (InfoType other) {
	Word tok (UNKNOWN_LOCATION, Token::EQUAL);
	auto ot = other-> to <ITupleInfo> ();
	if (this-> isType ()) return NULL;
	if (!this-> _isFake && other-> isSame (this)) {	    
	    auto ret = new (Z0)  ITupleInfo (ot-> isConst ());
	    for (auto it : Ymir::r (0, ot-> params.size ())) {
		auto l = ot-> params [it];
		ret-> params.push_back (l-> clone ());
	    }

	    ret-> _isUnion = this-> _isUnion;
	    ret-> binopFoo = &TupleUtils::InstCast;
	    return ret;
	} else if (other-> is <IUndefInfo> ()) {
	    auto ret = new (Z0)  ITupleInfo (this-> isConst ());
	    for (auto it : Ymir::r (0, this-> params.size ())) {
		auto l = this-> params [it];		
		ret-> params.push_back (l-> clone ());
	    }
	    
	    ret-> _isUnion = this-> _isUnion;
	    ret-> binopFoo = &TupleUtils::InstCast;
	    return ret;
	} else if (auto ref = other->to <IRefInfo> ()) {
	    if (this-> isLvalue () && ref-> content ()-> isSame (this)) {
		auto aux = new (Z0)  IRefInfo (false, this-> clone ());
		aux-> binopFoo = &TupleUtils::InstAddr;
		return aux;
	    }
	} else if (this-> _isFake && ot && ot-> params.size () == this-> params.size ()) {
	    auto ret = new (Z0) ITupleInfo (ot-> isConst ());
	    for (auto it : Ymir::r (0, ot-> params.size ())) {
		auto l = this-> params [it]-> CompOp (ot-> params [it]);
		if (l == NULL) return NULL;
		ret-> params.push_back (l);
	    }

	    ret-> _isUnion = this-> _isUnion;
	    ret-> _isFake = true;
	    ret-> binopFoo = &TupleUtils::InstCastFake;
	    return ret;
	}
	return NULL;
    }
		
    InfoType ITupleInfo::UnaryOp (Word op) {
	if (op == Token::AND && this-> isLvalue ()) {
	    auto ret = new (Z0) IPtrInfo (this-> isConst (), this-> clone ());
	    ret-> binopFoo = &TupleUtils::InstAddr;
	    return ret;
	}
	return NULL;
    }
    
    InfoType ITupleInfo::onClone () {
	auto tu = new (Z0)  ITupleInfo (IInfoType::isConst ());
	for (auto it : this-> params) {
	    tu-> params.push_back (it-> clone ());
	}
	//tu-> value = this-> value;
	tu-> isType (this-> isType ());
	tu-> _isUnion = this-> _isUnion;
	
	return tu;
    } 

    InfoType ITupleInfo::DotExpOp (syntax::Expression right) {
	if (!right-> info-> isImmutable ()) return NULL;
	if (right-> info-> value ()-> is <IFixedValue> ()) {
	    auto value = right-> info-> value ()-> to<IFixedValue> ()-> getValue ();
	    if (value >= (int) this-> params.size ()) return NULL;
	    else {
		auto ret = this-> params [value]-> clone ();
		if (this-> isConst ())
		    ret-> isConst (true);
		ret = new (Z0) IArrayRefInfo (this-> isConst (), ret);
		ret-> binopFoo = &TupleUtils::InstGet;
		if (!this-> _isFake) 
		    ret-> value () = NULL;
		return ret;
	    }
	}
	return NULL;
    }

    InfoType ITupleInfo::DotOpAggr (const Word & locId, InfoType aggr, syntax::Var var) {
	InfoType ret = NULL;
	ulong i = 0, currenti = 0;
	for (auto it : this-> params) {
	    auto current = it-> DotOp (var);
	    if (auto tu = it-> to <ITupleInfo> ())
		current = tu-> DotOpAggr (locId, aggr, var);
	    if (current) {
		if (ret) {
		    Ymir::Error::ambiguousAccess (var-> token, locId, aggr);
		    return NULL;
		} 
		ret = current;
		i = currenti;
	    }
	    currenti ++;
	}
	
	if (ret) {
	    ret-> nextToGet.push_back (ret-> toGet ());
	    ret-> toGet () = i;
	    ret-> nextBinop.push_back (ret-> binopFoo);
	    ret-> binopFoo = &TupleUtils::InstUnref;
	}
	return ret;
    }
    

    InfoType ITupleInfo::DColonOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "typeid") return StringOf ();
	if (var-> token == "arity") return Arity ();
	if (var-> token == "init") return Init ();
	if (var-> token == "sizeof") return SizeOf ();
	return NULL;
    }

    InfoType ITupleInfo::Init () {
	auto ret = this-> clone ();
	ret-> unopFoo = TupleUtils::InstInit;
	return ret;
    }

    InfoType ITupleInfo::SizeOf () {
	auto ret = new (Z0) IFixedInfo (true, FixedConst::UINT);
	ret-> unopFoo = TupleUtils::InstSizeOf;
	return ret;
    }

    InfoType ITupleInfo::Arity () {
	auto ret = new (Z0) IFixedInfo (true, FixedConst::UINT);
	ret-> value () = new (Z0) IFixedValue (FixedConst::UINT, this-> params.size (), this-> params.size ());
	return ret;
    }
    
    std::string ITupleInfo::innerTypeString () {
	Ymir::OutBuffer buf;
	buf.write ("(");
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    buf.write (this-> params [it]-> typeString ());
	    if (this-> _isUnion) {
		if (it != (int) this-> params.size () - 1) buf.write (" | ");
	    } else
		if (it != (int) this-> params.size () - 1) buf.write (", ");
	}
	buf.write (")");
	return buf.str ();
    }

    std::string ITupleInfo::innerSimpleTypeString () {
	Ymir::OutBuffer buf;
	buf.write ("T", this-> params.size (), "T");
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    buf.write (Mangler::mangle_type (this-> params [it], this-> params [it]-> simpleTypeString ()));
	    if (this-> _isUnion) {
		if (it != (int) this-> params.size () - 1) buf.write ("|");
	    }
	}
	return buf.str ();
    }
    
    Ymir::Tree ITupleInfo::toGeneric () {
	IInfoType::printConst (false);
	auto name = this-> innerTypeString ();
	IInfoType::printConst (true);
	Ymir::Tree tuple_type_node = IFinalFrame::getDeclaredType (name.c_str ());
	if (tuple_type_node.isNull ()) {
	    if (this-> params.size () != 0) {
		if (this-> _isUnion)
		    tuple_type_node = Ymir::makeUnion (name, this-> params);
		else
		    tuple_type_node = Ymir::makeTuple (name, this-> params);
	    } else
		tuple_type_node = Ymir::makeTuple (name, {new (Z0) ICharInfo (true)});
	    IFinalFrame::declareType (name, tuple_type_node);
	}
	return tuple_type_node;
    }

    Ymir::Tree ITupleInfo::genericConstructor () {
	vec<constructor_elt, va_gc> * elms = NULL;
	auto vtype = this-> toGeneric ();
	auto fields = Ymir::getFieldDecls (vtype);
	if (this-> params.size () != 0) {
	    if (this-> _isUnion) {
		ulong max_size = 0, index = 0;
		Ymir::Tree value;
		for (auto it : Ymir::r (0, this-> params.size ())) {
		    auto type = this-> params [it]-> toGeneric ();
		    if (TREE_INT_CST_LOW (TYPE_SIZE_UNIT (type.getTree ())) > max_size) { 
			value = this-> params [it]-> genericConstructor ();
			max_size = TREE_INT_CST_LOW (TYPE_SIZE_UNIT (type.getTree ()));
			index = it;
		    }
		}
		CONSTRUCTOR_APPEND_ELT (elms, fields [index].getTree (), value.getTree ());	    
	    } else {
		for (auto it : Ymir::r (0, this-> params.size ())) {
		    if (!this-> params [it]-> is <IVoidInfo> ())
			CONSTRUCTOR_APPEND_ELT (elms, fields [it].getTree (), this-> params [it]-> genericConstructor ().getTree ());
		}
	    }
	} else {
	    CONSTRUCTOR_APPEND_ELT (elms, fields [0].getTree (), build_int_cst_type (char_type_node, 0));
	}    
	return build_constructor (vtype.getTree (), elms);
    }
    
    const char* ITupleInfo::getId () {
	return ITupleInfo::id ();
    }

    InfoType ITupleInfo::Affect (Word, Expression right) {
	if (this-> isType ()) return NULL;
	if (auto tuple = right-> info-> type ()-> to <ITupleInfo> ()) {
	    if (!this-> isSame (tuple)) return NULL;
	    auto ret = this-> CompOp (tuple);
	    if (ret) ret = tuple-> ConstVerif (this);
	    if (ret == NULL) return NULL;
	    
	    ret-> binopFoo = &TupleUtils::InstAffect;
	    return ret;
	}
	return NULL;
    }
    
    InfoType ITupleInfo::AffectRight (Word, Expression left) {
	if (this-> isType ()) return NULL;
	if (left-> info-> type ()-> is <IUndefInfo> ()) {
	    auto ret = new (Z0)  ITupleInfo (false);
	    for (auto it : this-> params) {
		ret-> params.push_back (it-> clone ());
	    }

	    ret-> _isUnion = this-> _isUnion;
	    ret-> binopFoo = &TupleUtils::InstAffect;
	    return ret;
	}
	return NULL;
    }
    
    void ITupleInfo::addParam (InfoType type) {
	this-> params.push_back (type);
    }

    std::vector<InfoType> & ITupleInfo::getParams () {
	return this-> params;
    }

    std::vector<std::string> & ITupleInfo::getAttribs () {
	return this-> attribs;
    }
    
    InfoType ITupleInfo::Empty () {
	return NULL;
    }

    InfoType ITupleInfo::getTemplate (ulong nb) {
	if (nb < this-> params.size ())
	    return this-> params [nb];
	return NULL;
    }
    
    namespace TupleUtils {
	using namespace Ymir;

	template <typename T>
	T getAndRemoveBack (std::list <T> &list) {
	    if (list.size () != 0) {
		auto last = list.back ();	    
		list.pop_back ();
		return last;
	    } else {
		return (T) NULL;
	    }
	}
	
	Tree InstAffect (Word locus, InfoType, Expression left, Expression right) {
	    location_t loc = locus.getLocus ();
	    Ymir::TreeStmtList list;
	    auto ltree = Ymir::getExpr (list, left);
	    auto rtree = Ymir::getExpr (list, right);
	    if (ltree.getType () == rtree.getType ()) {
		list.append (buildTree (
		    MODIFY_EXPR, loc, ltree.getType (), ltree, rtree
		));
		
		return Ymir::compoundExpr (loc, list, ltree);		
	    } else {
		auto ptrl = Ymir::getAddr (loc, ltree).getTree ();
		auto ptrr = Ymir::getAddr (loc, rtree).getTree ();
		tree tmemcopy = builtin_decl_explicit (BUILT_IN_MEMCPY);
		tree size = TYPE_SIZE_UNIT (ltree.getType ().getTree ());
		auto result = build_call_expr_loc (loc, tmemcopy, 3, ptrl, ptrr, size);
		list.append (result);
		return Ymir::compoundExpr (loc, list, ltree);
	    }	   
	}

	Tree InstCast (Word locus, InfoType type, Expression elem, Expression) {
	    location_t loc = locus.getLocus ();
	    Ymir::TreeStmtList list;
	    auto rtree = Ymir::getExpr (list, elem);
	    auto ltype = type-> toGeneric ();	    
	    if (rtree.getType ().getTree () == ltype.getTree ()) {
	    	return Ymir::compoundExpr (loc, list, rtree);
	    } else {
		auto ltree = Ymir::makeAuxVar (loc, ISymbol::getLastTmp (), ltype);
		auto ptrl = Ymir::getAddr (loc, ltree).getTree ();
		auto ptrr = Ymir::getAddr (loc, rtree).getTree ();
		tree tmemcopy = builtin_decl_explicit (BUILT_IN_MEMCPY);
		tree size = TYPE_SIZE_UNIT (ltree.getType ().getTree ());
		auto result = build_call_expr_loc (loc, tmemcopy, 3, ptrl, ptrr, size);
		list.append (result);
		return Ymir::compoundExpr (loc, list, ltree);
	    }
	}

	Ymir::Tree InstSizeOf (Word, InfoType, Expression elem) {	    
	    return TYPE_SIZE_UNIT (elem-> info-> type ()-> toGeneric ().getTree ());
	}
	
	Tree InstCastFake (Word locus, InfoType type, Expression elem, Expression) {
	    location_t loc = locus.getLocus ();
	    TreeStmtList list;
	    auto rtree = Ymir::getExpr (list, elem);
	    auto ltype = type-> toGeneric ();

	    auto ltree = Ymir::makeAuxVar (loc, ISymbol::getLastTmp (), ltype);
	    auto info = type-> to <ITupleInfo> ();
	    auto elemInfo = elem-> info-> type ()-> to <ITupleInfo> ();
	    
	    for (auto it : Ymir::r (0, info-> nbParams ())) {
		auto laux = getField (loc, ltree, it);
		auto raux = getField (loc, rtree, it);
		auto relem = info-> getParams () [it]-> buildCastOp (
		    locus,
		    info-> getParams ()[it],
		    new (Z0) ITreeExpression (locus, elemInfo-> getParams ()[it], raux),
		    new (Z0) ITreeExpression (locus, info-> getParams ()[it], Ymir::Tree ())
		);
		
		list.append (buildTree (
		    MODIFY_EXPR, loc, void_type_node, laux, relem
		));		
	    }
	    
	    return Ymir::compoundExpr (locus.getLocus (), list.getTree (), ltree);			    
	}	

	Ymir::Tree InstCastTuple (Word locus, InfoType tuple, Expression elem, Expression) {
	    auto loc = locus.getLocus ();
	    auto type = tuple-> to <ITupleInfo> ()-> getParams () [tuple-> toGet ()];
	    type-> binopFoo = getAndRemoveBack (tuple-> nextBinop);
	    type-> nextBinop = tuple-> nextBinop;

	    type-> unopFoo = getAndRemoveBack (tuple-> nextUnop);
	    type-> nextUnop = tuple-> nextUnop;
	    
	    auto ltype = tuple-> toGeneric ();
	    auto ltree = Ymir::makeAuxVar (loc, ISymbol::getLastTmp (), ltype);
	    auto lelem = Ymir::getField (loc, ltree, tuple-> toGet ());

	    auto val = type-> buildCastOp (
		locus, type, elem, new (Z0) ITreeExpression (locus, type, Ymir::Tree ())
	    );

	    Ymir::TreeStmtList list;
	    if (lelem.getType () == val.getType ()) {
		list.append (buildTree (
		    MODIFY_EXPR, loc, void_type_node, lelem, val
		));
	    } else {
		auto ptrl = Ymir::getAddr (loc, lelem).getTree ();
		auto ptrr = Ymir::getAddr (loc, val).getTree ();
		tree tmemcopy = builtin_decl_explicit (BUILT_IN_MEMCPY);
		tree size = TYPE_SIZE_UNIT (lelem.getType ().getTree ());
		auto result = build_call_expr_loc (loc, tmemcopy, 3, ptrl, ptrr, size);
		list.append (result);
	    }
	    
	    return Ymir::compoundExpr (loc, list, ltree);
	}
	
	Tree InstGet (Word locus, InfoType, Expression left, Expression index) {
	    location_t loc = locus.getLocus ();
	    auto ltree = left-> toGeneric ();
	    auto value = index-> info-> value ()-> to <IFixedValue> ()-> getValue ();
	    return getField (loc, ltree, value);
	}

	Ymir::Tree InstAddr (Word locus, InfoType, Expression elem, Expression) {
	    return Ymir::getAddr (locus.getLocus (), elem-> toGeneric ());
	}	

	Tree InstInit (Word, InfoType type, Expression) {
	    return type-> genericConstructor ();
	}

	Ymir::Tree InstUnref (Word locus, InfoType t, Expression left, Expression right) {
	    auto type = t-> cloneOnExitWithInfo ();	    
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);

	    auto toget = type-> toGet ();	    
	    type-> toGet () = getAndRemoveBack (type-> nextToGet);

	    auto loc = locus.getLocus ();
	    InfoType innerType;
	    if (left-> info-> type ()-> is <IAggregateInfo> ())
		innerType = left-> info-> type ()-> to <IAggregateInfo> ()-> getImpl ()-> getParams () [toget];
	    else
		innerType = left-> info-> type ()-> to <ITupleInfo> ()-> getParams () [toget];
	    
	    Ymir::TreeStmtList list;
	    auto leftExp = Ymir::getExpr (list, left);
	    leftExp = getField (locus.getLocus (), leftExp, toget);
	    
	    return Ymir::compoundExpr (loc, list, type-> buildBinaryOp (
		locus,
		type,
		new (Z0) ITreeExpression (left-> token, innerType, leftExp),
		right
	    ));
	}
	
    }
    

}
