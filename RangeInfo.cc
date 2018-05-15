#include <ymir/semantic/types/_.hh>
#include <ymir/errors/Error.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/value/Value.hh>
#include <ymir/semantic/utils/RangeUtils.hh>
#include <ymir/semantic/utils/FixedUtils.hh>
using namespace syntax;

namespace semantic {

    IRangeInfo::IRangeInfo (bool isConst) :
	IInfoType (isConst),
	_content (new (Z0)  IVoidInfo ())
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
	else if (var-> token == "sizeof") {
	    auto ret = new (Z0)  IFixedInfo (true, FixedConst::UINT);
	    ret-> unopFoo = FixedUtils::InstSizeOf;
	    return ret;	
	}
	return NULL;       
    }

    InfoType IRangeInfo::DColonOp (Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "typeid") return StringOf ();
	return NULL;       
    }
        
    InfoType IRangeInfo::CompOp (InfoType other) {
	if (other-> is<IUndefInfo> () || other-> isSame (this)) {
	    auto ra = this-> clone ();
	    ra-> binopFoo = &RangeUtils::InstCast;
	    return ra;
	} else return NULL;
    }

    InfoType IRangeInfo::ApplyOp (const std::vector <syntax::Var> & vars) {
	if (vars.size () != 1) return NULL;	
	vars [0]-> info-> type = this-> _content-> clone ()-> CompOp (vars [0]-> info-> type);		
	if (vars [0]-> info-> type == NULL) return NULL;
	vars [0]-> info-> value () = NULL;
	auto ret = this-> clone ();
	ret-> applyFoo = &RangeUtils::InstApply;
	return ret;	
    }

    InfoType IRangeInfo::onClone () {
	return new (Z0)  IRangeInfo (this-> isConst (), this-> _content-> clone ());
    }

    InfoType IRangeInfo::content () {
	return this-> _content;
    }
    
    const char * IRangeInfo::getId () {
	return IRangeInfo::id ();
    }
    
    std::string IRangeInfo::innerTypeString () {
	return "r!" + this-> _content-> innerTypeString ();
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
	    auto ret = new (Z0)  IBoolInfo (true);
	    ret-> binopFoo = &RangeUtils::InstIn;
	    return ret;
	}
	return NULL;
    }

    Value & IRangeInfo::leftValue () {
	return this-> left;
    }

    Value & IRangeInfo::rightValue () {
	return this-> right;
    }
    
    InfoType IRangeInfo::Fst () {
	auto cst = this-> _content-> clone ();
	cst-> isConst (this-> isConst ());
	cst-> binopFoo = &RangeUtils::InstFst;
	return cst;
    }
    
    InfoType IRangeInfo::Scd () {
	auto cst = this-> _content-> clone ();
	cst-> isConst (this-> isConst ());
	cst-> value () = NULL;
	cst-> binopFoo = &RangeUtils::InstScd;
	return cst;
    }

    Ymir::Tree IRangeInfo::toGenericStatic (std::string innerName, Ymir::Tree inner) {
	std::string name = simpleTypeStringStatic (innerName);
	Ymir::Tree range_type_node = IFinalFrame::getDeclaredType (name.c_str ());
	if (range_type_node.isNull ()) {
	    range_type_node = Ymir::makeStructType (name, 2,
						    get_identifier ("fst"),
						    inner.getTree (),				      
						    get_identifier ("scd"),
						    inner.getTree ()
	    );
	    
	    IFinalFrame::declareType (name, range_type_node);
	}
	return range_type_node;
    }
    
    Ymir::Tree IRangeInfo::toGeneric () {
	Ymir::Tree inner = this-> _content-> toGeneric ();
	std::string name = this-> simpleTypeString ();
	Ymir::Tree range_type_node = IFinalFrame::getDeclaredType (name.c_str ());
	if (range_type_node.isNull ()) {
	    range_type_node = Ymir::makeStructType (name, 2,
						    get_identifier ("fst"),
						    inner.getTree (),				      
						    get_identifier ("scd"),
						    inner.getTree ()
	    );
	    
	    IFinalFrame::declareType (name, range_type_node);
	}
	return range_type_node;
    }

    InfoType IRangeInfo::getTemplate (ulong i) {
	if (i == 0) return this-> _content;
	return NULL;
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
	    
	    return Ymir::compoundExpr (loc, list.getTree (), ltree);
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

	Tree InstFst (Word locus, InfoType, Expression left, Expression) {
	    location_t loc = locus.getLocus ();
	    auto ltree = left-> toGeneric ();
	    return getField (loc, ltree, "fst");
	}

	Tree InstScd (Word locus, InfoType, Expression left, Expression) {
	    location_t loc = locus.getLocus ();
	    auto ltree = left-> toGeneric ();
	    return getField (loc, ltree, "scd");
	}
	
	Tree InstCast (Word, InfoType, Expression left, Expression) {
	    return left-> toGeneric ();
	}

	Tree InstApply (Word locus, InfoType, std::vector <Var> & vars, Block block, Expression iter) {
	    auto loc = locus.getLocus ();
	    auto range = iter-> toGeneric ();
	    auto rangeInfo = (RangeInfo) iter-> info-> type;
	    auto innerInfo = rangeInfo-> content ()-> toGeneric ();
	    auto scd = getField (loc, range, "scd");
	    auto begin = getField (loc, range, "fst");
	    auto one = convert (innerInfo.getTree (), build_int_cst_type (integer_type_node, 1));
	    
	    auto it = makeAuxVar (loc, ISymbol::getLastTmp (), innerInfo);
	    auto var = vars [0]-> toGeneric ();
	    
	    Ymir::TreeStmtList list, begin_part;
	    
	    list.append (buildTree (MODIFY_EXPR, loc, void_type_node, it, begin));
	    Ymir::Tree bool_expr = buildTree (NE_EXPR, loc, boolean_type_node, it, scd);
	    
	    Ymir::Tree test_label = Ymir::makeLabel (loc, "test");
	    Ymir::Tree begin_label = Ymir::makeLabel (loc, "begin");
	    Ymir::Tree end_label = Ymir::makeLabel (loc, "end");

	    Ymir::Tree goto_test = Ymir::buildTree (GOTO_EXPR, iter-> token.getLocus (), void_type_node, test_label);
	    Ymir::Tree goto_end = Ymir::buildTree (GOTO_EXPR, iter-> token.getLocus (), void_type_node, end_label);
	    Ymir::Tree goto_begin = Ymir::buildTree (GOTO_EXPR, iter-> token.getLocus (), void_type_node, begin_label);
	
	    Ymir::Tree test_expr = Ymir::buildTree (COND_EXPR, iter-> token.getLocus (), void_type_node, bool_expr, goto_begin, goto_end);
	    Ymir::Tree begin_label_expr = Ymir::buildTree (LABEL_EXPR, block-> token.getLocus (), void_type_node, begin_label);
	    list.append (goto_test);
	    list.append (begin_label_expr);

	    begin_part.append (Ymir::buildTree (MODIFY_EXPR, loc, void_type_node, var, it));
	    auto innerBl = block-> toGeneric ();
	    begin_part.append (innerBl);	    
	    auto add_expr = Ymir::buildTree (
	     	MODIFY_EXPR, locus.getLocus (), it.getType (), it,
	     	Ymir::buildTree (
	     	    PLUS_EXPR, locus.getLocus (), it.getType (), it, one
	     	)
	    );

	    auto sub_expr = Ymir::buildTree (
	     	MODIFY_EXPR, locus.getLocus (), it.getType (), it,
	     	Ymir::buildTree (
	     	    MINUS_EXPR, locus.getLocus (), it.getType (), it, one
	     	)
	    );
	    
	    Ymir::Tree bool2_expr = buildTree (LT_EXPR, loc, boolean_type_node, begin, scd);
	    begin_part.append (Ymir::buildTree (COND_EXPR, iter-> token.getLocus (), void_type_node, bool2_expr, add_expr, sub_expr));
	    
	    list.append (begin_part.getTree ());	    
	    list.append (goto_test);
	    
	    Ymir::Tree test_label_expr = Ymir::buildTree (LABEL_EXPR, iter-> token.getLocus (), void_type_node, test_label);
	    list.append (test_label_expr);
	    list.append (test_expr);

	    Ymir::Tree end_expr = Ymir::buildTree (LABEL_EXPR, iter-> token.getLocus (), void_type_node, end_label);	
	    list.append (end_expr);

	    return list.getTree ();
	    
	}

    }
    
}
