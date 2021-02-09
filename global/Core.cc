#include <ymir/global/Core.hh>

namespace global {
    DECLARE_ENUM_WITH_TYPE (CoreNames, std::string,
			    ARRAY_ALLOC		     = "_yrt_alloc_array", 
			    ARRAY_MODULE	     = "array",
			    ASSERT_FUNC		     = "abort",
			    ATOMIC_LOCK              = "_yrt_atomic_enter",
			    ATOMIC_MODULE            = "atom",
			    ATOMIC_MONITOR_LOCK      = "_yrt_atomic_monitor_enter",
			    ATOMIC_MONITOR_UNLOCK    = "_yrt_atomic_monitor_exit",
			    ATOMIC_UNLOCK            = "_yrt_atomic_exit",
			    BEGIN_OP_OVERRIDE	     = "begin",
			    BINARY_OP_OVERRIDE	     = "opBinary",
			    BINARY_OP_OVERRIDE_RIGHT = "opBinaryRight",
			    CLASS_ALLOC		     = "_yrt_alloc_class", 
			    CONTAIN_OP_OVERRIDE	     = "opContains",
			    CORE_MODULE		     = "core",
			    DCOPY_OP_OVERRIDE	     = "deepCopy",
			    DCOPY_TRAITS	     = "Copiable",
			    DEBUG_VERSION	     = "Debug",
			    DISPOSABLE_TRAITS	     = "Disposable",
			    DISPOSE_OP_OVERRIDE	     = "dispose",
			    DISPOSING_MODULE	     = "dispose",
			    DOLLAR_OP_OVERRIDE	     = "opDollar",
			    DUPLICATION_MODULE	     = "duplication",
			    DUPL_ANY		     = "_yrt_dupl_any", 
			    DUPL_SLICE		     = "_yrt_dup_slice", 
			    END_OP_OVERRIDE	     = "end",
			    EQUALS_OP_OVERRIDE	     = "opEquals",
			    EXCEPTION_MODULE	     = "exception",
			    EXCEPTION_RETHROW_FUNC   = "rethrow",
			    EXCEPTION_TYPE	     = "Exception",
			    EXCEPT_GET_VALUE	     = "_yrt_exc_check_type", 
			    EXCEPT_POP		     = "_yrt_exc_pop",
			    EXCEPT_PUSH		     = "_yrt_exc_push", 
			    GET_OP_OVERRIDE	     = "get",
			    INDEX_ASSIGN_OP_OVERRIDE = "opIndexAssign",
			    INDEX_OP_OVERRIDE	     = "opIndex",
			    JMP_BUF_TYPE	     = "_yrt_jmp_buf_type", 
			    LOGICAL_OP_OVERRIDE	     = "opCmp",
			    MUTEX_INIT               = "_YRT_MUTEX_INIT",
			    MUTEX_TYPE               = "pthread_mutex_t",
			    NEXT_OP_OVERRIDE	     = "next",
			    OBJECT_MODULE	     = "object",
			    OBJECT_TYPE		     = "Object",
			    OUT_OF_ARRAY	     = "outOfArray",
			    OUT_OF_ARRAY_TYPE	     = "OutOfArray",
			    RETHROW		     = "_yrt_exc_rethrow",
			    RUN_MAIN		     = "_yrt_run_main", 
			    RUN_MAIN_DEBUG	     = "_yrt_run_main_debug", 
			    SEG_FAULT_TYPE	     = "SegFault",
			    SET_JMP		     = "setjmp",
			    SLICE_CONCAT	     = "_yrt_concat_slices",
			    THROW		     = "_yrt_exc_throw", 
			    TYPE_IDS		     = "TypeIDs",
			    TYPE_INFO		     = "TypeInfo",
			    TYPE_INFO_EQUAL	     = "equals",
			    TYPE_INFO_MODULE	     = "typeinfo",
			    UNARY_ASSIGN_OP_OVERRIDE = "opUnaryAssign",
			    UNARY_OP_OVERRIDE	     = "opUnary",
	);
}
