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
    struct ExternalError {
	static const char*	ADDR_MIGHT_THROW;
	static const char*	AFFECT_COMPILE_TIME;
	static const char*	ALIAS_NO_EFFECT;
	static const char*	ALLOC_ABSTRACT_CLASS;
	static const char*	ASSERT_FAILED;
	static const char*	BRANCHING_VALUE;
	static const char*	BREAK_INSIDE_EXPR;
	static const char*	BREAK_NO_LOOP;
	static const char*	CALL_RECURSION;
	static const char*	CANDIDATE_ARE;
	static const char*	CANNOT_BE_CTE;
	static const char*	CANNOT_OVERRIDE_AS_PRIVATE;
	static const char*	CANNOT_OVERRIDE_FINAL;
	static const char*	CANNOT_OVERRIDE_NON_TRAIT_IN_IMPL;
	static const char*	CANNOT_OVERRIDE_TRAIT_OUTSIDE_IMPL; 
	static const char*	CATCH_MULTIPLE_TIME;
	static const char*	CATCH_OUT_OF_SCOPE;
	static const char*	COMPILATION_END;
	static const char*	COMPILE_TIME_UNKNOWN;
	static const char*	CONDITIONAL_NON_TEMPLATE_CLASS;
	static const char*	CONDITIONAL_NON_TEMPLATE_TRAIT;
	static const char*	CONFLICTING_DECLARATIONS;
	static const char*	CONFLICT_DECORATOR;
	static const char*	DECLARED_PROTECTION;
	static const char*	DECL_VARIADIC_FUNC;
	static const char*	DECO_OUT_OF_CONTEXT;
	static const char*	DISCARD_CONST;
	static const char*	DISCARD_CONST_LEVEL;
	static const char*	DISCARD_CONST_LEVEL_TEMPLATE;
	static const char*	DOC_FILE_ERROR;
	static const char*	DOLLAR_OUSIDE_CONTEXT;
	static const char*	DYNAMIC_CAST_FAILED;
	static const char*	ENUM_EMPTY;
	static const char*	EN_NO_VALUE;
	static const char*	EXTERNAL_VAR_DECL_WITHOUT_TYPE;
	static const char*	EXTERNAL_VAR_WITH_VALUE;
	static const char*	FAILURE_NO_THROW;
	static const char*	FIELD_NO_DEFAULT;
	static const char*	FORGET_TOKEN;
	static const char*	FORWARD_REFERENCE_VAR;
	static const char*	GLOBAL_VAR_DECL_WITHOUT_VALUE;
	static const char*	GLOBAL_VAR_DEPENDENCY; 
	static const char*	IMMUTABLE_LVALUE;
	static const char*	IMPLICIT_ALIAS;
	static const char*      IMPLICIT_PURE;
	static const char*	IMPLICIT_OVERRIDE;
	static const char*	IMPLICIT_OVERRIDE_BY_TRAIT;
	static const char*	IMPLICIT_REFERENCE;
	static const char*	IMPL_NO_TRAIT;
	static const char*	IMPOSSIBLE_EXTERN;
	static const char*	INCOMPATIBLE_TOKENS;
	static const char*	INCOMPATIBLE_TYPES_RANGE;
	static const char*	INCOMPATIBLE_TYPES;
	static const char*	INCOMPATIBLE_VALUES;
	static const char*	INCOMPLETE_TEMPLATE;
	static const char*	INCOMPLETE_TYPE;
	static const char*	INCOMPLETE_TYPE_CLASS;
	static const char*	INFINITE_CONSTRUCTION_LOOP;
	static const char*	INHERIT_FINAL_CLASS;
	static const char*	INHERIT_NO_CLASS;
	static const char*      INHERIT_RECURSIVE;
	static const char*	INSERT_NO_TABLE;
	static const char*	INVALID_MACRO_EVAL;
	static const char*	INVALID_MACRO_RULE;
	static const char*	IN_BLOCK_OPEN;
	static const char*	IN_COMPILE_TIME_EXEC;
	static const char*	IN_IMPORT;
	static const char*	IN_MACRO_EXPANSION;
	static const char*	IN_MATCH_DEF;
	static const char*	IN_TEMPLATE_DEF;
	static const char*	IN_TRAIT_VALIDATION;
	static const char*	IS_TYPE;
	static const char*	LOCAL_FIELD_OFFSET_OUT_CLASS;
	static const char*	LOCAL_TUPLEOF_OUT_CLASS;
	static const char*	LOCKED_CONTEXT;
	static const char*	MACRO_MULT_NO_VAR;
	static const char*	MACRO_REST;
	static const char*	MAIN_FUNCTION_ONE_ARG;
	static const char*	MALFORMED_CHAR;
	static const char*	MALFORMED_PRAGMA; 
	static const char*	MATCH_CALL;
	static const char*	MATCH_FINAL_NO_DEFAULT;
	static const char*	MATCH_NO_DEFAULT;
	static const char*	MATCH_PATTERN_CLASS;
	static const char*	MISMATCH_ARITY;
	static const char*	MONITOR_NON_CLASS;
	static const char*	MOVE_ONLY_CLOSURE;
	static const char*	MULTIPLE_CATCH;
	static const char*	MULTIPLE_DESTRUCTOR;
	static const char*	MULTIPLE_FIELD_INIT;
	static const char*	MUST_ESCAPE_CHAR; 
	static const char*	MUTABLE_CONST_ITER;
	static const char*	MUTABLE_CONST_PARAM;
	static const char*	NEVER_USED;
	static const char*	NOTHING_TO_CATCH;
	static const char*	NOT_ABSTRACT_NO_OVER;
	static const char*	NOT_AN_ALIAS;
	static const char*	NOT_A_CLASS;
	static const char*	NOT_A_LVALUE;
	static const char*	NOT_A_STRUCT;
	static const char*	NOT_CATCH;
	static const char*	NOT_CTE_ITERABLE;
	static const char*	NOT_IMPL_TRAIT;
	static const char*	NOT_ITERABLE;
	static const char*	NOT_ITERABLE_WITH;
	static const char*	NOT_OVERRIDE;
	static const char*	NO_ALIAS_EXIST;
	static const char*	NO_BODY_METHOD;
	static const char*	NO_COPY_EXIST;
	static const char*	NO_SIZE_FORWARD_REF;
	static const char*	NO_SUCH_FILE;
	static const char*	NO_SUPER_FOR_CLASS;
	static const char*	NULL_PTR;
	static const char*	OF; 
	static const char*	OTHER_ERRORS;
	static const char*	OVERFLOW_;
	static const char*	OVERFLOW_ARITY;
	static const char*	OVERFLOW_ARRAY;
	static const char*	OVERRIDE_MISMATCH_PROTECTION;
	static const char*	OVERRIDE_PRIVATE;
	static const char*	PACKED_AND_UNION;
	static const char*	PARAMETER_NAME;
	static const char*	NO_PARAMETER_NAMED;
	static const char*	PRIVATE_IN_THIS_CONTEXT;
	static const char*	PROTECTION_NO_IMPACT;
	static const char*	REF_NO_EFFECT;
	static const char*	REF_NO_VALUE;
	static const char*	REF_RETURN_TYPE;
	static const char*	REF_SELF;
	static const char*	RESERVED_RULE_NAME;
	static const char*	RETHROW_NOT_MATCHING_PARENT;
	static const char*	RETURN_INSIDE_EXPR;
	static const char*	RETURN_NO_FRAME;
	static const char*	SCOPE_OUT_OF_SCOPE;
	static const char*	SHADOWING_DECL;
	static const char*	SPACE_EXTERN_C;
	static const char*	SPECIALISATION_WORK_TYPE_BOTH;
	static const char*	SPECIALISATION_WORK_WITH_BOTH;
	static const char*	SPECIALISATION_WORK_WITH_BOTH_PURE;
	static const char*	SYNTAX_ERROR_AT;
	static const char*	SYNTAX_ERROR_AT_SIMPLE;
	static const char*	SYNTAX_ERROR_IF_ON_NON_TEMPLATE;
	static const char*	SYNTAX_ERROR_MISSING_TEMPL_PAR;
	static const char*	TEMPLATE_IN_TRAIT;
	static const char*	TEMPLATE_RECURSION;
	static const char*	TEMPLATE_REST;
	static const char*	TEMPLATE_TEST_FAILED;
	static const char*	TEMPLATE_VALUE_TRY;
	static const char*	THROWS;
	static const char*	THROWS_IN_LAMBDA;
	static const char*	THROWS_NOT_DECLARED;
	static const char*	THROWS_NOT_USED;
	static const char*	TRAIT_NO_METHOD;
	static const char*	TYPE_NO_FIELD;
	static const char*	UFC_REWRITING;
	static const char*	UNDEFINED_BIN_OP;
	static const char*	UNDEFINED_BRACKETS_OP;
	static const char*	UNDEFINED_CA;
	static const char*	UNDEFINED_CALL_OP;
	static const char*	UNDEFINED_CAST_OP;
	static const char*	UNDEFINED_ESCAPE;
	static const char*	UNDEFINED_FIELD_FOR;
	static const char*	UNDEFINED_MACRO_OP;
	static const char*	UNDEFINED_SCOPE_GUARD;
	static const char*	UNDEFINED_SUB_PART_FOR;
	static const char*	UNDEFINED_TEMPLATE_OP;
	static const char*	UNDEFINED_UN_OP;
	static const char*	UNDEF_MACRO_EVAL;
	static const char*	UNDEF_MACRO_EVAL_IN;
	static const char*	UNDEF_TYPE;
	static const char*	UNDEF_VAR;
	static const char*	UNINIT_FIELD;
	static const char*	UNION_CST_MULT;
	static const char*	UNION_INIT_FIELD;
	static const char*	UNKNOWN_LAMBDA_TYPE;
	static const char*	UNKNOWN_OPTION_NAME;
	static const char*	UNREACHBLE_STATEMENT;
	static const char*	UNRESOLVED_TEMPLATE;
	static const char*	UNTERMINATED_SEQUENCE;
	static const char*	UNTRUSTED_CONTEXT;
	static const char*	UNUSED_MATCH_CALL_OP;
	static const char*	USELESS_CATCH;
	static const char*	USELESS_DECORATOR;
	static const char*	USELESS_EXPR;
	static const char*	USE_AS_TYPE;
	static const char*	USE_AS_VALUE;
	static const char*	USE_UNIT_FOR_VOID;
	static const char*	VALIDATING;
	static const char*	VAR_DECL_IN_TRAIT;
	static const char*	VAR_DECL_WITHOUT_VALUE;
	static const char*	VAR_DECL_WITH_NOTHING;
	static const char*	VOID_VALUE;
	static const char*	VOID_VAR;
	static const char*	VOID_VAR_VALUE;
	static const char*	WRONG_IMPLEMENT;
	static const char*	WRONG_MODULE_NAME;
	static const char*	UNKNOWN_PRAGMA;
	static const char*      SCOPE_MUST_BE_SAFE;
    };
    
        
}
