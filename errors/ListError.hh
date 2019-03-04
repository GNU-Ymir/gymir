#pragma once

#include <ymir/utils/StringEnum.hh>

namespace Ymir {

    /**
     * \enum ErrorCode
     * This enum is used for exception handling
     */
    enum class ErrorCode : int {
	INTERNAL = 0, // internal error, due to compiler problem
	EXTERNAL = 1, // external error, due to source code error
	FATAL = 2, // fatal external error
	FAIL = 3 // fail error, too many errors for example, the compilation must stop
    };    

    /**
       \enum ErrorType
       This enumaration lists all the error types 
       We cannot use C++ enum, as it does not allows to have string values
    */
    DECLARE_ENUM_HEADER (ExternalError, std::string,			         
			 AFFECT_COMPILE_TIME,
			 BORROWED_HERE,
			 BREAK_INSIDE_EXPR,
			 BREAK_NO_LOOP,
			 CANDIDATE_ARE,
			 COMPILATION_END,
			 COMPILE_TIME_UNKNOWN,
			 CONFLICT_DECORATOR,
			 DECO_OUT_OF_CONTEXT,
			 DISCARD_CONST,
			 DISCARD_CONST_LEVEL,
			 DISCARD_LOCALITY,
			 DYNAMIC_CAST_FAILED,
			 FORWARD_REFERENCE_VAR,
			 IMMUTABLE_LVALUE,
			 IMPLICIT_COPY,
			 IMPLICIT_REFERENCE,
			 INCOMPATIBLE_TYPES,
			 INSERT_NO_TABLE,
			 MALFORMED_CHAR,
			 MUTABLE_CONST_PARAM,
			 MUTABLE_CONST_RETURN,
			 NEVER_USED,
			 NOT_A_LVALUE,
			 NO_ALIAS_EXIST,
			 NO_COPY_EXIST,
			 NO_SIZE_FORWARD_REF,
			 NO_SUCH_FILE,
			 NULL_PTR,
			 OVERFLOW,
			 OVERFLOW_ARITY,			 
			 PACKED_AND_UNION,
			 REF_NOT_TYPE,
			 REF_NO_EFFECT,
			 REF_NO_VALUE,
			 RETURN_INSIDE_EXPR,
			 RETURN_LOCAL_REFERENCE,
			 SHADOWING_DECL,
			 SPECIALISATION_WOTK_WITH_BOTH,
			 STATIC_ARRAY_REF,
			 SYNTAX_ERROR_AT,
			 SYNTAX_ERROR_AT_SIMPLE,
			 SYNTAX_ERROR_IF_ON_NON_TEMPLATE,
			 SYNTAX_ERROR_MISSING_TEMPL_PAR,
			 UNDEFINED_BIN_OP,
			 UNDEFINED_BRACKETS_OP,
			 UNDEFINED_CA,
			 UNDEFINED_CALL_OP,
			 UNDEFINED_ESCAPE,
			 UNDEFINED_SUB_PART,
			 UNDEFINED_SUB_PART_FOR,
			 UNDEFINED_UN_OP,
			 UNDEF_TYPE,
			 UNDEF_VAR,
			 UNREACHBLE_STATEMENT,
			 UNTERMINATED_SEQUENCE,
			 USELESS_DECORATOR,
			 USE_AS_TYPE, 
			 VAR_DECL_WITH_NOTHING,
			 VOID_VAR,	
			 WRONG_MODULE_NAME
    );
    
    // 	    SYNTAX_ERROR_FOR = "%%% unexpected when analysing statement %%%",
    // 	    SYNTAX_ERROR_SIMPLE = "%%% unexpected",
    // 	    LAST_ERROR
    // 	    };
        
}
