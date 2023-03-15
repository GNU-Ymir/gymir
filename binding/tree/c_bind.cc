#include "config.h"
#include "system.h"
#include "coretypes.h"

#define IN_GCC
#include "tm.h"
#undef IN_GCC

#include "tree.h"
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

#include "langhooks.h"
#include "ubsan.h"
#include "attribs.h"
#include "asan.h"
#include "tree-pretty-print.h"
#include "cppdefault.h"

#include <ymir/ymir1.hh>
#include <map>
#include <string>

extern "C" void _yrt_exc_panic (const char * filename, const char * function, uint64_t line);

/**
 * =========================================================================
 * =========================================================================
 * ======================          STATEMENTS            ===================
 * =========================================================================
 * =========================================================================
 * */


extern "C" tree c_binding_alloc_stmt_list () {
    return alloc_stmt_list ();
}

extern "C" void c_binding_append_to_statement_list (tree left, tree * right) {
    append_to_statement_list (left, right);
}

extern "C" void c_binding_tree_chain_set (tree left, tree right) {
    TREE_CHAIN (left) = right;
}

extern "C" void c_binding_block_chain_set (tree left, tree right) {
    BLOCK_CHAIN (left) = right;
}

extern "C" tree c_binding_block_chain_get (tree elem) {
    return BLOCK_CHAIN (elem);
}


extern "C" tree c_binding_build_block_tree (location_t, tree varChain, tree blockChain) {
    return build_block (varChain, blockChain, nullptr, nullptr);
}

extern "C" tree c_binding_build_bind_tree (location_t loc, tree varChain, tree content, tree block) {
    return build3_loc (loc, BIND_EXPR, void_type_node, varChain, content, block);
}

/**
 * =========================================================================
 * =========================================================================
 * ======================             TYPES              ===================
 * =========================================================================
 * =========================================================================
 * */

extern "C" tree c_binding_error_tree () {
    return error_mark_node;
}

extern "C" tree c_binding_build_block (tree vars, tree chain, tree a, tree b) {
    return build_block (vars, chain, a, b);
}

extern "C" tree c_binding_get_void_type () {
    return void_type_node;
}

extern "C" tree c_binding_get_bool_type () {
    return y_bool_type;
}

extern "C" tree c_binding_get_int_type (int32_t size, bool isSigned) {
    if (isSigned) {
	    switch (size) {
	    case 8 : return y_i8_type;
	    case 16 : return y_i16_type;
	    case 32 : return y_i32_type;
	    case 64 : return y_i64_type;
	    default : return y_isize_type;
	    }
	} else {
	    switch (size) {
	    case 8 : return y_u8_type;
	    case 16 : return y_u16_type;
	    case 32 : return y_u32_type;
	    case 64 : return y_u64_type;
	    default : return y_usize_type;
	    }
	}
}

extern "C" tree c_binding_get_float_type (int32_t size) {
    switch (size) {
    case 32: return y_f32_type;
    default: return y_f64_type;
    }
}

extern "C" tree c_binding_get_char_type (int32_t size) {
    switch (size) {
    case 8: return y_c8_type;
    case 16: return y_c16_type;
    default: return y_c32_type;
    }
}

extern "C" tree c_binding_build_array_type (tree innertype, int32_t size) {
    auto len = build_int_cst_type (y_usize_type, size - 1);
    auto begin = build_int_cst_type (y_usize_type, 0);
    auto range = build_range_type (y_i32_type, fold (begin), fold (len));
    return build_array_type (innertype, range);
}

extern "C" tree c_binding_build_pointer_type (tree innerType) {
    return build_pointer_type (innerType);
}

