#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/types/InfoType.hh>
#include <ymir/errors/Error.hh>
#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/semantic/pack/Symbol.hh>
#include <ymir/semantic/value/Value.hh>
#include <ymir/ast/Expression.hh>
#include <ymir/utils/Mangler.hh>
#include <ymir/semantic/types/RefInfo.hh>
#include <ymir/semantic/types/PtrInfo.hh>
#include <ymir/semantic/object/AggregateInfo.hh>
#include "toplev.h"

using namespace semantic;

static GTY(()) vec<tree, va_gc> *global_declarations;


namespace Ymir {
    
    std::map <std::string, Tree> __vtable__;
    
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

    Tree makeUnion (std::string name, const std::vector <InfoType>& types, const std::vector<std::string> & attrs) {
	tree fields_last = NULL_TREE, fields_begin = NULL_TREE;
	tree record_type = make_node (UNION_TYPE);
	for (uint i = 0 ; i < types.size () ; i++) {
	    tree ident;
	    if (i >= attrs.size ()) {
		OutBuffer buf;
		buf.write ("_", (int) i - attrs.size ());
		ident = get_identifier (buf.str ().c_str ());
	    } else {
		ident = get_identifier (attrs [i].c_str ());
	    }
	    
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
    
    Tree makeTuple (std::string name, const std::vector <InfoType>& types, const std::vector<std::string> & attrs, bool packed) {
	tree fields_last = NULL_TREE, fields_begin = NULL_TREE;
	tree record_type = make_node (RECORD_TYPE);
	auto size = 0;
	for (uint i = 0 ; i < types.size () ; i++) {
	    tree ident;
	    if (i >= attrs.size ()) {
		OutBuffer buf;
		buf.write ("_", (int) i - attrs.size ());
		ident = get_identifier (buf.str ().c_str ());
	    } else {
		ident = get_identifier (attrs [i].c_str ());
	    }
	    
	    tree type = types [i]-> toGeneric ().getTree (); 
	    
	    size += TREE_INT_CST_LOW (TYPE_SIZE_UNIT (type));
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
	
	if (packed) {
	    TYPE_SIZE (record_type) = bitsize_int (size * BITS_PER_UNIT);
	    TYPE_SIZE_UNIT (record_type) = size_int (size);
	    TYPE_PACKED (record_type) = 1;
	    SET_TYPE_ALIGN (record_type, 1 * BITS_PER_UNIT);
	    compute_record_mode (record_type);	
	}		
	
	return record_type;
    }

    Tree makeUnion (std::string name, const std::vector <InfoType>& types) {
	tree fields_last = NULL_TREE, fields_begin = NULL_TREE;
	tree record_type = make_node (UNION_TYPE);
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
	SET_TYPE_ALIGN (record_type, 1 * BITS_PER_UNIT);
	TYPE_PACKED (record_type) = 1;
	compute_record_mode (record_type);	
	layout_type (record_type);
	
	return record_type;
    }

    Tree getFieldDecl (Tree type, std::string name) {
	Tree field_decl = TYPE_FIELDS (type.getTree ());
	while (!field_decl.isNull ()) {
	    Tree decl_name = DECL_NAME (field_decl.getTree ());
	    std::string field_name (IDENTIFIER_POINTER (decl_name.getTree ()));
	    if (field_name == name) break;
	    else 
		field_decl = TREE_CHAIN (field_decl.getTree ());
	}
	if (field_decl.isNull ()) 
	    Ymir::Error::assert ((std::string ("undef attr ") + name).c_str ());
	
	return field_decl.getTree ();
    }

    std::vector <Tree> getFieldDecls (Tree type) {
	Tree field_decl = TYPE_FIELDS (type.getTree ());
	std::vector <Tree> decls;
	while (!field_decl.isNull ()) {
	    decls.push_back (field_decl.getTree ());
	    field_decl = TREE_CHAIN (field_decl.getTree ());
	}
	
	return decls;
    }
    
    
    Tree getField (location_t loc, Tree obj, std::string name) {
	if (obj.getType ().getTreeCode () == POINTER_TYPE) {
	    return getField (loc, getPointerUnref (loc, obj, TREE_TYPE (obj.getType ().getTree ()), 0), name);
	}
	
	Tree field_decl = TYPE_FIELDS (TREE_TYPE (obj.getTree ()));
	while (!field_decl.isNull ()) {
	    Tree decl_name = DECL_NAME (field_decl.getTree ());
	    std::string field_name (IDENTIFIER_POINTER (decl_name.getTree ()));
	    if (field_name == name) break;
	    else 
		field_decl = TREE_CHAIN (field_decl.getTree ());
	}

	if (field_decl.isNull ()) 
	    Ymir::Error::assert ((std::string ("undef attr ") + name).c_str ());
	
	return Ymir::buildTree (COMPONENT_REF, loc,
				TREE_TYPE (field_decl.getTree ()),
				obj,
				field_decl,
				Tree ()
	);
    }


    Tree getField (location_t loc, Tree obj, ulong it) {
	if (obj.getType ().getTreeCode () == POINTER_TYPE) {
	    return getField (loc, getPointerUnref (loc, obj, TREE_TYPE (obj.getType ().getTree ()), 0), it);
	}
	
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

    Ymir::Tree makeAuxVar (location_t locus, const std::string &name, Ymir::Tree type) {
	Ymir::Tree decl = build_decl (
	    locus,
	    VAR_DECL,	    
	    get_identifier (name.c_str ()),
	    type.getTree ()
	);

	DECL_CONTEXT (decl.getTree ()) = IFinalFrame::currentFrame ().getTree ();
	Ymir::getStackVarDeclChain ().back ().append (decl);
	return compoundExpr (locus,
			     buildTree (DECL_EXPR, locus, void_type_node, decl),
			     decl);
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
	return compoundExpr (locus,
			     buildTree (DECL_EXPR, locus, void_type_node, decl),
			     decl);
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
    
    void declareGlobal (Symbol sym, syntax::Expression value) {
	auto type_tree = sym-> type ()-> toGeneric ();
	auto name = Mangler::mangle_global (Namespace (sym-> space (), sym-> sym.getStr ()).toString ());
	tree decl = build_decl (
	    sym-> sym.getLocus (),
	    VAR_DECL,
	    get_identifier (name.c_str ()),
	    type_tree.getTree ()
	);

	TREE_STATIC (decl) = 1;
	TREE_USED (decl) = 1;
	DECL_EXTERNAL (decl) = 0;
	DECL_PRESERVE_P (decl) = 1;
	TREE_PUBLIC (decl) = 1;

	if (value) {
	    if (value-> info-> value ())
		DECL_INITIAL (decl) = value-> info-> value ()-> toYmir (value-> info)-> toGeneric ().getTree ();
	    else
		DECL_INITIAL (decl) = value-> toGeneric ().getTree ();
	} else {
	    auto type = sym-> type ();
	    DECL_INITIAL (decl) = type-> genericConstructor ().getTree ();
	}
	
	push_decl (decl);
	sym-> treeDecl (decl);	
    }
        
    Ymir::Tree declareVtable (const std::string & name, Tree type, Tree value) {	
	tree decl = build_decl (
	    UNKNOWN_LOCATION,
	    VAR_DECL,
	    get_identifier(name.c_str ()),
	    type.getTree ()
	);

	TREE_STATIC (decl) = 1;
	DECL_ARTIFICIAL (decl) = 1;
	TREE_READONLY (decl) = 1;
	SET_DECL_ALIGN (decl, TARGET_VTABLE_ENTRY_ALIGN);
	DECL_USER_ALIGN (decl) = true;
	DECL_EXTERNAL (decl) = 0;
	DECL_PRESERVE_P (decl) = 1;
	DECL_WEAK (decl) = 1;
	TREE_PUBLIC (decl) = 1;
	
	DECL_INITIAL (decl) = value.getTree ();	
	push_decl (decl);
	Ymir::__vtable__ [name] = decl;
	return decl;
    }

    Ymir::Tree declareVtableExtern (const std::string & name, Tree type) {	
	tree decl = build_decl (
	    UNKNOWN_LOCATION,
	    VAR_DECL,
	    get_identifier(name.c_str ()),
	    type.getTree ()
	);

	TREE_STATIC (decl) = 1;
	DECL_ARTIFICIAL (decl) = 1;
	TREE_READONLY (decl) = 1;
	SET_DECL_ALIGN (decl, TARGET_VTABLE_ENTRY_ALIGN);
	DECL_USER_ALIGN (decl) = true;
	DECL_EXTERNAL (decl) = 1;
	DECL_PRESERVE_P (decl) = 1;
	DECL_WEAK (decl) = 1;
	TREE_PUBLIC (decl) = 1;
	
	push_decl (decl);
	Ymir::__vtable__ [name] = decl;
	return decl;
    }

    void declareGlobalWeak (Symbol sym, syntax::Expression value) {
	declareGlobal (sym, value);
	DECL_WEAK (sym-> treeDecl ().getTree ()) = 1;
    }

    Ymir::Tree declareGlobalWeak (const std::string & name, Ymir::Tree type_tree, Ymir::Tree value) {
	static std::map <std::string, Tree> dones;
	
	if (dones.find (name) == dones.end ()) {
	    auto globName = Mangler::mangle_global (name);
	    tree decl = build_decl (
		BUILTINS_LOCATION,
		VAR_DECL,
		get_identifier (globName.c_str ()),
		type_tree.getTree ()
	    );

	    TREE_STATIC (decl) = 1;
	    TREE_USED (decl) = 1;
	    DECL_EXTERNAL (decl) = 0;
	    DECL_PRESERVE_P (decl) = 1;
	    TREE_PUBLIC (decl) = 1;
	    DECL_WEAK (decl) = 1;
	    DECL_INITIAL (decl) = value.getTree ();
	    push_decl (decl);
	    dones [name] = decl;
	    return decl;
	}
	
	return dones [name];
    }    
    
    Tree getVtable (const std::string & name) {
	auto it = Ymir::__vtable__.find (name);
	if (it == Ymir::__vtable__.end ())
	    return Ymir::Tree ();
	return it-> second;
    }
    
    void declareGlobalExtern (Symbol sym) {
	auto type_tree = sym-> type ()-> toGeneric ();
	auto name = Mangler::mangle_global (Namespace (sym-> space (), sym-> sym.getStr ()).toString ());
	tree decl = build_decl (
	    sym-> sym.getLocus (),
	    VAR_DECL,
	    get_identifier (name.c_str ()),
	    type_tree.getTree ()
	);

	TREE_STATIC (decl) = 1;
	//TREE_USED (decl) = 1;
	//DECL_EXTERNAL (decl) = 1;
	//TREE_PUBLIC (decl) = 1;
	sym-> treeDecl (decl);
    }

    Ymir::Tree declareGlobalExtern (const std::string & name, Ymir::Tree type_tree) {
	auto globName = Mangler::mangle_global (name);
	tree decl = build_decl (
	    BUILTINS_LOCATION,
	    VAR_DECL,
	    get_identifier (globName.c_str ()),
	    type_tree.getTree ()
	);

	TREE_STATIC (decl) = 1;
	//TREE_USED (decl) = 1;
	//DECL_EXTERNAL (decl) = 1;
	//TREE_PUBLIC (decl) = 1;
	return decl;
    }
    
    void finishCompilation () {
	int len = vec_safe_length (global_declarations);
	tree * addr = vec_safe_address (global_declarations);
	for (int i = 0 ; i < len ; i++) {
	    tree decl = addr [i];
	    wrapup_global_declarations (&decl, 1);
	}
    }

    tree promote (tree expr) {
	tree type = TREE_TYPE (expr);
	enum tree_code code = TREE_CODE (type);
	switch (code) {
	case BOOLEAN_TYPE : return convert (integer_type_node, expr);
	case ENUMERAL_TYPE : return convert (integer_type_node, expr);
	case INTEGER_TYPE : {
	    if (type == signed_char_type_node ||
		type == short_integer_type_node)
		return convert (integer_type_node, expr);
	    else if (type == unsigned_char_type_node ||
		     type == short_unsigned_type_node)
		return convert (unsigned_type_node, expr);
	    else return expr;
	}
	case REAL_TYPE : return convert (double_type_node, expr);
	default : return expr;
	}
    }
    
    Tree compoundExpr (location_t loc, Tree arg0, Tree arg1) {
	if (arg1.isNull ()) return arg0;
	if (arg0.isNull ()) return arg1;
	return fold_build2_loc (loc, COMPOUND_EXPR,
				arg1.getType ().getTree (),
				arg0.getTree (), arg1.getTree ());
    }       

    Tree compoundExpr (location_t loc, TreeStmtList arg0, Tree arg1) {
	if (arg1.isNull ()) return arg0.getTree ();
	return fold_build2_loc (loc, COMPOUND_EXPR,
				arg1.getType ().getTree (),
				arg0.getTree (), arg1.getTree ());
    }       

    
    Tree getExpr (TreeStmtList & list, Tree arg) {
	if (arg.getTreeCode () == COMPOUND_EXPR) {
	    list.append (arg.getOperand (0));
	    return arg.getOperand (1);
	} else if (arg.getTreeCode () == CALL_EXPR) {
	    auto aux = makeAuxVar (arg.getLocus (), ISymbol::getLastTmp (), arg.getType ());
	    list.append (buildTree (MODIFY_EXPR, arg.getLocus (), arg.getType (), aux, arg));
	    return aux;
	}
	return arg;
    }

    Tree getExpr (TreeStmtList & list, syntax::Expression expr) {
	auto arg = expr-> toGeneric ();
	if (arg.getTreeCode () == COMPOUND_EXPR) {
	    list.append (arg.getOperand (0));
	    return arg.getOperand (1);
	} else if (arg.getTreeCode () == CALL_EXPR) {
	    auto aux = makeAuxVar (arg.getLocus (), ISymbol::getLastTmp (), arg.getType ());
	    list.append (buildTree (MODIFY_EXPR, arg.getLocus (), arg.getType (), aux, arg));
	    return aux;
	}
	return arg;
    }

    Tree callLib (location_t locus, const std::string & name, Tree ret, std::vector <Tree> params) {
	std::vector <tree> fndecl_type_params (params.size ());
	std::vector <tree> real_params (params.size ());
	for (auto it : Ymir::r (0, params.size ())) {
	    fndecl_type_params [it] = params [it].getType ().getTree ();
	    real_params [it] = params [it].getTree ();
	}
	
	tree fndecl_type = build_function_type_array (ret.getTree (), fndecl_type_params.size (), fndecl_type_params.data ());
	tree fndecl = build_fn_decl (name.c_str (), fndecl_type);
	tree fn = build1 (ADDR_EXPR, build_pointer_type (fndecl_type), fndecl);
	return build_call_array_loc (locus, ret.getTree (), fn, real_params.size (), real_params.data ());
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

