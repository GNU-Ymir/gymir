#include <ymir/global/Core.hh>

namespace global {
    DECLARE_ENUM_WITH_TYPE (CoreNames, std::string,
			    RUN_MAIN       = "_yrt_run_main", // (int, char**)
			    RUN_MAIN_DEBUG = "_yrt_run_main_debug", // (int, char**)
			    DUPL_SLICE     = "_yrt_dup_slice", // (size_t len, void* data, size_t innerSize)
			    ARRAY_ALLOC    = "_yrt_alloc_array", // (void* value, size_t value_size, size_t len)
			    DUPL_ANY       = "_yrt_dupl_any", // (void* value, size_t len),
			    THROW          = "_yrt_exc_throw", // (char *file, char *function, unsigned line, TypeInfo* info, void* data)
			    SET_JMP        = "setjmp",
			    RETHROW        = "_yrt_exc_rethrow",
			    EXCEPT_PUSH    = "_yrt_exc_push", // (jmp_buf *j, int returned) {
			    JMP_BUF_TYPE   = "_yrt_jmp_buf_type", 
    );
}
