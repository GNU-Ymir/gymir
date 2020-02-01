#pragma once

#include <ymir/utils/StringEnum.hh>
#include <vector>
#include <string>
#include <set>

namespace global {

    DECLARE_ENUM_HEADER (CoreNames, std::string,
			 ARRAY_ALLOC,
			 ARRAY_MODULE,
			 ASSERT_FUNC,
			 BINARY_OP_OVERRIDE,
			 CLASS_ALLOC,
			 CORE_MODULE,			 
			 DCOPY_OP_OVERRIDE,
			 DCOPY_TRAITS,
			 DUPLICATION_MODULE,
			 DUPL_ANY,
			 DUPL_SLICE,
			 EXCEPTION_MODULE,
			 EXCEPT_GET_VALUE,
			 EXCEPT_POP,
			 EXCEPT_PUSH,
			 INDEX_OP_OVERRIDE,
			 JMP_BUF_TYPE,
			 RETHROW,
			 RUN_MAIN,
			 RUN_MAIN_DEBUG,
			 SET_JMP,
			 THROW,
			 TYPE_IDS,
			 TYPE_INFO,
			 TYPE_INFO_EQUAL,
			 TYPE_INFO_MODULE,
    );
    
}
