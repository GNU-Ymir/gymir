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
using namespace syntax;

namespace semantic {

    ITupleInfo::ITupleInfo (bool isConst) :
	IInfoType (isConst),
	isFake (false)
    {}

    ITupleInfo::ITupleInfo (bool isConst, bool isFake) :
	IInfoType (isConst),
	isFake (isFake)
    {}
    
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
    
    bool ITupleInfo::isSame (InfoType other) {
	if (auto tu = other-> to <ITupleInfo> ()) {
	    if (tu-> params.size () != this-> params.size ()) return NULL;
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
	    if (auto ot = templates [it]-> info-> type-> to <IStructCstInfo> ()) {
		auto type = ot-> TempOp ({});
		if (type == NULL) return NULL;
		tuple-> params.push_back (type);
	    } else {
		tuple-> params.push_back (templates [it]-> info-> type);
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
	if (other-> isSame (this)) {	    
	    auto ret = new (Z0)  ITupleInfo (ot-> isConst ());
	    for (auto it : Ymir::r (0, ot-> params.size ())) {
		auto l = ot-> params [it];
		ret-> params.push_back (l-> clone ());
	    }
	    
	    ret-> binopFoo = &TupleUtils::InstCast;
	    return ret;
	} else if (other-> is <IUndefInfo> ()) {
	    auto ret = new (Z0)  ITupleInfo (this-> isConst ());
	    for (auto it : Ymir::r (0, this-> params.size ())) {
		auto l = this-> params [it];		
		ret-> params.push_back (l-> clone ());
	    }	    
	    ret-> binopFoo = &TupleUtils::InstCast;
	    return ret;
	} else if (auto ref = other->to <IRefInfo> ()) {
	    if (!this-> isConst () && ref-> content ()-> isSame (this)) {
		auto aux = new (Z0)  IRefInfo (false, this-> clone ());
		aux-> binopFoo = &TupleUtils::InstAddr;
		return aux;
	    }
	} else if (this-> isFake && ot && ot-> params.size () == this-> params.size ()) {
	    auto ret = new (Z0) ITupleInfo (ot-> isConst ());
	    for (auto it : Ymir::r (0, ot-> params.size ())) {
		auto l = this-> params [it]-> CompOp (ot-> params [it]);
		ret-> params.push_back (l);
	    }
	    
	    ret-> isFake = true;
	    ret-> binopFoo = &TupleUtils::InstCastFake;
	    return ret;
	}
	return NULL;
    }
		
    InfoType ITupleInfo::UnaryOp (Word op) {
	if (op == Token::AND) {
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
		return ret;
	    }
	}
	return NULL;
    }

    InfoType ITupleInfo::DColonOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "typeid") return StringOf ();
	if (var-> token == "arity") return Arity ();
	return NULL;
    }

    InfoType ITupleInfo::Arity () {
	auto ret = new (Z0) IFixedInfo (true, FixedConst::UINT);
	ret-> value () = new (Z0) IFixedValue (FixedConst::UINT, this-> params.size (), this-> params.size ());
	return ret;
    }
    
    std::string ITupleInfo::innerTypeString () {
	Ymir::OutBuffer buf;
	if (this-> isType ()) buf.write ("t!(");
	else buf.write ("t(");
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    buf.write (this-> params [it]-> typeString ());
	    if (this-> params [it]-> binopFoo)
		buf.write ("bin");
	    if (it != (int) this-> params.size () - 1) buf.write (", ");
	}
	buf.write (")");
	return buf.str ();
    }

    std::string ITupleInfo::innerSimpleTypeString () {
	Ymir::OutBuffer buf;
	buf.write ("T");
	for (auto it : this-> params) {
	    buf.write (it-> simpleTypeString ());
	}
	return buf.str ();
    }
    
    Ymir::Tree ITupleInfo::toGeneric () {
	IInfoType::printConst (false);
	auto name = this-> innerTypeString ();
	IInfoType::printConst (true);
	auto tuple_type_node = IFinalFrame::getDeclaredType (name.c_str ());
	if (tuple_type_node.isNull ()) {
	    if (this-> params.size () != 0) 
		tuple_type_node = Ymir::makeTuple (name, this-> params);
	    else
		tuple_type_node = Ymir::makeTuple (name, {new (Z0) ICharInfo (true)});
	    IFinalFrame::declareType (name, tuple_type_node);
	}
	return tuple_type_node;
    }
	    
    const char* ITupleInfo::getId () {
	return ITupleInfo::id ();
    }

    InfoType ITupleInfo::Affect (Word, Expression right) {
	if (this-> isType ()) return NULL;
	if (auto tuple = right-> info-> type-> to <ITupleInfo> ()) {
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
	if (left-> info-> type-> is <IUndefInfo> ()) {
	    auto ret = new (Z0)  ITupleInfo (false);
	    for (auto it : this-> params) {
		ret-> params.push_back (it-> clone ());
	    }

	    ret-> binopFoo = &TupleUtils::InstAffect;
	    return ret;
	}
	return NULL;
    }
    
    void ITupleInfo::addParam (InfoType type) {
	this-> params.push_back (type-> clone ());
    }

    std::vector<InfoType> & ITupleInfo::getParams () {
	return this-> params;
    }
    
    InfoType ITupleInfo::SizeOf () {
	return NULL;
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
	
	Tree InstAffect (Word locus, InfoType type, Expression left, Expression right) {
	    location_t loc = locus.getLocus ();
	    auto ltree = left-> toGeneric ();
	    auto rtree = right-> toGeneric ();	    
	    Ymir::TreeStmtList list;
	    if (ltree.getType ().getTree () == rtree.getType ().getTree ()) {
		return buildTree (
		    MODIFY_EXPR, loc, ltree.getType (), ltree, rtree
		);		
	    } else {
		auto info = type-> to <ITupleInfo> ();
		for (auto it : Ymir::r (0, info-> nbParams ())) {
		    auto laux = getField (loc, ltree, it);
		    auto raux = getField (loc, rtree, it);
		    list.append (buildTree (
			MODIFY_EXPR, loc, void_type_node, laux, raux
		    ));		
		}
		
		getStackStmtList ().back ().append (list.getTree ());
		return ltree;
	    }	   
	}

	Tree InstCast (Word locus, InfoType type, Expression elem, Expression) {
	    location_t loc = locus.getLocus ();
	    auto rtree = elem-> toGeneric ();
	    auto ltype = type-> toGeneric ();
	    if (rtree.getType ().getTree () == ltype.getTree ())
	    	return rtree;
	    else {
		TreeStmtList list;
		auto ltree = Ymir::makeAuxVar (loc, ISymbol::getLastTmp (), ltype);
		auto info = type-> to <ITupleInfo> ();
		for (auto it : Ymir::r (0, info-> nbParams ())) {
		    auto laux = getField (loc, ltree, it);
		    auto raux = getField (loc, rtree, it);
		    list.append (buildTree (
					    MODIFY_EXPR, loc, void_type_node, laux, raux
					    ));		
		}
		
		getStackStmtList ().back ().append (list.getTree ());
		return ltree;		
	    }
	}

	Tree InstCastFake (Word locus, InfoType type, Expression elem, Expression) {
	    location_t loc = locus.getLocus ();
	    auto rtree = elem-> toGeneric ();
	    auto ltype = type-> toGeneric ();
	    TreeStmtList list;
	    auto ltree = Ymir::makeAuxVar (loc, ISymbol::getLastTmp (), ltype);
	    auto info = type-> to <ITupleInfo> ();
	    auto elemInfo = elem-> info-> type-> to <ITupleInfo> ();
	    
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
	    
	    getStackStmtList ().back ().append (list.getTree ());
	    return ltree;			    
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
    }
    

}