extern "C" tree c_binding_build_tuple_type (const char * name, uint64_t nbFields, tree* fieldTypes, uint64_t nbFieldNames, const char ** fieldNames, bool isUnion, bool isPacked) {
    tree field_last = nullptr, field_begin = nullptr;
    tree record_type = nullptr;
    if (isUnion) {
        record_type = make_node (UNION_TYPE);
    } else record_type = make_node (RECORD_TYPE);

    auto size = 0;
    for (uint64_t i = 0 ; i < nbFields ; i++) {
        tree ident ;
        if (i >= nbFieldNames) {
            char buffer [64];
            snprintf (buffer, 64, "_%ld", i);
            ident = get_identifier (buffer);
        } else ident = get_identifier (fieldNames [i]);

        tree type = fieldTypes [i];
        size += TREE_INT_CST_LOW (TYPE_SIZE_UNIT (type));

        tree field = build_decl (BUILTINS_LOCATION, FIELD_DECL, ident, type);
        DECL_FIELD_CONTEXT (field) = record_type;

        if (field_begin == nullptr) field_begin = field;
        if (field_last != nullptr) TREE_CHAIN (field_last) = field;
        field_last = field;
    }

    TYPE_NAME (record_type) = get_identifier (name);
    if (field_last != nullptr) TREE_CHAIN (field_last) = nullptr;

    TYPE_FIELDS (record_type) = field_begin;
    layout_type (record_type);

    if (isPacked && !isUnion) {
        TYPE_SIZE (record_type) = bitsize_int (size * BITS_PER_UNIT);
        TYPE_SIZE_UNIT (record_type) = size_int (size);
        TYPE_PACKED (record_type) = 1;
        SET_TYPE_ALIGN (record_type, 1 * BITS_PER_UNIT);
        compute_record_mode (record_type);
    } else compute_record_mode (record_type);

    return record_type;
}

extern "C" tree c_binding_build_option_type (uint64_t nbFieldNames, const char ** fieldNames, uint64_t nbCommon, tree * commons, uint64_t nbUnions, tree * unions) {
    tree field_begin = nullptr, field_last = nullptr;
    tree record_type = make_node (RECORD_TYPE);

    for (uint64_t i = 0 ; i < nbCommon ; i++) {
        tree ident = nullptr;
        if (i >= nbFieldNames) {
            char buffer [64];
            snprintf (buffer, 64, "_%ld", i);
            ident = get_identifier (buffer);
        } else ident = get_identifier (fieldNames [i]);

        tree type = commons [i];
        tree field = build_decl (BUILTINS_LOCATION, FIELD_DECL, ident, type);
        DECL_FIELD_CONTEXT (field) = record_type;

        if (field_begin == nullptr) field_begin = field;
        if (field_last != nullptr) TREE_CHAIN (field_last) = field;
        field_last = field;
    }

    tree offset = nullptr;
    for (uint64_t i = 0 ; i < nbUnions ; i++) {
        tree ident = nullptr;
        if (i + nbCommon >= nbFieldNames) {
            char buffer [64];
            snprintf (buffer, 64, "_%ld", i + nbCommon);
            ident = get_identifier (buffer);
        } else ident = get_identifier (fieldNames [i + nbCommon]);

        tree type = unions [i];
        tree field = build_decl (BUILTINS_LOCATION, FIELD_DECL, ident, type);
        DECL_FIELD_CONTEXT (field) = record_type;
        if (i == 0) offset = DECL_FIELD_OFFSET (field);
        else DECL_FIELD_OFFSET (field) = offset;

        if (field_begin == nullptr) field_begin = field;
        if (field_last != nullptr) TREE_CHAIN (field_last) = field;
        field_last = field;
    }

    if (field_last != nullptr) TREE_CHAIN (field_last) = nullptr;

    TYPE_NAME (record_type) = get_identifier ("option");
    TYPE_FIELDS (record_type) = field_begin;
    layout_type (record_type);

    return record_type;
}

extern "C" tree c_binding_build_function_type (tree retType, uint64_t nbParams, tree * params) {
    return build_function_type_array (retType, nbParams, params);
}

extern "C" tree c_binding_build_method_type (tree retType, tree self, uint64_t nbParams, tree * params) {
    return build_method_type (self, build_function_type_array (retType, nbParams, params));
}


/**
 * =========================================================================
 * =========================================================================
 * =========================           TESTS           =====================
 * =========================================================================
 * =========================================================================
 * */


extern "C" bool c_binding_is_scalar (tree type) {
    if (type == nullptr) return false;
    auto code = TREE_CODE (type);

    switch (code) {
    case RECORD_TYPE:
    case ARRAY_TYPE:
    case UNION_TYPE:
        return false;
    default:
        return true;
    }
}

extern "C" bool c_binding_is_pointer (tree type) {
    if (type == nullptr) return false;
    return TREE_CODE (type) == POINTER_TYPE;
}

extern "C" bool c_binding_is_float (tree type) {
    return (type == y_f32_type) || (type == y_f64_type);
}

extern "C" bool c_binding_is_compound (tree value) {
    if (value == nullptr) return false;
    return (TREE_CODE (value) == COMPOUND_EXPR);
}

