#include <ymir/semantic/tree/Runtime.hh>

namespace Ymir {

    std::string Runtime::EXC_RETHROW = "_y_exc_rethrow";
    std::string Runtime::EXC_THROW = "_y_exc_throw";
    std::string Runtime::EXC_PUSH = "_y_exc_push";
    std::string Runtime::EXC_CHECK_TYPE = "_y_exc_check_type";
    std::string Runtime::SETJMP = "setjmp";

    int Runtime::JMP_BUF_SIZE = 200;

    std::string Runtime::ARRAY_INFO_STATIC = "StaticArray_info";
    std::string Runtime::ARRAY_INFO_DYNAMIC = "DynamicArray_info";
    std::string Runtime::TYPE_INFO_MODULE = "core.info";
    std::string Runtime::TYPE_INFO_SUFFIX = "_info";

    std::string Runtime::VTABLE_FIELD_TYPEINFO = "vtable";
    std::string Runtime::LEN_FIELD_TYPEINFO = "len";
    std::string Runtime::C_O_A_TYPEINFO = "c_o_a";
    
    std::string Runtime::STRUCT_INFO = "Struct_info";
    std::string Runtime::TUPLE_INFO = "Tuple_info";
    std::string Runtime::AGGREGATE_INFO = "Aggregate_info";
    std::string Runtime::CHAR_INFO = "char_info";
    std::string Runtime::BOOL_INFO = "bool_info";
    std::string Runtime::FLOAT_INFO = "f32_info";
    std::string Runtime::DOUBLE_INFO = "f64_info";

    std::string Runtime::FUNC_PTR_INFO = "FuncPtr_info";
    std::string Runtime::DELEGATE_INFO = "Delegate_info";

    std::string Runtime::PTR_INFO = "Ptr_info";
    std::string Runtime::RANGE_INFO = "Range_info";
    std::string Runtime::REF_INFO = "Ref_info";

    std::string Runtime::MEM_EQ_ARRAY = "_y_mem_equals_array";
    std::string Runtime::MEM_CMP_ARRAY = "_y_mem_cmp_array";
    std::string Runtime::COMPARE_TYPEINFO = "_y_cmp_typeinfo";
    std::string Runtime::DYNAMIC_CAST = "_y_dynamic_cast";
    std::string Runtime::ALLOC_AND_AFF = "_y_alloc_and_aff";

    std::string Runtime::MEM_INIT_ARRAY = "_y_mem_init";
    std::string Runtime::NEW_ARRAY = "_y_new_array";
    std::string Runtime::DUP_ARRAY = "_y_dup_array";
    
    
}
