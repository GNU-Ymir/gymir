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
			    INSERT_NO_TABLE = "insertion of a symbol in an entity that cannot be a referent",
			    WRONG_MODULE_NAME = "module named %(y) must be placed in file named %(y)",
			    SHADOWING_DECL = "declaration of %(y) shadow another declaration",
			    UNDEFINED_CA = "custom attribute %(y), does not exist in this context",
			    PACKED_AND_UNION = "structure could not be packed and union at the same time",
			    VAR_DECL_WITH_NOTHING = "var declaration must at least have a type or a value",
			    INCOMPATIBLE_TYPES = "incompatible types %(y) and %(y)",
			    UNDEF_TYPE = "undefined type %(y)",
			    UNREACHBLE_STATEMENT = "unreachable statement",
			    OVERFLOW = "overflow capacity for type %(y)",
			    UNDEFINED_BIN_OP = "undefined operator %(y) for types %(y) and %(y)",
			    UNDEF_VAR = "undefined symbol %(y)",
			    NEVER_USED = "symbol %(y) declared but never used",
			    VOID_VAR = "cannot declare var of type void",
			    NOT_A_LVALUE = "not a lvalue",
			    UNDEFINED_ESCAPE = "undefined escape sequence",
			    UNTERMINATED_SEQUENCE = "unterminated escape sequence",
			    MALFORMED_CHAR = "malformed literal, number of %(y) is %(y)"
    );    
}