extern "C" tree c_binding_get_list_operand (tree value) {
    if (value == nullptr) return nullptr;
    if (TREE_CODE (value) == COMPOUND_EXPR) {
        return TREE_OPERAND (value, 0);
    } else return nullptr;
}

extern "C" tree c_binding_get_value_operand (tree value) {
    if (value == nullptr) return nullptr;
    if (TREE_CODE (value) == COMPOUND_EXPR) {
        return TREE_OPERAND (value, 1);
    } else return value;
}

extern "C" tree c_binding_unref_pointer (tree value, uint64_t index) {
    auto ptrType = TREE_TYPE (value);
    auto inner = TREE_TYPE (ptrType);
    auto innerSize = TYPE_SIZE_UNIT (inner);

    if (index == 0) {
        return build2 (MEM_REF, inner, value, build_int_cst_type (ptrType, 0));
    } else {
        auto it = build_int_cst_type (y_usize_type, index);
        auto offset = convert_to_ptrofftype (fold_build2 (MULT_EXPR, y_usize_type, it, innerSize));

        auto addr = build2 (POINTER_PLUS_EXPR, ptrType, value, offset);
        return build2 (MEM_REF, inner, addr, build_int_cst_type (ptrType, 0));
    }
}

extern "C" tree c_binding_unref_pointer_force_type (tree value, tree type, uint64_t index) {
    auto ptrType = build_pointer_type (type);
    auto inner = type;
    auto innerSize = TYPE_SIZE_UNIT (inner);

    if (index == 0) {
        return build2 (MEM_REF, inner, value, build_int_cst_type (ptrType, 0));
    } else {
        auto it = build_int_cst_type (y_usize_type, index);
        auto offset = convert_to_ptrofftype (fold_build2 (MULT_EXPR, y_usize_type, it, innerSize));

        auto addr = build2 (POINTER_PLUS_EXPR, ptrType, value, offset);
        return build2 (MEM_REF, inner, addr, build_int_cst_type (ptrType, 0));
    }
}



extern "C" tree c_binding_access_field_by_name (tree value, const char * fieldname) {
    tree type = TREE_TYPE (value);

    tree field_decl = TYPE_FIELDS (type);
    while (field_decl != nullptr) {
        tree decl_name = DECL_NAME (field_decl);
        if (strcmp (IDENTIFIER_POINTER (decl_name), fieldname) == 0) {
            break;
        }
        else field_decl = TREE_CHAIN (field_decl);
    }

    if (field_decl == nullptr) return nullptr;
    return build2 (COMPONENT_REF, TREE_TYPE (field_decl), value, field_decl);
}

extern "C" tree c_binding_to_direct_value (tree value) {
    if (value == nullptr) return nullptr;
    if (TREE_CODE (TREE_TYPE (value)) == POINTER_TYPE) {
        return c_binding_unref_pointer (value, 0);
    } else return value;
}

extern "C" bool c_binding_is_string_type (tree type) {
    if (type == nullptr) return false;
    if (TREE_CODE (type) != POINTER_TYPE) {
        return false;
    }

    return (TYPE_MAIN_VARIANT (TREE_TYPE (type)) == y_c8_type);
}

extern "C" bool c_binding_is_array_type (tree type) {
    return TREE_CODE (type) == ARRAY_TYPE;
}

extern "C" bool c_binding_is_empty_stmt_list (tree value) {
    if (TREE_CODE (value) != STATEMENT_LIST) return false;

    return STATEMENT_LIST_HEAD (value) == nullptr;
}

extern "C" bool c_binding_is_stmt_list (tree value) {
    return TREE_CODE (value) == STATEMENT_LIST;
}

extern "C" tree c_binding_get_type (tree value) {
    return TREE_TYPE (value);
}

extern "C" void c_binding_set_type (tree value, tree type) {
    TREE_TYPE (value) = type;
}

extern "C" tree c_binding_get_operand (tree t, uint32_t index) {
    return TREE_OPERAND (t, index);
}

extern "C" tree c_binding_get_array_size (tree type) {
    auto range = TYPE_DOMAIN (type);
    return convert (y_usize_type,
                    build2_loc (UNKNOWN_LOCATION, PLUS_EXPR,
                                y_i32_type,
                                TYPE_MAX_VALUE (range),
                                build_int_cst_type (y_i32_type, 1))
        );
}

