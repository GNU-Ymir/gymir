#include <ymir/tree/Tree.hh>
#include <ymir/tree/StmtList.hh>
#include <ymir/tree/ChainBase.hh>
#include "config.h"
#include "system.h"
#include "coretypes.h"

#include "target.h"
#include "tree-iterator.h"
#include "input.h"
#include "diagnostic.h"
#include "stringpool.h"
#include "cgraph.h"
#include "gimplify.h"
#include "gimple-expr.h"
#include "convert.h"
#include "print-tree.h"
#include "stor-layout.h"
#include "fold-const.h"

#include "convert.h"
#include "langhooks.h"
#include "ubsan.h"
#include "stringpool.h"
#include "attribs.h"
#include "asan.h"
#include "tree-pretty-print.h"
#include "print-tree.h"
#include "cppdefault.h"
#include "tm.h"

namespace generic {

    Tree::Tree () :
	_t (NULL_TREE),
	_loc (UNKNOWN_LOCATION)
    {}

    Tree Tree::init (const location_t & loc, const tree & t) {
	auto ret = Tree ();
	ret._t = t;
	ret._loc = loc;
	return ret;
    }

    Tree Tree::error () {
	auto ret = Tree ();
	ret._t = error_mark_node;
	ret._loc = UNKNOWN_LOCATION;
	return ret;
    }

    Tree Tree::empty () {
	return Tree ();
    }

    
    Tree Tree::block (const lexing::Word & loc, const Tree & vars, const Tree & chain) {
	return Tree::init (
	    loc.getLocation (), build_block (
		vars.getTree (), chain.getTree (), NULL_TREE, NULL_TREE
	    )
	);
    }
    
    Tree Tree::voidType () {
	return Tree::init (UNKNOWN_LOCATION, void_type_node);
    }

    Tree Tree::boolType () {
	return Tree::init (UNKNOWN_LOCATION, unsigned_char_type_node);
    }

    Tree Tree::intType (int size, bool isSigned) {
	if (isSigned) {
	    switch (size) {
	    case 8 : return Tree::init (UNKNOWN_LOCATION, signed_char_type_node);
	    case 16 : return Tree::init (UNKNOWN_LOCATION, short_integer_type_node);
	    case 32 : return Tree::init (UNKNOWN_LOCATION, integer_type_node);
	    case 64 : return Tree::init (UNKNOWN_LOCATION, long_integer_type_node);
	    default : return Tree::init (UNKNOWN_LOCATION, long_integer_type_node);
	    }
	} else {
	    switch (size) {
	    case 8 : return Tree::init (UNKNOWN_LOCATION, unsigned_char_type_node);
	    case 16 : return Tree::init (UNKNOWN_LOCATION, short_unsigned_type_node);
	    case 32 : return Tree::init (UNKNOWN_LOCATION, unsigned_type_node);
	    case 64 : return Tree::init (UNKNOWN_LOCATION, long_unsigned_type_node);
	    default : return Tree::init (UNKNOWN_LOCATION, long_unsigned_type_node);	       
	    } 
	}
    }

    Tree Tree::sizeType () {
	return Tree::init (UNKNOWN_LOCATION, long_unsigned_type_node);
    }
    
    Tree Tree::floatType (int size) {
	switch (size) {
	case 32: return Tree::init (UNKNOWN_LOCATION, float_type_node);
	case 64: return Tree::init (UNKNOWN_LOCATION, double_type_node);
	default : return Tree::init (UNKNOWN_LOCATION, double_type_node);
	}
    }

    Tree Tree::charType (int size) {
	switch (size) {
	case 8 : return Tree::init (UNKNOWN_LOCATION, unsigned_char_type_node);
	case 16 : return Tree::init (UNKNOWN_LOCATION, short_unsigned_type_node);
	case 32 : return Tree::init (UNKNOWN_LOCATION, unsigned_type_node);
	default : return Tree::init (UNKNOWN_LOCATION, unsigned_type_node);	    
	}
    }

    Tree Tree::staticArray (const Tree & inner, int size) {
	auto len = build_int_cst_type (long_unsigned_type_node, size - 1);
	auto begin = build_int_cst_type (long_unsigned_type_node, 0);
	auto range = Tree::init (UNKNOWN_LOCATION, build_range_type (integer_type_node, fold (begin), fold (len)));
	auto array_type = Tree::init (UNKNOWN_LOCATION, build_array_type (inner.getTree (), range.getTree ()));
	return array_type;
    }

    Tree Tree::sliceType (const Tree & inner) {
	auto len = Tree::intType (64, false);
	auto data = Tree::pointerType (inner);

	return Tree::tupleType ({"len", "ptr"}, {len, data});
    }    
    
    Tree Tree::pointerType (const Tree & innerType) {
	return Tree::init (UNKNOWN_LOCATION, build_pointer_type (innerType.getTree ()));
    }

