#include <ymir/semantic/pack/InternalFunction.hh>
#include <ymir/semantic/tree/Tree.hh>

namespace semantic {

    Ymir::Tree InternalFunction::__fnMalloc__;
    Ymir::Tree InternalFunction::__y_newArray__;	
    Ymir::Tree InternalFunction::__y_Init_int__;
    Ymir::Tree InternalFunction::__y_memcpy__;
    
    Ymir::Tree InternalFunction::getMalloc () {
	if (__fnMalloc__.isNull ()) {

	    tree fndecl_type_params[] = {
		long_unsigned_type_node
	    };

	    tree ret = build_pointer_type (
		void_type_node
	    );

	    tree fndecl_type = build_function_type_array (ret, 1, fndecl_type_params);
	    tree fndecl = build_fn_decl ("GC_malloc", fndecl_type);
	    DECL_EXTERNAL (fndecl) = 1;

	    __fnMalloc__ = build1 (ADDR_EXPR, build_pointer_type (fndecl_type), fndecl);
	}
	return __fnMalloc__;
    }

    Ymir::Tree InternalFunction::getYNewArray () {
	if (__y_newArray__.isNull ()) {
	    tree fndecl_type_params [] = {
		build_pointer_type (void_type_node),
		long_unsigned_type_node		
	    };

	    tree ret = build_pointer_type (void_type_node);
	    tree fndecl_type = build_function_type_array (ret, 2, fndecl_type_params);
	    tree fndecl = build_fn_decl ("_y_newArray", fndecl_type);
	    DECL_EXTERNAL (fndecl) = 1;

	    __y_newArray__ = build1 (ADDR_EXPR, build_pointer_type (fndecl_type), fndecl);
	}
	return __y_newArray__;
    }

    Ymir::Tree InternalFunction::getYMemcpy () {
	if (__y_newArray__.isNull ()) {
	    tree fndecl_type_params [] = {
		build_pointer_type (void_type_node),
		build_pointer_type (void_type_node),
		long_unsigned_type_node		
	    };

	    tree ret = build_pointer_type (void_type_node);
	    tree fndecl_type = build_function_type_array (ret, 2, fndecl_type_params);
	    tree fndecl = build_fn_decl ("_y_memcpy", fndecl_type);
	    DECL_EXTERNAL (fndecl) = 1;

	    __y_newArray__ = build1 (ADDR_EXPR, build_pointer_type (fndecl_type), fndecl);
	}
	return __y_newArray__;
    }
    
    Ymir::Tree InternalFunction::getYInitInt () {
	if (__y_Init_int__.isNull ()) {
	    tree ret = build_pointer_type (void_type_node);
	    tree fndecl_type = build_function_type_array (ret, 0, NULL);
	    tree fndecl = build_fn_decl ("_y_Init_int", fndecl_type);
	    DECL_EXTERNAL (fndecl) = 1;
	    __y_Init_int__ = build1 (ADDR_EXPR, build_pointer_type (fndecl_type), fndecl);
	}
	return __y_Init_int__;
    }

    

}
