#include <ymir/semantic/pack/PreBuiltPtr.hh>
#include <ymir/semantic/tree/Tree.hh>

namespace semantic {

    Ymir::Tree InternalFunction::__fnMalloc__;

    Ymir::Tree InternalFunction::getMalloc () {
	if (__fnMalloc__.isNull ()) {

	    tree fndecl_type_params[] = {
		long_unsigned_type_node
	    };

	    tree ret = build_pointer_type (
		void_type_node
	    );

	    tree fndecl_type = build_function_type_array (ret, 0, fndecl_type_params);
	    tree fndecl = build_fn_decl ("GC_malloc", fndecl_type);
	    DECL_EXTERNAL (fndecl) = 1;

	    __fnMalloc__ = build1 (ADDR_EXPR, build_pointer_type (fndecl_type), fndecl);
	}
	return __fnMalloc__;
    }
    

}