    Tree Tree::tupleType (const std::vector <std::string> & attrs, const std::vector <Tree> & types) {
	return Tree::tupleType ("", attrs, types);
    }
    
    Tree Tree::tupleType (const std::string & name, const std::vector <std::string> & attrs, const std::vector <Tree> & types, bool isUnion, bool isPacked) {
	tree field_last = NULL_TREE, field_begin = NULL_TREE;
	tree record_type;
	if (isUnion)
	    record_type = make_node (UNION_TYPE);
	else record_type = make_node (RECORD_TYPE);
	
	auto size = 0;
	for (uint i = 0 ; i < types.size () ; i++) {
	    tree ident;
	    if (i >= attrs.size ()) {
		Ymir::OutBuffer buf;
		buf.write ("_", (int) i - attrs.size ());
		ident = get_identifier (buf.str ().c_str ());
	    } else ident = get_identifier (attrs [i].c_str ());
	    
	    tree type = types [i].getTree ();
	    size += TREE_INT_CST_LOW (TYPE_SIZE_UNIT (type));
	    tree field = build_decl (BUILTINS_LOCATION, FIELD_DECL, ident, type);
	    DECL_CONTEXT (field) = record_type;

	    if (field_begin == NULL_TREE) field_begin = field;
	    if (field_last != NULL_TREE) TREE_CHAIN (field_last) = field;
	    field_last = field;	    
	}

	TYPE_NAME (record_type) = get_identifier (name.c_str ());
	TREE_CHAIN (field_last) = NULL_TREE;
	TYPE_FIELDS (record_type) = field_begin;
	layout_type (record_type);
	
	if (isPacked && !isUnion) {
	    TYPE_SIZE (record_type) = bitsize_int (size * BITS_PER_UNIT);
	    TYPE_SIZE_UNIT (record_type) = size_int (size);
	    TYPE_PACKED (record_type) = 1;
	    SET_TYPE_ALIGN (record_type, 1 * BITS_PER_UNIT);
	    compute_record_mode (record_type);	
	}
	  

	return Tree::init (UNKNOWN_LOCATION, record_type);
    }
    
    Tree Tree::varDecl (const lexing::Word & loc, const std::string & name, const Tree & type) {
	return Tree::init (loc.getLocation (),
			   build_decl (
			       loc.getLocation (),
			       VAR_DECL,
			       get_identifier (name.c_str ()),
			       type.getTree ()
			   )
	);
    }

    Tree Tree::declExpr (const lexing::Word & loc, const Tree & decl) {
	return Tree::build (DECL_EXPR, loc, Tree::voidType (), decl);	
    }
    
    Tree Tree::paramDecl (const lexing::Word & loc, const std::string & name, const Tree & type) {
	return Tree::init (loc.getLocation (),
			   build_decl (
			       loc.getLocation (),
			       PARM_DECL,
			       get_identifier (name.c_str ()),
			       type.getTree ()
			   )
	);
    }

    Tree Tree::functionType (const Tree & retType, const std::vector <Tree> & params) {
	std::vector <tree> tree_params (params.size ());
	for (auto i : Ymir::r (0, params.size ()))
	    tree_params [i] = params [i].getTree ();
	
	return Tree::init (UNKNOWN_LOCATION,
			   build_function_type_array (
			       retType.getTree (),
			       tree_params.size (),
			       tree_params.data ()
			   )
	);
    }

    Tree Tree::functionDecl (const lexing::Word & loc, const std::string & name, const Tree & type) {
	return Tree::init (loc.getLocation (),
			   build_decl (
			       loc.getLocation (),
			       FUNCTION_DECL,
			       get_identifier (name.c_str ()),
			       type.getTree ()
			   )
	);			   
    }
    
    Tree Tree::resultDecl (const lexing::Word & loc, const Tree & type) {
	return Tree::init (
	    loc.getLocation (),
	    build_decl (
		loc.getLocation (),
		RESULT_DECL,
		NULL_TREE,
		type.getTree ()
	    )
	);			
    }    