extern "C" tree c_binding_get_string_size (tree type) {
    uint32_t innerSize = 0;
    if (TYPE_MAIN_VARIANT (TREE_TYPE (type)) == y_c8_type) {
        innerSize = 1;
    } else if (TYPE_MAIN_VARIANT (TREE_TYPE (type)) == y_c16_type) {
        innerSize =  2;
    } else innerSize = 4;

    auto t = c_binding_get_operand (c_binding_get_operand (type, 0), 0);
    return build_int_cst_type (y_usize_type,
                               (TREE_STRING_LENGTH (t) / innerSize) - 1);
}

extern "C" uint32_t c_binding_get_type_size (tree type) {
    if (type == nullptr) return 0;
    return TREE_INT_CST_LOW (TYPE_SIZE_UNIT (type));
}

extern "C" bool c_binding_is_error (tree type) {
    return error_operand_p (type);
}

extern "C" bool c_binding_is_static (tree t) {
    if (t == nullptr) return false;
    return TREE_STATIC (t);
}

extern "C" void c_binding_set_static (tree t, bool set) {
    TREE_STATIC (t) = set;
}

extern "C" bool c_binding_is_global_ctor (tree t) {
    return DECL_STATIC_CONSTRUCTOR (t);
}

extern "C" void c_binding_set_global_ctor (tree t, bool set) {
    DECL_STATIC_CONSTRUCTOR (t) = set;
}

extern "C" bool c_binding_is_used (tree t) {
    return TREE_USED (t);
}

extern "C" void c_binding_set_used (tree t, bool set) {
    TREE_USED (t) = set;
}

extern "C" bool c_binding_is_public (tree t) {
    return TREE_PUBLIC (t);
}

extern "C" void c_binding_set_public (tree t, bool set) {
    TREE_PUBLIC (t) = set;
}

extern "C" bool c_binding_is_weak (tree t) {
    return DECL_WEAK (t);
}

extern "C" void c_binding_set_weak (tree t, bool set) {
    DECL_WEAK (t) = set;
}

extern "C" bool c_binding_is_external (tree t) {
    return DECL_EXTERNAL (t);
}

extern "C" void c_binding_set_external (tree t, bool set) {
    DECL_EXTERNAL (t) = set;
}

extern "C" bool c_binding_is_preserved (tree t) {
    return DECL_PRESERVE_P (t);
}

extern "C" void c_binding_set_preserved (tree t, bool set) {
    DECL_PRESERVE_P (t) = set;
}

extern "C" bool c_binding_is_addressable (tree t) {
    return TREE_ADDRESSABLE (t);
}

extern "C" void c_binding_set_addressable (tree t, bool set) {
    TREE_ADDRESSABLE (t) = set;
}

extern "C" const char* c_binding_get_asm_name (tree t) {
    if (t == nullptr) return nullptr;
    return IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (t));
}

extern "C" void c_binding_set_asm_name (tree t, const char * name) {
    SET_DECL_ASSEMBLER_NAME (t, get_identifier (name));
}

extern "C" const char* c_binding_get_name (tree t) {
    if (t == nullptr) return nullptr;
    return IDENTIFIER_POINTER (DECL_NAME (t));
}

extern "C" void c_binding_set_decl_context (tree t, tree context) {
    DECL_CONTEXT (t) = context;
}

extern "C" tree c_binding_get_decl_context (tree t) {
    return DECL_CONTEXT (t);
}

extern "C" void c_binding_set_decl_initial (tree t, tree init) {
    DECL_INITIAL (t) = init;
}

extern "C" tree c_binding_get_decl_initial (tree t) {
    return DECL_INITIAL (t);
}

extern "C" void c_binding_set_decl_saved_tree (tree t, tree saved) {
    DECL_SAVED_TREE (t) = saved;
}

extern "C" tree c_binding_get_decl_saved_tree (tree t) {
    return DECL_SAVED_TREE (t);
}

extern "C" void c_binding_set_decl_arguments (tree t, uint64_t nbArgs, tree * args) {
    tree arglist = nullptr;
    for (uint64_t i = 0 ; i < nbArgs ; i++) {
        arglist = chainon (arglist, args [i]);
    }

    DECL_ARGUMENTS (t) = arglist;
}

extern "C" void c_binding_set_arg_type (tree t, tree type) {
    DECL_ARG_TYPE (t) = type;
}

