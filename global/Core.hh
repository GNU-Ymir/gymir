#pragma once

#include <ymir/utils/StringEnum.hh>
#include <vector>
#include <string>
#include <set>

namespace global {

    DECLARE_ENUM_HEADER (CoreNames, std::string,
			 ARRAY_ALLOC,
			 CLASS_ALLOC,
			 DUPL_ANY,
			 DUPL_SLICE,
			 EXCEPT_GET_VALUE,
			 EXCEPT_POP,
			 EXCEPT_PUSH,
			 JMP_BUF_TYPE,
			 RETHROW,
			 RUN_MAIN,
			 RUN_MAIN_DEBUG,
			 SET_JMP,
			 THROW,
			 TYPE_INFO_EQUAL,
			 TYPE_INFO,
			 TYPE_IDS,
			 DCOPY_OP_OVERRIDE,
			 DCOPY_TRAITS,
			 BINARY_OP_OVERRIDE,
			 CORE_MODULE,
			 DUPLICATION_MODULE,
			 TYPE_INFO_MODULE
    );
    
}
