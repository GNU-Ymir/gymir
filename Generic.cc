#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/types/InfoType.hh>
#include <ymir/errors/Error.hh>
#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/semantic/pack/Symbol.hh>
#include <ymir/semantic/value/Value.hh>
#include <ymir/ast/Expression.hh>
#include "toplev.h"

using namespace semantic;

static GTY(()) vec<tree, va_gc> *global_declarations;

namespace Ymir {
    
    Tree makeField (InfoType type, std::string name) {
	Tree t_decl = type-> toGeneric ();
	Tree field_decl = build_decl (BUILTINS_LOCATION, FIELD_DECL,
				      get_identifier (name.c_str ()),
				      t_decl.getTree ()
	);
	
	TREE_ADDRESSABLE (field_decl.getTree ()) = 1;
	
	return field_decl;
    }
    
    Tree makeStructType (std::string name, int nbfields, ...) {
	tree fields_last = NULL_TREE, fields_begin = NULL_TREE;
	va_list ap;
	tree record_type = make_node (RECORD_TYPE);
	
	va_start (ap, nbfields);
	for (int i = 0 ; i< nbfields ; i++) {
	    tree ident = va_arg (ap, tree);
	    tree type = va_arg (ap, tree);
	    tree field = build_decl (BUILTINS_LOCATION, FIELD_DECL, ident, type);
	    DECL_CONTEXT (field) = record_type;

	    if (fields_begin == NULL) {
		fields_begin = field;
	    }
	    if (fields_last != NULL) {
		TREE_CHAIN (fields_last) = field;
	    }

	    fields_last = field;
	}
	
	va_end (ap);

	TYPE_NAME (record_type) = get_identifier (name.c_str ());
	TREE_CHAIN (fields_last) = NULL_TREE;
	
	TYPE_FIELDS (record_type) = fields_begin;
	//finish_builtin_struct (type, name.c_str (), fields, NULL_TREE);
	layout_type (record_type);

	return record_type;
    }

    Tree makeTuple (std::string name, const std::vector <InfoType>& types, const std::vector<std::string> & attrs) {
	tree fields_last = NULL_TREE, fields_begin = NULL_TREE;
	tree record_type = make_node (RECORD_TYPE);
	for (uint i = 0 ; i < types.size () ; i++) {
	    tree ident = get_identifier (attrs [i].c_str ());
	    tree type = types [i]-> toGeneric ().getTree ();
	    tree field = build_decl (BUILTINS_LOCATION, FIELD_DECL, ident, type);
	    DECL_CONTEXT (field) = record_type;

	    if (fields_begin == NULL) fields_begin = field;
	    if (fields_last != NULL) TREE_CHAIN (fields_last) = field;
	    fields_last = field;
	}

	TYPE_NAME (record_type) = get_identifier (name.c_str ());
	TREE_CHAIN (fields_last) = NULL_TREE;
	TYPE_FIELDS (record_type) = fields_begin;
	layout_type (record_type);
	return record_type;
    }

    Tree makeTuple (std::string name, const std::vector<InfoType> & types) {
	tree fields_last = NULL_TREE, fields_begin = NULL_TREE;
	tree record_type = make_node (RECORD_TYPE);
	
	for (uint i = 0 ; i < types.size () ; i++) {
	    OutBuffer buf;
	    buf.write ("_", (int) i);
	    tree ident = get_identifier (buf.str ().c_str ());
	    tree type = types [i]-> toGeneric ().getTree ();
	    tree field = build_decl (BUILTINS_LOCATION, FIELD_DECL, ident, type);
	    DECL_CONTEXT (field) = record_type;

	    if (fields_begin == NULL) fields_begin = field;
	    if (fields_last != NULL) TREE_CHAIN (fields_last) = field;
	    fields_last = field;
	}
	
	TYPE_NAME (record_type) = get_identifier (name.c_str ());
	TREE_CHAIN (fields_last) = NULL_TREE;
	TYPE_FIELDS (record_type) = fields_begin;
	layout_type (record_type);
	return record_type;
    }

    
    Tree getField (location_t loc, Tree obj, std::string name) {
	Tree field_decl = TYPE_FIELDS (TREE_TYPE (obj.getTree ()));

	while (!field_decl.isNull ()) {
	    Tree decl_name = DECL_NAME (field_decl.getTree ());
	    std::string field_name (IDENTIFIER_POINTER (decl_name.getTree ()));

	    if (field_name == name) break;
	    else 
		field_decl = TREE_CHAIN (field_decl.getTree ());
	}

	if (field_decl.isNull ())
	    Ymir::Error::assert ("");
	
	return Ymir::buildTree (COMPONENT_REF, loc,
				TREE_TYPE (field_decl.getTree ()),
				obj,
				field_decl,
				Tree ()
	);
    }


    Tree getField (location_t loc, Tree obj, ulong it) {
	Tree field_decl = TYPE_FIELDS (TREE_TYPE (obj.getTree ()));

	for (auto it __attribute__((unused)) : Ymir::r (0, it)) {
	    Tree decl_name = DECL_NAME (field_decl.getTree ());
	    std::string field_name (IDENTIFIER_POINTER (decl_name.getTree ()));	   
	    field_decl = TREE_CHAIN (field_decl.getTree ());
	}

	if (field_decl.isNull ())
	    Ymir::Error::assert ("");
	
	return Ymir::buildTree (COMPONENT_REF, loc,
				TREE_TYPE (field_decl.getTree ()),
				obj,
				field_decl,
				Tree ()
	);

    }
    