    Tree Tree::affect (generic::TreeChain & stackContext, const generic::Tree & segmentContext, const lexing::Word & loc, const Tree & left, const Tree & right) {
	if (left.getType ().isScalar ()) {
	    return Tree::build (MODIFY_EXPR, loc, left.getType (), left,
				Tree::init (
				    loc.getLocation (),
				    convert (left.getType ().getTree (), right.getTree ())
				)
	    );
	} else {
	    if (left.getType () == right.getType ())
		return Tree::build (MODIFY_EXPR, loc, left.getType (), left, right);

	    Tree var = Tree::varDecl (loc, "#_aff", right.getType ());
	    var.setDeclContext (segmentContext);
	    stackContext.append (var);
	    
	    TreeStmtList list = TreeStmtList::init ();	    
	    
	    list.append (left.getList ());
	    list.append (right.getList ());
	    
	    auto lvalue = left.getValue ();
	    auto rvalue = right.getValue ();
	    
	    // If rvalue is a call expression, allocation etc, we must not recall it again
	    // So we put it in temp variable
	    list.append (Tree::build (MODIFY_EXPR, loc, right.getType (), var, right));
	    
	    auto ptrl = Tree::buildAddress (loc, lvalue, Tree::pointerType (lvalue.getType ()));
	    auto ptrr = Tree::buildAddress (loc, var, Tree::pointerType (rvalue.getType ()));
	    
	    auto tmemcopy = Tree::init (UNKNOWN_LOCATION, builtin_decl_explicit (BUILT_IN_MEMCPY));
	    auto size = Tree::init (UNKNOWN_LOCATION, TYPE_SIZE_UNIT (lvalue.getType ().getTree ()));
	    
	    list.append (
		Tree::init (loc.getLocation (), build_call_expr (tmemcopy.getTree (), 3, ptrl.getTree (), ptrr.getTree (), size.getTree ()))
	    );
	    
	    return  Tree::compound (
	    	loc,
		var, 
	    	list.toTree ()
	    );
	    
	}
    }

    Tree Tree::buildCall (const lexing::Word & loc, const Tree & type, const Tree & fn, const std::vector <Tree> & params) {
	std::vector <tree> real_params;
	for (auto it : params)
	    real_params.push_back (it.getTree ());
	
	return Tree::init (loc.getLocation (),
			   build_call_array_loc (loc.getLocation (), type.getTree (), fn.getTree (), real_params.size (), real_params.data ())
	);
    }

    Tree Tree::buildCall (const lexing::Word & loc, const Tree & type, const std::string & name, const std::vector <Tree> & params) {
	std::vector <tree> fndecl_type_params (params.size ());
	std::vector <tree> real_params (params.size ());
	for (auto it : Ymir::r (0, params.size ())) {
	    fndecl_type_params [it] = params [it].getType ().getTree ();
	    real_params [it] = params [it].getTree ();
	}

	tree fndecl_type = build_function_type_array (type.getTree (), fndecl_type_params.size (), fndecl_type_params.data ());
	tree fndecl = build_fn_decl (name.c_str (), fndecl_type);
	tree fn = build1 (ADDR_EXPR, Tree::pointerType (Tree::init (BUILTINS_LOCATION, fndecl_type)).getTree (), fndecl);

	return Tree::init (
	    loc.getLocation (),
	    build_call_array_loc (loc.getLocation (), type.getTree (), fn, real_params.size (), real_params.data ())
	);
    }

    Tree Tree::buildFrameProto (const lexing::Word & loc, const Tree & type, const std::string & name, const std::vector <Tree> & args) {
	std::vector <tree> fndecl_type_params (args.size ());
	for (auto it : Ymir::r (0, args.size ())) {
	    fndecl_type_params [it] = args [it].getTree ();
	}

	tree fndecl_type = build_function_type_array (type.getTree (), fndecl_type_params.size (), fndecl_type_params.data ());

	tree fndecl = build_fn_decl (name.c_str (), fndecl_type);
	return Tree::init (loc.getLocation (), build1 (ADDR_EXPR, Tree::pointerType (Tree::init (BUILTINS_LOCATION, fndecl_type)).getTree (), fndecl));
    }

    
    Tree Tree::buildArrayRef (const lexing::Word & loc, const Tree & array, const Tree & index) {
	auto inner = array.getType ().getType ();
	return Tree::build (ARRAY_REF, loc, inner, array, index, Tree::empty (), Tree::empty ());			   
    }

    
    Tree Tree::binary (const lexing::Word & loc, tree_code code, const Tree & type, const Tree & left, const Tree & right) {
	if (left.getType ().getSize () >= right.getType ().getSize ()) {
	    return Tree::build (code, loc, type, left,
				Tree::init (
				    loc.getLocation (),
				    convert (left.getType ().getTree (), right.getTree ())
				)
	    );
	} else {
	    return Tree::build (code, loc, type,
				Tree::init (
				    loc.getLocation (),
				    convert (right.getType ().getTree (), left.getTree ())
				), right
	    );
	}
    }

    Tree Tree::binaryPtr (const lexing::Word & loc, tree_code code, const Tree & type, const Tree & left, const Tree & right) {
	return Tree::build (code, loc, type, left,
			    Tree::init (
				loc.getLocation (),
				convert_to_ptrofftype (right.getTree ())
			    )
	);
    }

    Tree Tree::binaryPtrTest (const lexing::Word & loc, tree_code code, const Tree & type, const Tree & left, const Tree & right) {
	auto ulong_type = Tree::intType (64, false);

	return Tree::build (code, loc, type,
			    Tree::init (
				loc.getLocation (),
				convert (ulong_type.getTree (), left.getTree ())
			    ),
			    Tree::init (
				loc.getLocation (),
				convert (ulong_type.getTree (), right.getTree ())
			    )
	);
    }

    
    
    Tree Tree::binaryDirect (const lexing::Word & loc, tree_code code, const Tree & type, const Tree & left, const Tree & right) {
	return Tree::build (code, loc, type, left, right);
    }