extern "C" tree c_binding_get_arg_type (tree t) {
    return DECL_ARG_TYPE (t);
}

extern "C" void c_binding_set_result_decl (tree t, tree result) {
    DECL_RESULT (t) = result;
}

extern "C" tree c_binding_get_result_decl (tree t) {
    return DECL_RESULT (t);
}

extern "C" void c_binding_set_block_super_context (tree t, tree super) {
    BLOCK_SUPERCONTEXT (t) = super;
}

extern "C" tree c_binding_get_block_super_context (tree t) {
    return BLOCK_SUPERCONTEXT (t);
}



/**
 * =========================================================================
 * =========================================================================
 * ======================           DECLARATION           ==================
 * =========================================================================
 * =========================================================================
 * */


extern "C" location_t c_binding_get_builtin_location () {
    return BUILTINS_LOCATION;
}

extern "C" location_t c_binding_get_unknwon_location () {
    return UNKNOWN_LOCATION;
}

extern "C" location_t c_binding_build_debug_location (const char * filename, uint64_t line, uint64_t col) {
    static std::map <std::string, char*> __filename__;
    char * name = nullptr;
    auto it = __filename__.find (std::string (filename));
    if (it == __filename__.end ()) {
        auto len = strlen (filename);
        char * aux = new char [len + 1];

        memcpy (aux, filename, len);
        aux [len] = 0;
        name = aux;
    } else {
        name = it-> second;
    }

    linemap_add (line_table, LC_ENTER, 0, name, line);
    linemap_line_start (line_table, line, 0);
    auto ret = linemap_position_for_column (line_table, col);
    linemap_add (line_table, LC_LEAVE, 0, NULL, 0);

    return ret;
}

extern "C" tree c_binding_build_var_decl (location_t loc, const char * name, tree type) {
    return build_decl (loc, VAR_DECL, get_identifier (name), type);
}

extern "C" tree c_binding_build_decl (location_t loc, tree type, tree content) {
    return build1_loc (loc, DECL_EXPR, type, content);
}

extern "C" tree c_binding_build_param_decl (location_t loc, const char * name, tree type) {
    return build_decl (loc, PARM_DECL, get_identifier (name), type);
}

extern "C" tree c_binding_build_function_decl (location_t loc, const char * name, tree type) {
    auto fnDecl = build_fn_decl (name, type);
    TREE_NOTHROW (fnDecl) = 0;
    TREE_PUBLIC (fnDecl) = 0;
    TREE_STATIC (fnDecl) = 1;

    return fnDecl;
}

extern "C" tree c_binding_build_result_decl (location_t loc, tree type) {
    return build_decl (loc, RESULT_DECL, nullptr, type);
}


/**
 * =========================================================================
 * =========================================================================
 * ======================           EXPRESSION           ===================
 * =========================================================================
 * =========================================================================
 * */

extern "C" tree c_binding_build_modify_expr (location_t loc, tree type, tree left, tree right) {
    return build2_loc (loc, MODIFY_EXPR, type, left, convert (type, right));
}

extern "C" tree c_binding_build_address (location_t loc, tree type, tree value) {
    TREE_ADDRESSABLE (value) = true;
    return build1_loc (loc, ADDR_EXPR, type, value);
}

extern "C" tree c_binding_build_memcpy (location_t loc, tree left, tree right) {
    auto memcpyFunc = builtin_decl_explicit (BUILT_IN_MEMCPY);
    auto size = TYPE_SIZE_UNIT (TREE_TYPE (left));

    auto lPtr = c_binding_build_address (loc, c_binding_build_pointer_type (TREE_TYPE (left)), left);
    auto rPtr = c_binding_build_address (loc, c_binding_build_pointer_type (TREE_TYPE (right)), right);

    return build_call_expr (memcpyFunc, 3, lPtr, rPtr, size);
}

extern "C" tree c_binding_build_compound (tree list, tree value) {
    if (list == nullptr) return value;
    if (value == nullptr) return list;

    return fold_build2 (COMPOUND_EXPR, TREE_TYPE (value), list, value);
}

extern "C" tree c_binding_build_call (location_t loc, tree retType, tree fn, uint64_t nbArgs, tree* args) {
    return build_call_array_loc (loc, retType, fn, nbArgs, args);
}

extern "C" tree c_binding_build_array_ref (location_t loc, tree array, tree index) {
    auto innerType = TREE_TYPE (TREE_TYPE (array));
    return build2_loc (loc, ARRAY_REF, innerType, array, index);
}


