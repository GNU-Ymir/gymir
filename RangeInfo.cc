#include <ymir/semantic/types/_.hh>
#include <ymir/errors/Error.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/semantic/tree/_.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/value/Value.hh>
#include <ymir/semantic/utils/RangeUtils.hh>
#include <ymir/semantic/utils/FixedUtils.hh>
#include <ymir/semantic/object/AggregateInfo.hh>
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

    InfoType IRangeInfo::UnaryOp (Word op) {
	if (op == Token::AND && this-> isLvalue ()) {
	    auto ret = new (Z0) IPtrInfo (this-> isConst (), this-> clone ());
	    ret-> binopFoo = &RangeUtils::InstAddr;
	    return ret;
	}
	return NULL;
    }
    
    InfoType IRangeInfo::DotOp (Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "fst") return Fst ();
	if (var-> token == "scd") return Scd ();
	if (var-> token == "step") return Step ();
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
	} else if (auto ref = other-> to <IRefInfo> ()) {
	    if (this-> isLvalue () && this-> isSame (ref-> content ())) {
		auto aux = new (Z0) IRefInfo (this-> isConst (), this-> clone ());
		aux-> binopFoo = &RangeUtils::InstAddr;
		return aux;
	    }
	}
	return NULL;
    }

    InfoType IRangeInfo::ApplyOp (const std::vector <syntax::Var> & vars) {
	if (vars.size () != 1) return NULL;	
	vars [0]-> info-> type (this-> _content-> clone ()-> CompOp (vars [0]-> info-> type ()));		
	if (vars [0]-> info-> type () == NULL) return NULL;
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
	if (this-> isSame (right-> info-> type ())) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &RangeUtils::InstAffect;
	    return ret;
	}
	return NULL;
    }
	
    InfoType IRangeInfo::AffectRight (Expression left) {
	if (left-> info-> type ()-> is <IUndefInfo> ()) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &RangeUtils::InstAffect;
	    return ret;
	}
	return NULL;
    }
    
    InfoType IRangeInfo::In (Expression left) {
	if (this-> _content-> isSame (left-> info-> type ())) {
	    auto ret = new (Z0)  IBoolInfo (true);
	    ret-> binopFoo = &RangeUtils::InstIn;
	    return ret;
	}
	return NULL;
    }

    bool& IRangeInfo::isInclusive () {
	return this-> _include;
    }
    
    Value & IRangeInfo::leftValue () {
	return this-> left;
    }

    Value & IRangeInfo::rightValue () {
	return this-> right;
    }
    
    InfoType IRangeInfo::Fst () {
	auto cst = this-> _content-> cloneOnExit ();
	cst = new (Z0) IArrayRefInfo (this-> isConst (), cst);	
	cst-> binopFoo = &RangeUtils::InstFst;
	return cst;
    }
    
    InfoType IRangeInfo::Scd () {
	auto cst = this-> _content-> cloneOnExit ();
	cst = new (Z0) IArrayRefInfo (this-> isConst (), cst);	
	cst-> binopFoo = &RangeUtils::InstScd;
	return cst;
    }
    
    InfoType IRangeInfo::Step () {
	auto cst = this-> _content-> cloneOnExit ();
	cst = new (Z0) IArrayRefInfo (this-> isConst (), cst);	
	cst-> binopFoo = &RangeUtils::InstStep;
	return cst;
    }
    
    Ymir::Tree IRangeInfo::toGenericStatic (std::string innerName, Ymir::Tree inner) {
	std::string name = simpleTypeStringStatic (innerName);
	Ymir::Tree range_type_node = IFinalFrame::getDeclaredType (name.c_str ());
	if (range_type_node.isNull ()) {
	    range_type_node = Ymir::makeStructType (name, 4,
						    get_identifier ("in"),
						    boolean_type_node,
						    get_identifier ("step"),
						    inner.getTree (),
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
	    range_type_node = Ymir::makeStructType (name, 4,
						    get_identifier ("in"),
						    boolean_type_node,
						    get_identifier ("step"),
						    inner.getTree (),
						    get_identifier ("fst"),
						    inner.getTree (),				      
						    get_identifier ("scd"),
						    inner.getTree ()
	    );
	    
	    IFinalFrame::declareType (name, range_type_node);
	}
	return range_type_node;
    }

    Ymir::Tree IRangeInfo::genericConstructor () {
	auto vtype = this-> toGeneric ();
	vec <constructor_elt, va_gc> * elms = NULL;
	CONSTRUCTOR_APPEND_ELT (elms, getFieldDecl (vtype, "in").getTree (), build_int_cst_type (unsigned_char_type_node, 0));
	CONSTRUCTOR_APPEND_ELT (elms, getFieldDecl (vtype, "step").getTree (), this-> _content-> genericConstructor ().getTree ());
	CONSTRUCTOR_APPEND_ELT (elms, getFieldDecl (vtype, "fst").getTree (), this-> _content-> genericConstructor ().getTree ());
	CONSTRUCTOR_APPEND_ELT (elms, getFieldDecl (vtype, "scd").getTree (), this-> _content-> genericConstructor ().getTree ());
	return build_constructor (vtype.getTree (), elms);
    }
    
    InfoType IRangeInfo::getTemplate (ulong i) {
	if (i == 0) return this-> _content;
	return NULL;
    }

    Ymir::Tree IRangeInfo::genericTypeInfo () {
	auto inner = this-> content ()-> genericTypeInfo ();

	auto type = Table::instance ().getTypeInfoType ()-> TempOp ({});
	auto typeTree = type-> toGeneric ();
	vec <constructor_elt, va_gc> * elms = NULL;
		    
	auto ptr_info_type = Table::instance ().getTypeInfoType (Ymir::Runtime::RANGE_INFO)-> TempOp ({})-> to <IAggregateInfo> ();
	auto vtable = ptr_info_type-> getVtable ();
	    
	CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Keys::VTABLE_FIELD).getTree (), Ymir::getAddr (vtable).getTree ());	   
	CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Ymir::Runtime::VTABLE_FIELD_TYPEINFO).getTree (), build_int_cst_type (long_unsigned_type_node, 0));
	CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Ymir::Runtime::LEN_FIELD_TYPEINFO).getTree (), build_int_cst_type (long_unsigned_type_node, 0));
	CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Ymir::Runtime::C_O_A_TYPEINFO).getTree (), getAddr (inner).getTree ());
	
	auto name = Ymir::Runtime::TYPE_INFO_MODULE + "." + this-> innerSimpleTypeString () + Ymir::Runtime::TYPE_INFO_SUFFIX;
	auto glob = Ymir::declareGlobalWeak (name, typeTree, build_constructor (typeTree.getTree (), elms));

	return glob;
    }
    
    
    namespace RangeUtils {
	using namespace syntax;
	using namespace Ymir;

	Tree InstAffect (Word locus, InfoType, Expression left, Expression right) {
	    location_t loc = locus.getLocus ();
	    TreeStmtList list;
	    
	    auto ltree = left-> toGeneric ();
	    auto rtree = right-> toGeneric ();

	    if (ltree.getType () != rtree.getType ()) {
		auto ptrl = Ymir::getAddr (loc, ltree).getTree ();
		auto ptrr = Ymir::getAddr (loc, rtree).getTree ();
		tree tmemcopy = builtin_decl_explicit (BUILT_IN_MEMCPY);
		tree size = TYPE_SIZE_UNIT (ltree.getType ().getTree ());	    
		auto result = build_call_expr_loc (loc, tmemcopy, 3, ptrl, ptrr, size);
		list.append (result);
		return Ymir::compoundExpr (loc, list, ltree);
	    } else {
		list.append (buildTree (
		    MODIFY_EXPR, loc, void_type_node, ltree, rtree 
		));
		return Ymir::compoundExpr (loc, list, ltree);
	    }	    
	}
	
	Tree InstIn (Word locus, InfoType, Expression left, Expression right) {
	    location_t loc = locus.getLocus ();
	    auto ltree = left-> toGeneric ();
	    auto rtree = right-> toGeneric ();
	    auto inclusive = getField (loc, rtree, "in"); //right-> info-> type ()-> to<IRangeInfo> ()-> isInclusive ();
	    auto rfst = getField (loc, rtree, "fst"), rscd = getField (loc, rtree, "scd");

	    tree_code left_fst = OperatorUtils::toGeneric (Token::INF_EQUAL);
	    tree_code ge_scd = OperatorUtils::toGeneric (Token::SUP_EQUAL), gt_scd = OperatorUtils::toGeneric (Token::SUP);
	    tree_code left_and = OperatorUtils::toGeneric (Token::AND);
	    
	    auto left_test = buildTree (
		left_and, loc, boolean_type_node,
		buildTree (left_fst, loc, boolean_type_node, rfst, ltree),
		buildTree (COND_EXPR, loc, boolean_type_node, inclusive, 
			   buildTree (ge_scd, loc, boolean_type_node, rscd, ltree),
			   buildTree (gt_scd, loc, boolean_type_node, rscd, ltree)
		)
	    );

	    tree_code right_fst = OperatorUtils::toGeneric (Token::SUP_EQUAL);
	    tree_code le_scd = OperatorUtils::toGeneric (Token::INF_EQUAL), lt_scd = OperatorUtils::toGeneric (Token::INF);
	    tree_code right_and = OperatorUtils::toGeneric (Token::AND);
	    
	    auto right_test = buildTree (
		right_and, loc, boolean_type_node,
		buildTree (right_fst, loc, boolean_type_node, rfst, ltree),
		buildTree (COND_EXPR, loc, boolean_type_node, inclusive, 
			   buildTree (le_scd, loc, boolean_type_node, rscd, ltree),
			   buildTree (lt_scd, loc, boolean_type_node, rscd, ltree)
		)
	    );

	    tree_code test_op = OperatorUtils::toGeneric (Token::INF);
	    auto test = buildTree (
		test_op, loc, boolean_type_node, rfst, rscd
	    );
	    
	    auto result = buildTree (
	    	COND_EXPR, loc, boolean_type_node, test, left_test, right_test
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
	
	Tree InstStep (Word locus, InfoType, Expression left, Expression) {
	    location_t loc = locus.getLocus ();
	    auto ltree = left-> toGeneric ();
	    return getField (loc, ltree, "step");
	}

	Tree InstCast (Word, InfoType, Expression left, Expression) {
	    return left-> toGeneric ();
	}

	Tree InstApply (Word locus, InfoType, std::vector <Var> & vars, Block block, Expression iter) {
	    auto loc = locus.getLocus ();
	    auto range = iter-> toGeneric ();
	    auto rangeInfo = (RangeInfo) iter-> info-> type ();
	    auto innerInfo = rangeInfo-> content ()-> toGeneric ();
	    auto inclusive = getField (loc, range, "in");
	    auto one = getField (loc, range, "step");
	    auto scd = getField (loc, range, "scd");
	    auto begin = getField (loc, range, "fst");
	    //auto one = convert (innerInfo.getTree (), build_int_cst_type (integer_type_node, 1));
	    
	    auto it = makeAuxVar (loc, ISymbol::getLastTmp (), innerInfo);
	    auto var = vars [0]-> toGeneric ();
	    
	    Ymir::TreeStmtList list, begin_part;
	    
	    list.append (buildTree (MODIFY_EXPR, loc, void_type_node, it, begin));
	    Ymir::Tree bool2_expr = buildTree (LT_EXPR, loc, boolean_type_node, begin, scd);
	    Ymir::Tree bool_expr;
	    auto lt_expr = buildTree (LT_EXPR, loc, boolean_type_node, it, scd);
	    auto gt_expr = buildTree (GT_EXPR, loc, boolean_type_node, it, scd);
	    auto le_expr = buildTree (LE_EXPR, loc, boolean_type_node, it, scd);
	    auto ge_expr = buildTree (GE_EXPR, loc, boolean_type_node, it, scd);
	    
	    bool_expr = buildTree (COND_EXPR, loc, boolean_type_node, inclusive, 
				   buildTree (COND_EXPR, loc, boolean_type_node, bool2_expr, le_expr, ge_expr),
				   buildTree (COND_EXPR, loc, boolean_type_node, bool2_expr, lt_expr, gt_expr)
	    );
	    
	    
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
	    
	    //Ymir::Tree bool2_expr = buildTree (LT_EXPR, loc, boolean_type_node, begin, scd);
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

	Ymir::Tree InstAddr (Word locus, InfoType, Expression elem, Expression) {
	    return Ymir::getAddr (locus.getLocus (), elem-> toGeneric ());
	}
	
    }
    
}
