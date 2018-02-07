#include <ymir/semantic/pack/InternalFunction.hh>
#include <ymir/semantic/tree/Tree.hh>

namespace semantic {

    Ymir::Tree InternalFunction::__fnMalloc__;
    Ymir::Tree InternalFunction::__y_newArray__;	
    Ymir::Tree InternalFunction::__y_memcpy__;
    Ymir::Tree InternalFunction::__y_memset__;
    Ymir::Tree InternalFunction::__y_main__;
    Ymir::Tree InternalFunction::__y_run_main__;
    std::map <std::string, Ymir::Tree> InternalFunction::__funcs__;
    
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
	if (__y_memcpy__.isNull ()) {
	    tree fndecl_type_params [] = {
		build_pointer_type (void_type_node),
		build_pointer_type (void_type_node),
		long_unsigned_type_node		
	    };

	    tree ret = build_pointer_type (void_type_node);
	    tree fndecl_type = build_function_type_array (ret, 2, fndecl_type_params);
	    tree fndecl = build_fn_decl ("memcpy", fndecl_type);
	    DECL_EXTERNAL (fndecl) = 1;

	    __y_memcpy__ = build1 (ADDR_EXPR, build_pointer_type (fndecl_type), fndecl);
	}
	return __y_memcpy__;
    }

    Ymir::Tree InternalFunction::getYMemset () {
	if (__y_memset__.isNull ()) {
	    tree fndecl_type_params [] = {
		build_pointer_type (void_type_node),
		long_unsigned_type_node,
		long_unsigned_type_node		
	    };

	    tree ret = build_pointer_type (void_type_node);
	    tree fndecl_type = build_function_type_array (ret, 2, fndecl_type_params);
	    tree fndecl = build_fn_decl ("memset", fndecl_type);
	    DECL_EXTERNAL (fndecl) = 1;

	    __y_memset__ = build1 (ADDR_EXPR, build_pointer_type (fndecl_type), fndecl);
	}
	return __y_memset__;
    }

    
    Ymir::Tree InternalFunction::getYInitType (const char * name) {
	auto it = __funcs__.find (name);
	if (it == __funcs__.end ()) {
	    tree ret = build_pointer_type (void_type_node);
	    tree fndecl_type = build_function_type_array (ret, 0, NULL);
	    tree fndecl = build_fn_decl ((std::string("_y_Init_") + name).c_str (), fndecl_type);
	    DECL_EXTERNAL (fndecl) = 1;
	    __funcs__ [name] = build1 (ADDR_EXPR, build_pointer_type (fndecl_type), fndecl);
	    return __funcs__ [name];
	}	
	return it-> second;
    }
        
    Ymir::Tree InternalFunction::getYMainPtr () {
	if (__y_main__.isNull ()) {
	    tree ret = int_type_node;
	    tree fndecl_type = build_function_type_array (ret, 0, NULL);
	    tree fndecl = build_fn_decl ("_Ymain", fndecl_type);
	    DECL_EXTERNAL (fndecl) = 1;
	    __y_main__ = build1 (ADDR_EXPR, build_pointer_type (fndecl_type), fndecl);
	}
	return __y_main__;
    }

    Ymir::Tree InternalFunction::getYRunMain () {
	if (__y_run_main__.isNull ()) {
	    tree args [] = {
		int_type_node,
		build_pointer_type (build_pointer_type (ubyte_type_node)),
		getYMainPtr ().getType ().getTree ()
	    };
	    tree ret = int_type_node;
	    tree fndecl_type = build_function_type_array (ret, 3, args);
	    tree fndecl = build_fn_decl ("y_run_main", fndecl_type);
	    DECL_EXTERNAL (fndecl) = 1;
	    __y_run_main__ = build1 (ADDR_EXPR, build_pointer_type (fndecl_type), fndecl);
	}
	return __y_run_main__;
    }

    
    
}