    Tree Tree::castTo (const lexing::Word & loc, const Tree & type, const Tree & value) {
	return Tree::init (loc.getLocation (), convert (type.getTree (), value.getTree ()));
    }
    
    Tree Tree::unary (const lexing::Word & loc, tree_code code, const Tree & type, const Tree & operand) {
	return Tree::build (code, loc, type, operand);
    }
    
    
    Tree Tree::compound (const lexing::Word & loc, const Tree & left, const Tree & right) {
	if (left.isEmpty ()) return right;
	if (right.isEmpty ()) return left;
	return Tree::init (loc.getLocation (),
			   fold_build2_loc (loc.getLocation (), COMPOUND_EXPR,
					    left.getType ().getTree (), right.getTree (),
					    left.getTree ())
	);
    }

    
    Tree Tree::constructIndexed (const lexing::Word & loc, const Tree & type, const std::vector <Tree> & params) {
	vec <constructor_elt, va_gc> * elms = NULL;
	for (auto it : Ymir::r (0, params.size ())) {
	    CONSTRUCTOR_APPEND_ELT (elms, size_int (it), params [it].getTree ());
	}

	return Tree::init (loc.getLocation (), build_constructor (type.getTree (), elms));
    }

    Tree Tree::constructIndexed0 (const lexing::Word & loc, const Tree & type, const Tree & value, uint size) {
	vec <constructor_elt, va_gc> * elms = NULL;
	for (auto it : Ymir::r (0, size)) {
	    CONSTRUCTOR_APPEND_ELT (elms, size_int (it), value.getTree ());
	}

	return Tree::init (loc.getLocation (), build_constructor (type.getTree (), elms));
    }

    Tree Tree::constructField (const lexing::Word & loc, const Tree & type, const std::vector <std::string> & names, const std::vector <Tree> & values) {
	vec <constructor_elt, va_gc> * elms = NULL;
	for (auto it : Ymir::r (0, values.size ())) {
	    if (it >= (int) names.size ()) {
		Ymir::OutBuffer buf;
		buf.write ("_", (int) it - names.size ());
		CONSTRUCTOR_APPEND_ELT (elms, type.getField (buf.str ()).getTree (), values [it].getTree ());
	    } else 
		CONSTRUCTOR_APPEND_ELT (elms, type.getField (names [it]).getTree (), values [it].getTree ());
	}
	
	return Tree::init (loc.getLocation (), build_constructor (type.getTree (), elms));
    }
    
    Tree Tree::conditional (const lexing::Word & loc, const Tree & context, const Tree & test, const Tree & left, const Tree & right) {
	Tree thenLabel = Tree::makeLabel (loc, context, "then");
	Tree endLabel  = Tree::makeLabel (loc, context, "end_if");
	Tree gotoThen  = Tree::gotoExpr  (loc, thenLabel);
	Tree gotoEnd   = Tree::gotoExpr  (loc, endLabel);

	Tree gotoElse = Tree::empty (), elseLabel = Tree::empty ();
	if (!right.isEmpty ()) {
	    elseLabel = Tree::makeLabel (loc, context, "else");
	    gotoElse = Tree::gotoExpr (loc, elseLabel);
	} else {
	    gotoElse = gotoEnd;
	}

	TreeStmtList list = TreeStmtList::init ();
	list.append (Tree::condExpr (loc, test, gotoThen, gotoElse));
	list.append (Tree::labelExpr (loc, thenLabel));
	list.append (left);
	list.append (gotoEnd);

	if (!right.isEmpty ()) {
	    list.append (Tree::labelExpr (loc, elseLabel));
	    list.append (right);
	    list.append (gotoEnd);
	}
	
	list.append (Tree::labelExpr (loc, endLabel));
	return list.toTree ();
    }    

    Tree Tree::makeLabel (const lexing::Word & loc, const Tree & context, const std::string & name) {
	auto decl = Tree::init (loc.getLocation (), build_decl (loc.getLocation (), LABEL_DECL, get_identifier (name.c_str ()), void_type_node));
	decl.setDeclContext (context);
	return decl;
    }

    Tree Tree::gotoExpr (const lexing::Word & loc, const Tree & label) {
	return Tree::build (
	    GOTO_EXPR, loc, Tree::voidType (), label 	
	);
    }

    Tree Tree::labelExpr (const lexing::Word & loc, const Tree & label) {
	return Tree::build (
	    LABEL_EXPR, loc, Tree::voidType (), label	
	);			   
    }    

    Tree Tree::condExpr (const lexing::Word & loc, const Tree & test, const Tree & gotoS, const Tree & gotoF) {
	return Tree::build (
	    COND_EXPR, loc, Tree::voidType (), test, gotoS, gotoF	    
	);
    }

