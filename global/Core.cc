#include <ymir/global/Core.hh>

namespace global {
    DECLARE_ENUM_WITH_TYPE (CoreNames, std::string,
			    RUN_MAIN       = "_yrt_run_main",
			    RUN_MAIN_DEBUG = "_yrt_run_main_debug",
			    DUPL_SLICE     = "_yrt_dup_slice"
    );
}
