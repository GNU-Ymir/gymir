#include <ymir/global/Core.hh>

namespace global {
    DECLARE_ENUM_WITH_TYPE (CoreNames, std::string,
			    ARRAY_ALLOC    = "_yrt_alloc_array", // (void* value, size_t value_size, size_t len)
			    CLASS_ALLOC    = "_yrt_alloc_class", // (size_t class_size)
			    DUPL_ANY       = "_yrt_dupl_any", // (void* value, size_t len),
			    DUPL_SLICE     = "_yrt_dup_slice", // (size_t len, void* data, size_t innerSize)
			    EXCEPT_GET_VALUE = "_yrt_exc_check_type", // void* (TypeInfo info)
			    EXCEPT_POP     = "_yrt_exc_pop",
			    EXCEPT_PUSH    = "_yrt_exc_push", // (jmp_buf *j, int returned)
			    JMP_BUF_TYPE   = "_yrt_jmp_buf_type", 
			    RETHROW        = "_yrt_exc_rethrow",
			    RUN_MAIN       = "_yrt_run_main", // (int, char**)
			    RUN_MAIN_DEBUG = "_yrt_run_main_debug", // (int, char**)
			    SET_JMP        = "setjmp",
			    THROW          = "_yrt_exc_throw", // (char *file, char *function, unsigned line, TypeInfo* info, void* data)
			    TYPE_INFO_EQUAL = "equals",
    );
}