    Tree Tree::buildPtrCst (const lexing::Word & loc, ulong value) {
	auto cst = Tree::buildIntCst (loc, value, Tree::intType (64, false)).getTree ();
	return Tree::init (
	    loc.getLocation (), 
	    convert_to_ptrofftype (cst)
	);
    }
    
    Tree Tree::buildIntCst (const lexing::Word & loc, ulong value, const Tree & type) {
	return Tree::init (loc.getLocation (), build_int_cst_type (type.getTree (), value));
    }
    
    Tree Tree::buildIntCst (const lexing::Word & loc, long value, const Tree & type) {
	return Tree::init (loc.getLocation (), build_int_cst_type (type.getTree (), value));
    }

    Tree Tree::buildStringLiteral (const lexing::Word & loc, const char * lit, ulong len, int size) {
	return Tree::init (loc.getLocation (), build_string_literal (len * (size / 8), lit));
    }
    
    Tree Tree::buildSizeCst (ulong value) {
	return Tree::init (UNKNOWN_LOCATION, build_int_cst_type (long_unsigned_type_node, value));
    }
    
    Tree Tree::buildFloatCst (const lexing::Word & loc, const std::string & value, const Tree & type) {
	REAL_VALUE_TYPE real_value;
	auto str = value;
	if (str == "INF") str = "Inf";
	if (str == "NAN") str = "QNaN";
	real_from_string (&real_value, str.c_str ());
	return Tree::init (loc.getLocation (), build_real (type.getTree (), real_value));
    }

    Tree Tree::buildFloatCst (const lexing::Word & loc, float val, const Tree & type) {
	Ymir::OutBuffer buf;
	buf.write (val);
	return buildFloatCst (loc, buf.str (), type);
    }
    
    Tree Tree::buildFloatCst (const lexing::Word & loc, double val, const Tree & type) {
	Ymir::OutBuffer buf;
	buf.write (val);
	return buildFloatCst (loc, buf.str (), type);
    }

    Tree Tree::buildBoolCst (const lexing::Word & loc, bool value) {
	return Tree::init (loc.getLocation (), build_int_cst_type (unsigned_char_type_node, (ulong) value));
    }
    
    Tree Tree::buildCharCst (const lexing::Word & loc, uint value, const Tree & type) {
	return Tree::init (loc.getLocation (), build_int_cst_type (type.getTree (), value));
    }

    Tree Tree::buildAddress (const lexing::Word & loc, const Tree & value, const Tree & type) {
	auto aux_value = value;
	aux_value.isAddressable (true);
	return Tree::build (ADDR_EXPR, loc, type, aux_value);
    }
    
    Tree Tree::returnStmt (const lexing::Word & loc, const Tree & result_decl, const Tree & value) {
	auto inside = Tree::build (MODIFY_EXPR, loc, Tree::voidType (), result_decl, value); 
	return Tree::build (RETURN_EXPR, loc, Tree::voidType (), inside);	
    }

    Tree Tree::returnStmt (const lexing::Word & loc) {
	return Tree::build (RETURN_EXPR, loc, Tree::voidType (), Tree::empty ());	
    }
    
    Tree Tree::build (tree_code tc, const lexing::Word & loc, const Tree & type, const Tree & t1) {
	return Tree::init (loc.getLocation (), build1_loc (loc.getLocation (), tc, type.getTree (), t1.getTree ()));
    }
    
    Tree Tree::build (tree_code tc, const lexing::Word & loc, const Tree & type, const Tree & t1, const Tree & t2) {
	return Tree::init (loc.getLocation (), build2_loc (loc.getLocation (), tc, type.getTree (), t1.getTree (), t2.getTree ()));
    }

    Tree Tree::build (tree_code tc, const lexing::Word & loc, const Tree & type, const Tree & t1, const Tree & t2, const Tree & t3) {
	return Tree::init (loc.getLocation (), build3_loc (loc.getLocation (), tc, type.getTree (), t1.getTree (), t2.getTree (), t3.getTree ()));
    }

    Tree Tree::build (tree_code tc, const lexing::Word & loc, const Tree & type, const Tree & t1, const Tree & t2, const Tree & t3, const Tree & t4) {
	return Tree::init (loc.getLocation (), build4_loc (loc.getLocation (), tc, type.getTree (), t1.getTree (), t2.getTree (), t3.getTree (), t4.getTree ()));
    }

    Tree Tree::build (tree_code tc, lexing::Word loc, const Tree & type, const Tree & t1, const Tree & t2, const Tree & t3, const Tree & t4, const Tree & t5) {
	return Tree::init (loc.getLocation (), build5_loc (loc.getLocation (), tc, type.getTree (), t1.getTree (), t2.getTree (), t3.getTree (), t4.getTree (), t5.getTree ()));
    }
    
    const location_t & Tree::getLocus () const {
	return this-> _loc;
    }

    void Tree::setLocus (const location_t & loc) {
	this-> _loc = loc;
    }

