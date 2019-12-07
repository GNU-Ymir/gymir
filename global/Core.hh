#pragma once

#include <ymir/utils/StringEnum.hh>
#include <vector>
#include <string>
#include <set>

namespace global {

    DECLARE_ENUM_HEADER (CoreNames, std::string,
			 RUN_MAIN,
			 RUN_MAIN_DEBUG,
			 DUPL_SLICE,
			 ARRAY_ALLOC,
			 DUPL_ANY,
			 THROW
    );
    
}
