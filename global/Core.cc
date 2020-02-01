#include <ymir/global/Core.hh>

namespace global {
    DECLARE_ENUM_WITH_TYPE (CoreNames, std::string,
			    ARRAY_ALLOC        = "_yrt_alloc_array", 
			    ARRAY_MODULE      = "array",
			    ASSERT_FUNC        = "abort",
			    BINARY_OP_OVERRIDE = "opBinary",
			    CLASS_ALLOC        = "_yrt_alloc_class", 
			    CORE_MODULE        = "core",
			    DCOPY_OP_OVERRIDE  = "deepCopy",
			    DCOPY_TRAITS       = "Copiable",
			    DUPLICATION_MODULE = "duplication",
			    DUPL_ANY           = "_yrt_dupl_any", 
			    DUPL_SLICE         = "_yrt_dup_slice", 
			    EXCEPTION_MODULE   = "exception",
			    EXCEPT_GET_VALUE   = "_yrt_exc_check_type", 
			    EXCEPT_POP         = "_yrt_exc_pop",
			    EXCEPT_PUSH        = "_yrt_exc_push", 
			    INDEX_OP_OVERRIDE = "opIndex",
			    JMP_BUF_TYPE       = "_yrt_jmp_buf_type", 
			    RETHROW            = "_yrt_exc_rethrow",
			    RUN_MAIN           = "_yrt_run_main", 
			    RUN_MAIN_DEBUG     = "_yrt_run_main_debug", 
			    SET_JMP            = "setjmp",
			    THROW              = "_yrt_exc_throw", 
			    TYPE_IDS           = "TypeIDs",
			    TYPE_INFO          = "TypeInfo",
			    TYPE_INFO_EQUAL    = "equals",
			    TYPE_INFO_MODULE   = "typeinfo",
    );
}