    Tree Tree::promote () const {
	tree type = getType ().getTree ();
	enum tree_code code = TREE_CODE (type);
	switch (code) {
	case BOOLEAN_TYPE : return Tree::init (this-> _loc, convert (integer_type_node, getTree ()));
	case ENUMERAL_TYPE : return Tree::init (this-> _loc, convert (integer_type_node, getTree ()));
	case INTEGER_TYPE : {
	    if (type == signed_char_type_node ||
		type == short_integer_type_node)
		return Tree::init (this-> _loc, convert (integer_type_node, getTree ()));
	    else if (type == unsigned_char_type_node ||
		     type == short_unsigned_type_node)
		return Tree::init (this-> _loc, convert (unsigned_type_node, getTree ()));
	    else return *this;
	}
	case REAL_TYPE : return Tree::init (this-> _loc, convert (double_type_node, getTree ()));
	default : return *this;
	}

    }    
    
    const tree & Tree::getTree () const {
	return this-> _t;
    }

    void Tree::setTree (const tree & t) {
	this-> _t = t;
    }

    tree_code Tree::getTreeCode () const {
	return TREE_CODE (this-> _t);
    }

    bool Tree::isError () const {
	return error_operand_p (this-> _t);
    }

    bool Tree::isEmpty () const {
	return this-> _t == NULL_TREE;
    }

    bool Tree::isStatic () const {
	return TREE_STATIC (this-> _t) == 1;
    }
    
    void Tree::isStatic (bool st) {
	TREE_STATIC (this-> _t) = st;
    }

    void Tree::isGlobalCstr (bool is) {
	DECL_STATIC_CONSTRUCTOR (this-> _t) = is;
    }
    
    bool Tree::isUsed () const {
	return TREE_USED (this-> _t) == 1;
    }
    
    void Tree::isUsed (bool us) {
	TREE_USED (this-> _t) = us;
    }

    bool Tree::isExternal () const {
	return DECL_EXTERNAL (this-> _t) == 1;
    }
    
    void Tree::isExternal (bool ext) {
	DECL_EXTERNAL (this-> _t) = ext;
    }

    bool Tree::isPreserved () const {
	return DECL_PRESERVE_P (this-> _t) == 1;
    }
    
    void Tree::isPreserved (bool pre) {
	DECL_PRESERVE_P (this-> _t) = pre;
    }

    bool Tree::isAddressable () const {
	return TREE_ADDRESSABLE (this-> _t) == 1;
    }

    void Tree::isAddressable (bool is) {
	TREE_ADDRESSABLE (this-> _t) = is;
    }

    void Tree::asmName (const std::string & name) {
	SET_DECL_ASSEMBLER_NAME (this-> _t, get_identifier (name.c_str ()));
    }

    std::string Tree::funcDeclName () const {
	return std::string (IDENTIFIER_POINTER( DECL_NAME (this-> _t)));	    
    }
    
    bool Tree::isPublic () const {
	return TREE_PUBLIC (this-> _t) == 1;
    }
    
    void Tree::isPublic (bool b) {
	TREE_PUBLIC (this-> _t) = b;
    }

    Tree Tree::getDeclContext () const {
	return Tree::init (
	    UNKNOWN_LOCATION,
	    DECL_CONTEXT (this-> _t)
	);			   
    }
    
    void Tree::setDeclContext (const Tree & context) {
	DECL_CONTEXT (this-> _t) = context.getTree ();
    }

    Tree Tree::getDeclInitial () const {
	return Tree::init (
	    UNKNOWN_LOCATION,
	    DECL_INITIAL (this-> _t)
	);			   	
    }

    void Tree::setDeclInitial (const Tree & init) {
	DECL_INITIAL (this-> _t) = init.getTree ();
    }

    Tree Tree::getDeclSavedTree () const {
	return Tree::init (
	    UNKNOWN_LOCATION,
	    DECL_SAVED_TREE (this-> _t)
	);			   	
    }

    void Tree::setDeclSavedTree (const Tree & saved) {
	DECL_SAVED_TREE (this-> _t) = saved.getTree ();
    }

    bool Tree::isWeak () const {
	return DECL_WEAK (this-> _t) == 1;
    }

    void Tree::isWeak (bool is) {
	DECL_WEAK (this-> _t) = is;
    }

    bool Tree::isScalar () const {
	if (this-> _t == NULL_TREE)
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	
	return getTreeCode () != RECORD_TYPE &&
	    getTreeCode () != ARRAY_TYPE && getTreeCode () != UNION_TYPE;	    
    }    
    
    void Tree::setDeclArguments (const std::list <Tree> & args) {
	tree arglist = NULL_TREE; 
	for (auto & it : args)
	    arglist = chainon (arglist, it.getTree ());
	DECL_ARGUMENTS (this-> _t) = arglist;
    }

    void Tree::setArgType (const Tree & type) {
	DECL_ARG_TYPE (this-> _t) = type.getTree ();
    }

    void Tree::setResultDecl (const Tree & result) {
	DECL_RESULT (this-> _t) = result.getTree ();
    }

