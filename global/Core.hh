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
    );
    
}
