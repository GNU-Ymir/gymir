#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/types/InfoType.hh>
#include <ymir/errors/Error.hh>

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

	TREE_CHAIN (fields_last) = NULL_TREE;
	
	TYPE_FIELDS (record_type) = fields_begin;
	//finish_builtin_struct (type, name.c_str (), fields, NULL_TREE);
	layout_type (record_type);

	return record_type;
    }

    Tree makeTuple (std::string name, std::vector <InfoType> types, std::vector <std::string> attrs) {
	Tree field_last;
	Tree record_type = make_node (RECORD_TYPE);
	
	for (int i = 0 ; i < types.size () ; i++) {
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
    
}