    Ymir::Tree makeAuxVar (location_t locus, ulong id, Ymir::Tree type) {
	OutBuffer buf ;
	buf.write ("_", id, "_");
	Ymir::Tree decl = build_decl (
	    locus,
	    VAR_DECL,	    
	    get_identifier (buf.str ().c_str ()),
	    type.getTree ()
	);

	DECL_CONTEXT (decl.getTree ()) = IFinalFrame::currentFrame ().getTree ();
	Ymir::getStackVarDeclChain ().back ().append (decl);
	Ymir::getStackStmtList ().back ().append (buildTree (DECL_EXPR, locus, void_type_node, decl));
	return decl;
							     
    }

    Tree getArrayRef (location_t locus, Tree array, Tree inner, ulong index) {	
	Tree it = build_int_cst_type (long_unsigned_type_node, index);	
	return buildTree (ARRAY_REF, locus, inner, array, it, Tree (), Tree ());
    }

    Tree getPointerUnref (location_t loc, Tree ptr, Tree inner, ulong index) {
	if (index != 0) {
	    Tree it = build_int_cst_type (long_unsigned_type_node, index);
	    return getPointerUnref (loc, ptr, inner, it);
	} else {
	    auto ptype = build_pointer_type (inner.getTree ());
	    return build2 (MEM_REF, inner.getTree (), ptr.getTree (), build_int_cst (ptype, 0));
	}
    }

    Tree getArrayRef (location_t locus, Tree array, Tree inner, Tree index) {
	return buildTree (ARRAY_REF, locus, inner, array, index, Tree (), Tree ());
    }

    Tree getPointerUnref (location_t loc, Tree ptr, Tree inner, Tree index) {
	tree ptype = build_pointer_type (inner.getTree ());
	tree element_size = TYPE_SIZE_UNIT (inner.getTree ());
	index = fold_convert_loc (loc, size_type_node, index.getTree ());
	tree offset = fold_build2_loc (loc, MULT_EXPR, size_type_node, index.getTree (), element_size);
	
	tree it = convert_to_ptrofftype (offset);
	tree addr = build2 (POINTER_PLUS_EXPR, TREE_TYPE (ptr.getTree ()), ptr.getTree (), it);
	return build2 (MEM_REF, inner.getTree (), addr, build_int_cst (ptype, 0));
    }
    
    Tree getAddr (Tree elem) {
	TREE_ADDRESSABLE (elem.getTree ()) = 1;
	return buildTree (ADDR_EXPR, UNKNOWN_LOCATION, build_pointer_type (elem.getType ().getTree ()), elem);
    }

    Tree getAddr (location_t loc, Tree elem) {
	TREE_ADDRESSABLE (elem.getTree ()) = 1;
	return buildTree (ADDR_EXPR, loc, build_pointer_type (elem.getType ().getTree ()), elem);
    }
   
    Tree makeLabel (location_t loc, std::string & name) {
	tree decl = build_decl (loc, LABEL_DECL, get_identifier (name.c_str ()), void_type_node);
	DECL_CONTEXT (decl) = IFinalFrame::currentFrame ().getTree ();
	return decl;
    }

    Tree makeLabel (location_t loc, const char* name) {
	tree decl = build_decl (loc, LABEL_DECL, get_identifier (name), void_type_node);
	DECL_CONTEXT (decl) = IFinalFrame::currentFrame ().getTree ();
	return decl;
    }

    Tree getGlobalContext () {
	static Tree global_context;
	if (global_context.isNull ()) {
	    global_context = build_translation_unit_decl (NULL_TREE);
	}
	return global_context;
    }

    void push_decl (Tree decl_) {
	auto decl = decl_.getTree ();
	if (!DECL_CONTEXT (decl)) {
	    if (!Ymir::currentContext ().isNull ()) {
		DECL_CONTEXT (decl) = Ymir::currentContext ().getTree ();
	    } else {
		DECL_CONTEXT (decl) = getGlobalContext ().getTree ();
	    }
	}

	if (TREE_STATIC (decl)) {
	    vec_safe_push (global_declarations, decl);
	} else {
	    Ymir::getStackVarDeclChain ().back ().append (decl);
	}
    }
    
    void declareGlobal (Symbol sym) {
	auto type_tree = sym-> type-> toGeneric ();
	tree decl = build_decl (
	    sym-> sym.getLocus (),
	    VAR_DECL,
	    get_identifier (sym-> sym.getStr ().c_str ()),
	    type_tree.getTree ()
	);

	TREE_STATIC (decl) = 1;
	TREE_USED (decl) = 1;
	DECL_EXTERNAL (decl) = 0;
	DECL_PRESERVE_P (decl) = 1;
	TREE_PUBLIC (decl) = 1;
	if (sym-> value ()) {
	    DECL_INITIAL (decl) = sym-> value ()-> toYmir (sym)-> toGeneric ().getTree ();
	} else {
	    DECL_INITIAL (decl) = error_mark_node;
	}
	
	push_decl (decl);
	sym-> treeDecl (decl);
    }

    void declareGlobalExtern (Symbol sym) {
	auto type_tree = sym-> type-> toGeneric ();
	tree decl = build_decl (
	    sym-> sym.getLocus (),
	    VAR_DECL,
	    get_identifier (sym-> sym.getStr ().c_str ()),
	    type_tree.getTree ()
	);

	TREE_STATIC (decl) = 1;
	//TREE_USED (decl) = 1;
	//DECL_EXTERNAL (decl) = 1;
	//TREE_PUBLIC (decl) = 1;
	sym-> treeDecl (decl);
    }
    
    void finishCompilation () {
	int len = vec_safe_length (global_declarations);
	tree * addr = vec_safe_address (global_declarations);
	for (int i = 0 ; i < len ; i++) {
	    tree decl = addr [i];
	    wrapup_global_declarations (&decl, 1);
	}
    }
    
}

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