extern "C" tree c_binding_build_binary (location_t loc, const char * op, tree type, tree left, tree right, bool pointer, bool fl) {
    tree_code code = LSHIFT_EXPR;
    if (strcmp (op, "<<") == 0) { code = LSHIFT_EXPR; }
    else if (strcmp (op, ">>") == 0) { code = RSHIFT_EXPR; }
    else if (strcmp (op, "|") == 0) { code = BIT_IOR_EXPR; }
    else if (strcmp (op, "&") == 0) { code = BIT_AND_EXPR; }
    else if (strcmp (op, "^") == 0) { code = BIT_XOR_EXPR; }
    else if (strcmp (op, "+") == 0) { if (pointer) { code = POINTER_PLUS_EXPR; } else { code = PLUS_EXPR; } }
    else if (strcmp (op, "-") == 0) { if (pointer) { code = POINTER_DIFF_EXPR; } else { code = MINUS_EXPR; } }
    else if (strcmp (op, "*") == 0) { code = MULT_EXPR; }
    else if (strcmp (op, "/") == 0) { if (fl) { code = RDIV_EXPR; } else { code = TRUNC_DIV_EXPR; } }
    else if (strcmp (op, "%") == 0) { code = TRUNC_MOD_EXPR; }
    else if (strcmp (op, "<") == 0) { code = LT_EXPR; }
    else if (strcmp (op, ">") == 0) { code = GT_EXPR; }
    else if (strcmp (op, "<=") == 0) { code = LE_EXPR; }
    else if (strcmp (op, ">=") == 0) { code = GE_EXPR; }
    else if (strcmp (op, "==") == 0) { code = EQ_EXPR; }
    else if (strcmp (op, "!=") == 0) { code = NE_EXPR; }
    else if (strcmp (op, "&&") == 0) { code = TRUTH_ANDIF_EXPR; }
    else if (strcmp (op, "||") == 0) { code = TRUTH_ORIF_EXPR; }
    else _yrt_exc_panic (__FILE__, __FUNCTION__, __LINE__);

    return build2_loc (loc, code, type, left, right);
}

extern "C" tree c_binding_build_label_decl (location_t loc, const char * name) {
    return build_decl (loc, LABEL_DECL, get_identifier (name), void_type_node);
}

extern "C" tree c_binding_build_label_expr (location_t loc, tree label) {
    return build1_loc (loc, LABEL_EXPR, void_type_node, label);
}

extern "C" tree c_binding_build_goto_expr (location_t loc, tree label) {
    return build1_loc (loc, GOTO_EXPR, void_type_node, label);
}

extern "C" tree c_binding_build_return_expr (location_t loc, tree expression) {
    return build1_loc (loc, RETURN_EXPR, void_type_node, expression);
}

extern "C" tree c_binding_build_cond_expr (location_t loc, tree test, tree gotoS, tree gotoF) {
    return build3_loc (loc, COND_EXPR, void_type_node, test, gotoS, gotoF);
}

extern "C" tree c_binding_build_int_cst_value (tree type, uint64_t value) {
    return build_int_cst_type (type, value);
}

/**
 * =========================================================================
 * =========================================================================
 * ========================           CONVERT           ====================
 * =========================================================================
 * =========================================================================
 * */


extern "C" tree c_binding_convert (tree type, tree value) {
    return convert (type, value);
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
	    _yrt_exc_panic (__FILE__, __FUNCTION__, __LINE__);
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

    debug_tree (type);
    _yrt_exc_panic (__FILE__, __FUNCTION__, __LINE__);
    return error_mark_node;
}

/**
 * =========================================================================
 * =========================================================================
 * =======================           FINALIZE           ====================
 * =========================================================================
 * =========================================================================
 * */

extern "C" void c_binding_gimplify_function (tree t) {
    gimplify_function_tree (t);
}

extern "C" void c_binding_finalize_function (tree t) {
    cgraph_node::finalize_function (t, true);
}

/**
 * =========================================================================
 * =========================================================================
 * =========================           DEBUG           =====================
 * =========================================================================
 * =========================================================================
 * */


extern "C" void c_binding_debug_print_tree (tree t) {
    debug_tree (t);
}

extern "C" void c_binding_debug_generic_print_tree (tree t) {
    print_generic_stmt (stdout, t);
}