    Tree Tree::getResultDecl () const {
	return Tree::init (this-> _loc, DECL_RESULT (this-> _t));
    }

    void Tree::setBlockSuperContext (const Tree & tree) {
	BLOCK_SUPERCONTEXT (this-> _t) = tree.getTree ();
    }
    
    Tree Tree::getType () const {
	if (this-> _t == NULL_TREE)
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	return Tree::init (this-> _loc, TREE_TYPE (this-> _t));
    }

    uint Tree::getSize () const {
	if (this-> _t == NULL_TREE)
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	return TREE_INT_CST_LOW (TYPE_SIZE_UNIT (this-> _t));
    }    

    Tree Tree::getArraySize () const {
	auto range = TYPE_DOMAIN (this-> _t);
	return Tree::init (BUILTINS_LOCATION,
			   convert (
			       long_unsigned_type_node,
			       Tree::build (PLUS_EXPR, lexing::Word::eof (),
					    Tree::init (BUILTINS_LOCATION, integer_type_node),
					    Tree::init (BUILTINS_LOCATION, TYPE_MAX_VALUE (range)),
					    Tree::init (BUILTINS_LOCATION, build_int_cst_type (integer_type_node, 1))
			       ).getTree ()
			   )
	);
	
    }

    Tree Tree::getStringSize (int inner) const {	
	auto t = getOperand (0).getOperand (0);
	return Tree::init (BUILTINS_LOCATION,
			   convert (
			       long_unsigned_type_node,
			       Tree::init (BUILTINS_LOCATION,
					   build_int_cst_type (
					       long_unsigned_type_node,
					       (TREE_STRING_LENGTH (t.getTree ()) / (inner / 8)) - 1
					   )
			       ).getTree ()
			   ));   	
    }
    
    bool Tree::isStringType () const {
	return this-> getTreeCode () == POINTER_TYPE
	    && TYPE_MAIN_VARIANT (TREE_TYPE (this-> getTree ())) == char_type_node;       
    }
    
    bool Tree::isArrayType () const {
	return this-> getTreeCode () == ARRAY_TYPE;
    }
    
    Tree Tree::getOperand (int i) const {
	if (this-> _t == NULL_TREE)
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	return Tree::init (this-> _loc, TREE_OPERAND (this-> _t, i));
    }

    Tree Tree::getList () const {
	if (this-> _t == NULL_TREE)
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	if (this-> getTreeCode () == COMPOUND_EXPR) {
	    return this-> getOperand (0);
	} else return Tree::empty ();
    }

    Tree Tree::getValue () const {
	if (this-> _t == NULL_TREE)
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	if (this-> getTreeCode () == COMPOUND_EXPR) {
	    return this-> getOperand (1);
	} else {
	    return *this;
	}
    }

    Tree Tree::toDirect () const {
	if (this-> _t == NULL_TREE)
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	if (this-> getType ().isPointerType ()) {
	    return this-> buildPointerUnref (0);
	} else
	    return *this;
    }

    bool Tree::isPointerType () const {
	return this-> getTreeCode () == POINTER_TYPE;
    }

    bool Tree::isVoidType () const {
	return this-> getTreeCode () == VOID_TYPE;
    }
    
    Tree Tree::buildPointerUnref (int index) const {
	auto inner = TREE_TYPE (this-> getType ().getTree ());
	auto element_size = TYPE_SIZE_UNIT (inner);
	tree it = build_int_cst_type (long_unsigned_type_node, index);
	it = fold_convert_loc (this-> _loc, size_type_node, it);
	auto offset = fold_build2_loc (this-> _loc, MULT_EXPR, size_type_node, it, element_size);

	it = convert_to_ptrofftype (offset);
	tree addr = build2 (POINTER_PLUS_EXPR, this-> getType ().getTree (), this-> _t, it);
	return Tree::init (this-> _loc, build2 (MEM_REF, inner, addr, build_int_cst (this-> getType ().getTree (), 0)));
    }


    Tree Tree::buildPointerUnref (Tree type, int index) const {
	auto ptrType = pointerType (type);
	auto inner = type.getTree ();
	auto element_size = TYPE_SIZE_UNIT (inner);
	tree it = build_int_cst_type (long_unsigned_type_node, index);
	it = fold_convert_loc (this-> _loc, size_type_node, it);
	auto offset = fold_build2_loc (this-> _loc, MULT_EXPR, size_type_node, it, element_size);

	it = convert_to_ptrofftype (offset);
	tree addr = build2 (POINTER_PLUS_EXPR, ptrType.getTree (), this-> _t, it);
	return Tree::init (this-> _loc, build2 (MEM_REF, inner, addr, build_int_cst (ptrType.getTree (), 0)));
    }
    
