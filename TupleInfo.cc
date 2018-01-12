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
	IInfoType (isConst)
    {}

    ulong ITupleInfo::nbParams () {
	return this-> params.size ();
    }

    InfoType ITupleInfo::ConstVerif (InfoType other) {
	if (auto tuple = other-> to <ITupleInfo> ()) {
	    auto other = new ITupleInfo (IInfoType::isConst ());
	    for (auto it : Ymir::r (0, this-> params.size ())) {
		auto res = this-> params [it]-> ConstVerif (tuple-> params [it]);
		if (res == NULL) return NULL;
		else other-> addParam (res);
	    }
	    return other;
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
    
    InfoType ITupleInfo::create (Word, std::vector <syntax::Expression> templates) {
	auto tuple = new ITupleInfo (false);
	for (auto it : Ymir::r (0, templates.size ())) {
	    tuple-> params.push_back (templates [it]-> info-> type);
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
	if (this-> isType ()) return NULL;
	if (other-> isSame (this) || other-> is <IUndefInfo> ()) {
	    auto ret = new ITupleInfo (IInfoType::isConst ());
	    for (auto it : this-> params) {
		ret-> params.push_back (it-> BinaryOp (tok, it));
		//TODO ret-> params.back ()-> value = NULL;
	    }
	    
	    ret-> binopFoo = &TupleUtils::InstCast;
	    return ret;
	} else if (auto ot = other->to <IRefInfo> ()) {
	    if (!this-> isConst () && ot-> content ()-> isSame (this)) {
		auto aux = new (GC) IRefInfo (false, this-> clone ());
		aux-> binopFoo = &TupleUtils::InstAddr;
		return aux;
	    }
	}
	return NULL;
    }
		
    InfoType ITupleInfo::clone () {
	auto tu = new ITupleInfo (IInfoType::isConst ());
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
		ret-> binopFoo = &TupleUtils::InstGet;
		return ret;
	    }
	}
	return NULL;
    }

    InfoType ITupleInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	//if (var-> token == "typeid") return
	return NULL;
    }
	
    std::string ITupleInfo::innerTypeString () {
	Ymir::OutBuffer buf;
	if (this-> isType ()) buf.write ("t!(");
	else buf.write ("t(");
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    buf.write (this-> params [it]-> typeString ());
	    if (it != (int) this-> params.size () - 1) buf.write (", ");
	}
	buf.write (")");
	return buf.str ();
    }

    std::string ITupleInfo::simpleTypeString () {
	Ymir::OutBuffer buf;
	buf.write ("T");
	for (auto it : this-> params) {
	    buf.write (it-> simpleTypeString ());
	}
	return buf.str ();
    }
    
    Ymir::Tree ITupleInfo::toGeneric () {
	auto name = this-> simpleTypeString ();
	auto tuple_type_node = IFinalFrame::getDeclaredType (name.c_str ());
	if (tuple_type_node.isNull ()) {
	    tuple_type_node = Ymir::makeTuple (name, this-> params);	
	    IFinalFrame::declareType (name, tuple_type_node);
	}
	return tuple_type_node;
    }
	    
    const char* ITupleInfo::getId () {
	return ITupleInfo::id ();
    }

    InfoType ITupleInfo::Affect (Word tok, Expression right) {
	if (this-> isType ()) return NULL;
	if (auto tuple = right-> info-> type-> to <ITupleInfo> ()) {
	    if (tuple-> nbParams () != this-> nbParams ()) return NULL;
	    auto ret = new (GC) ITupleInfo (false);
	    for (auto it : Ymir::r (0, this-> params.size ())) {
		ret-> params.push_back (this-> params [it]-> BinaryOp (tok, tuple-> params [it]));
		if (ret-> params.back () == NULL) return NULL;
		//TODO ret-> params.back ()-> value = NULL;
	    }
	    ret-> binopFoo = &TupleUtils::InstAffect;
	    return ret;
	}
	return NULL;
    }
    
    InfoType ITupleInfo::AffectRight (Word tok, Expression left) {
	if (this-> isType ()) return NULL;
	if (left-> info-> type-> is <IUndefInfo> ()) {
	    auto ret = new ITupleInfo (false);
	    for (auto it : this-> params) {
		ret-> params.push_back (it-> BinaryOpRight (tok, left));
		//TODO ret-> params.back ()-> value = NULL;
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
	    TupleInfo info = type-> to<ITupleInfo> ();
	    auto rtype = right-> info-> type-> to <ITupleInfo> ();
	    Ymir::TreeStmtList list;
	    
	    for (auto it : Ymir::r (0, info-> nbParams ())) {
		auto laux = getField (loc, ltree, it);
		auto raux = getField (loc, rtree, it);
		auto ret = info-> getParams () [it]-> buildBinaryOp (
		    locus,
		    info-> getParams () [it],
		    new (GC) ITreeExpression (locus, info-> getParams () [it], laux),
		    new (GC) ITreeExpression (locus, rtype-> getParams () [it], raux)
		);
		list.append (ret);		
	    }
	    
	    getStackStmtList ().back ().append (list.getTree ());
	    return ltree;
	}

	Tree InstCast (Word locus, InfoType type, Expression elem, Expression) {
	    location_t loc = locus.getLocus ();
	    TupleInfo info = type-> to<ITupleInfo> ();
	    auto rtree = elem-> toGeneric ();
	    
	    auto rtype = elem-> info-> type-> to <ITupleInfo> ();
	    auto ltree = Ymir::makeAuxVar (loc, ISymbol::getLastTmp (), info-> toGeneric ());
	    Ymir::TreeStmtList list;
	    
	    for (auto it : Ymir::r (0, info-> nbParams ())) {
		auto laux = getField (loc, ltree, it);
		auto raux = getField (loc, rtree, it);
		auto ret = info-> getParams () [it]-> buildBinaryOp (
		    locus,
		    info-> getParams () [it],
		    new (GC) ITreeExpression (locus, info-> getParams () [it], laux),
		    new (GC) ITreeExpression (locus, rtype-> getParams () [it], raux)
		);
		list.append (ret);		
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
