#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/types/InfoType.hh>
#include <ymir/errors/Error.hh>
#include <ymir/semantic/pack/FinalFrame.hh>

using namespace semantic;

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
    
    Tree makeStructType (std::string, int nbfields, ...) {
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

	TREE_CHAIN (fields_last) = NULL_TREE;
	
	TYPE_FIELDS (record_type) = fields_begin;
	//finish_builtin_struct (type, name.c_str (), fields, NULL_TREE);
	layout_type (record_type);

	return record_type;
    }

    Tree makeTuple (std::string, std::vector <InfoType> types, std::vector <std::string> attrs) {
	Tree field_last;
	Tree record_type = make_node (RECORD_TYPE);
	
	for (int i = 0 ; i < (int) types.size () ; i++) {
	    Tree field_decl = makeField (types [i], attrs [i]);
	    DECL_CONTEXT (field_decl.getTree ()) = record_type.getTree ();
	    TREE_PUBLIC (field_decl.getTree ()) = 1;
	    TREE_CHAIN (field_decl.getTree ()) = field_last.getTree ();	    
	    field_last = field_decl;
	}
	
	TYPE_FIELDS (record_type.getTree ()) = field_last.getTree ();
	
	//layout_type (record_type.getTree ());
	return record_type;
    }

    Tree getField (location_t loc, Tree obj, std::string name) {
	Tree field_decl = TYPE_FIELDS (TREE_TYPE (obj.getTree ()));

	while (!field_decl.isNull ()) {
	    //DECL_CONTEXT (field_decl.getTree ()) = obj.getTree ();
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
	Tree it = build_int_cst_type (long_unsigned_type_node, index);
	return getPointerUnref (loc, ptr, inner, it);
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
	return buildTree (ADDR_EXPR, UNKNOWN_LOCATION, build_pointer_type (elem.getType ().getTree ()), elem);
    }

    Tree getAddr (location_t loc, Tree elem) {
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

    
    
}
