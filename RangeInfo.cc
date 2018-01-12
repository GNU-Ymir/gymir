#include <ymir/semantic/types/_.hh>
#include <ymir/errors/Error.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/utils/RangeUtils.hh>
using namespace syntax;

namespace semantic {

    IRangeInfo::IRangeInfo (bool isConst) :
	IInfoType (isConst),
	_content (new IVoidInfo ())
    {}
    
    IRangeInfo::IRangeInfo (bool isConst, InfoType content) :
	IInfoType (isConst),
	_content (content)
    {}

    bool IRangeInfo::isSame (InfoType other) {
	if (auto ot = other-> to<IRangeInfo> ()) {
	    return ot-> _content-> isSame (this-> _content);
	}
	return false;
    }
        
    InfoType IRangeInfo::BinaryOp (Word token, Expression right) {
	if (token == Token::EQUAL) return Affect (right);
	return NULL;
    }
    
    InfoType IRangeInfo::BinaryOpRight (Word token, Expression left) {
	if (token == Token::EQUAL) return AffectRight (left);
	else if (token == Keys::IN) return In (left);
	return NULL;
    }
    
    InfoType IRangeInfo::DotOp (Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "fst") return Fst ();
	if (var-> token == "scd") return Scd ();
	return NULL;       
    }
        
    InfoType IRangeInfo::CompOp (InfoType other) {
	if (other-> is<IUndefInfo> () || other-> isSame (this)) {
	    auto ra = this-> clone ();
	    ra-> binopFoo = &RangeUtils::InstCast;
	    return ra;
	} else return NULL;
    }

    InfoType IRangeInfo::clone () {
	return new IRangeInfo (this-> isConst (), this-> _content-> clone ());
    }
    
    const char * IRangeInfo::getId () {
	return IRangeInfo::id ();
    }
    
    std::string IRangeInfo::innerTypeString () {
	return "range!" + this-> _content-> innerTypeString ();
    }

    std::string IRangeInfo::innerSimpleTypeString () {
	return "R_" + this-> _content-> simpleTypeString ();
    }

    std::string IRangeInfo::simpleTypeStringStatic (std::string & inner) {
	return "R_" + inner;
    }

    InfoType IRangeInfo::Affect (Expression right) {
	if (this-> isSame (right-> info-> type)) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &RangeUtils::InstAffect;
	    return ret;
	}
	return NULL;
    }
	
    InfoType IRangeInfo::AffectRight (Expression left) {
	if (left-> info-> type-> is <IUndefInfo> ()) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &RangeUtils::InstAffect;
	    return ret;
	}
	return NULL;
    }
    
    InfoType IRangeInfo::In (Expression left) {
	if (this-> _content-> isSame (left-> info-> type)) {
	    auto ret = new IBoolInfo (true);
	    ret-> binopFoo = &RangeUtils::InstIn;
	    return ret;
	}
	return NULL;
    }
    
    InfoType IRangeInfo::Fst () {
	auto cst = this-> _content-> clone ();
	cst-> isConst (this-> isConst ());
	cst-> unopFoo = &RangeUtils::InstFst;
	return cst;
    }
    
    InfoType IRangeInfo::Scd () {
	auto cst = this-> _content-> clone ();
	cst-> isConst (this-> isConst ());
	cst-> unopFoo = &RangeUtils::InstScd;
	return cst;
    }

    Ymir::Tree IRangeInfo::toGenericStatic (std::string innerName, Ymir::Tree inner) {
	std::string name = simpleTypeStringStatic (innerName);
	Ymir::Tree range_type_node =
	    Ymir::makeStructType (name, 2,
				  get_identifier ("fst"),
				  inner.getTree (),				      
				  get_identifier ("scd"),
				  inner.getTree ()
	    );
	
	IFinalFrame::declareType (name, range_type_node);
	return range_type_node;
    }
    
    Ymir::Tree IRangeInfo::toGeneric () {
	Ymir::Tree inner = this-> _content-> toGeneric ();
	std::string name = this-> simpleTypeString ();
	Ymir::Tree range_type_node =
	    Ymir::makeStructType (name, 2,
				  get_identifier ("fst"),
				  inner.getTree (),				      
				  get_identifier ("scd"),
				  inner.getTree ()
	    );
	
	IFinalFrame::declareType (name, range_type_node);
	return range_type_node;
    }

    namespace RangeUtils {
	using namespace syntax;
	using namespace Ymir;

	Tree InstAffect (Word locus, InfoType, Expression left, Expression right) {
	    location_t loc = locus.getLocus ();
	    TreeStmtList list;
	    
	    auto ltree = left-> toGeneric ();
	    auto rtree = right-> toGeneric ();

	    auto lfst = getField (loc, ltree, "fst"), lscd = getField (loc, ltree, "scd");
	    auto rfst = getField (loc, rtree, "fst"), rscd = getField (loc, rtree, "scd");

	    list.append (buildTree (
		MODIFY_EXPR, loc, void_type_node, lfst.getTree (), rfst.getTree ()
	    ));

	    list.append (buildTree (
		MODIFY_EXPR, loc, void_type_node, lscd.getTree (), rscd.getTree ()
	    ));
	    
	    getStackStmtList ().back ().append (list.getTree ());
	    return ltree;
	}
	
	Tree InstIn (Word locus, InfoType, Expression left, Expression right) {
	    location_t loc = locus.getLocus ();
	    auto ltree = left-> toGeneric ();
	    auto rtree = right-> toGeneric ();
	    auto rfst = getField (loc, rtree, "fst"), rscd = getField (loc, rtree, "scd");

	    tree_code left_fst = OperatorUtils::toGeneric (Token::INF_EQUAL);
	    tree_code left_scd = OperatorUtils::toGeneric (Token::SUP);
	    tree_code left_and = OperatorUtils::toGeneric (Token::AND);
	    
	    auto left_test = buildTree (
		left_and, loc, boolean_type_node,
		buildTree (left_fst, loc, boolean_type_node, rfst, ltree),
		buildTree (left_scd, loc, boolean_type_node, rscd, ltree)
	    );

	    tree_code right_fst = OperatorUtils::toGeneric (Token::SUP_EQUAL);
	    tree_code right_scd = OperatorUtils::toGeneric (Token::INF);
	    tree_code right_and = OperatorUtils::toGeneric (Token::AND);
	    
	    auto right_test = buildTree (
		right_and, loc, boolean_type_node,
		buildTree (right_fst, loc, boolean_type_node, rfst, ltree),
		buildTree (right_scd, loc, boolean_type_node, rscd, ltree)
	    );

	    tree_code test_op = OperatorUtils::toGeneric (Token::INF);
	    auto test = buildTree (
		test_op, loc, boolean_type_node, rfst, rscd
	    );
	    
	    auto result = buildTree (
		COND_EXPR, loc, void_type_node, test, left_test, right_test
	    );
	    
	    return result;
	}

	Tree InstFst (Word locus, InfoType, Expression left) {
	    location_t loc = locus.getLocus ();
	    auto ltree = left-> toGeneric ();
	    return getField (loc, ltree, "fst");
	}

	Tree InstScd (Word locus, InfoType, Expression left) {
	    location_t loc = locus.getLocus ();
	    auto ltree = left-> toGeneric ();
	    return getField (loc, ltree, "scd");
	}
	
	Tree InstCast (Word, InfoType, Expression left, Expression) {
	    return left-> toGeneric ();
	}

    }
    
}
