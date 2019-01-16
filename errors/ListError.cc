#include <ymir/errors/ListError.hh>

namespace Ymir {    
    DECLARE_ENUM_WITH_TYPE (ExternalError, std::string,			         
			    SYNTAX_ERROR_AT = "[%] expected, when %(y) found", // syntax error a specific location
			    SYNTAX_ERROR_AT_SIMPLE = "%(y) unexpected", // syntax error a specific location
			    SYNTAX_ERROR_MISSING_TEMPL_PAR = "multiple ! arguments are not allowed",
			    SYNTAX_ERROR_IF_ON_NON_TEMPLATE = "test on non template declaration",
			    NO_SUCH_FILE = "%(y) no such file, or permission denied",
			    COMPILATION_END = "",
			    DYNAMIC_CAST_FAILED = "dynamic cast failed : %",
			    NULL_PTR = "try to unref a null pointer",
			    INSERT_NO_TABLE = "insertion of a symbol in an entity that cannot be a referent"
    );    
}