    Tree Tree::getField (const std::string & name) const {
	if (this-> _t == NULL_TREE)
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	tree type = this-> _t;
	if (this-> getTreeCode () != RECORD_TYPE && this-> getTreeCode () != UNION_TYPE) type = TREE_TYPE (this-> _t);
	
	tree field_decl = TYPE_FIELDS (type);
	while (field_decl != NULL_TREE) {
	    tree decl_name = DECL_NAME (field_decl);
	    std::string field_name (IDENTIFIER_POINTER (decl_name));
	    if (field_name == name) break;
	    else field_decl = TREE_CHAIN (field_decl);
	}

	if (field_decl == NULL_TREE)
	    Ymir::Error::halt ("(%r) - undefined field : %(r)", "Critical" , name);

	// If this-> _t is not a RECORD_TYPE, we assume we want the component ref
	if (this-> getTreeCode () == RECORD_TYPE || this-> getTreeCode () == UNION_TYPE)
	    return Tree::init (BUILTINS_LOCATION, field_decl);
	
	return Tree::build (
	    COMPONENT_REF,
	    lexing::Word::eof (),
	    Tree::init (UNKNOWN_LOCATION, TREE_TYPE (field_decl)),
	    *this,
	    Tree::init(UNKNOWN_LOCATION, field_decl),
	    Tree::empty ()
	);    
    }

    void Tree::print () const {
	debug_tree (this-> getTree ());
    }

    void Tree::gimplifyFunction (const Tree & fn_decl) {
	gimplify_function_tree (fn_decl.getTree ());
    }

    void Tree::finalizeFunction (const Tree & fn_decl) {
	cgraph_node::finalize_function (fn_decl.getTree (), true);
    }
    
}

/**
 * Mandatory Implementation from GCC internals
 */
tree convert (tree type, tree expr) {
    tree e = expr;
    enum tree_code code = TREE_CODE (type);
    const char *invalid_conv_diag;
    tree ret;
    location_t loc = EXPR_LOCATION (expr);

    if (type == error_mark_node
	|| error_operand_p (expr))
	return error_mark_node;

    if ((invalid_conv_diag
	 = targetm.invalid_conversion (TREE_TYPE (expr), type)))
	{
	    error ("%s", invalid_conv_diag);
	    return error_mark_node;
	}

    if (type == TREE_TYPE (expr))
	return expr;
    ret = targetm.convert_to_type (type, expr);
    if (ret)
	return ret;

    STRIP_TYPE_NOPS (e);

    if (TYPE_MAIN_VARIANT (type) == TYPE_MAIN_VARIANT (TREE_TYPE (expr))
	&& (TREE_CODE (TREE_TYPE (expr)) != COMPLEX_TYPE
	    || TREE_CODE (e) == COMPLEX_EXPR))
	return fold_convert_loc (loc, type, expr);
    if (TREE_CODE (TREE_TYPE (expr)) == ERROR_MARK)
	return error_mark_node;
    if (TREE_CODE (TREE_TYPE (expr)) == VOID_TYPE)
	{
	    error ("void value not ignored as it ought to be");
	    return error_mark_node;
	}

    switch (code)
	{
	case VOID_TYPE:
	    return fold_convert_loc (loc, type, e);

	case INTEGER_TYPE:
	case ENUMERAL_TYPE:
	    if (sanitize_flags_p (SANITIZE_FLOAT_CAST)
		&& current_function_decl != NULL_TREE
		&& TREE_CODE (TREE_TYPE (expr)) == REAL_TYPE
		&& COMPLETE_TYPE_P (type))
		{
		    expr = save_expr (expr);
		    tree check = ubsan_instrument_float_cast (loc, type, expr);
		    expr = fold_build1 (FIX_TRUNC_EXPR, type, expr);
		    if (check == NULL_TREE)
			return expr;
		    return fold_build2 (COMPOUND_EXPR, TREE_TYPE (expr), check, expr);
		}
	    ret = convert_to_integer (type, e);
	    goto maybe_fold;

	case BOOLEAN_TYPE:
	    return fold_convert_loc
		(loc, type, expr);

	case POINTER_TYPE:
	case REFERENCE_TYPE:
	    ret = convert_to_pointer (type, e);
	    goto maybe_fold;

	case REAL_TYPE:
	    ret = convert_to_real (type, e);
	    goto maybe_fold;

	case FIXED_POINT_TYPE:
	    ret = convert_to_fixed (type, e);
	    goto maybe_fold;

	case COMPLEX_TYPE:
	    ret = convert_to_complex (type, e);
	    goto maybe_fold;

	case VECTOR_TYPE:
	    ret = convert_to_vector (type, e);
	    goto maybe_fold;

	case RECORD_TYPE:
	case UNION_TYPE:
	    if (lang_hooks.types_compatible_p (type, TREE_TYPE (expr)))
		return e;
	    break;

	default:
	    break;

	maybe_fold:
	    if (TREE_CODE (ret) != C_MAYBE_CONST_EXPR)
		ret = fold (ret);
	    return ret;
	}

    error ("conversion to non-scalar type requested");
    return error_mark_node;
}

