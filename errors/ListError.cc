#include <ymir/errors/ListError.hh>

namespace Ymir {
    DECLARE_ENUM_WITH_TYPE (ExternalError, std::string,			         
			    SYNTAX_ERROR_AT = "[%] expected, when %%% found", // syntax error a specific location
    );
}
