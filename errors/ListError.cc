#include <ymir/errors/ListError.hh>

namespace Ymir {    

    const char*		ExternalError::ADDR_MIGHT_THROW			  = "a function pointer cannot be created from the %(y) function that could throw an exception"; 
    const char*		ExternalError::AFFECT_COMPILE_TIME		  = "assignment at compile time is prohibited";
    const char*		ExternalError::ALIAS_NO_EFFECT			  = "the creation of an alias has no effect on the left operand";
    const char*		ExternalError::ALLOC_ABSTRACT_CLASS		  = "the class %(y) is declared abstract";
    const char*		ExternalError::ASSERT_FAILED			  = "assertion failed : %(y)";
    const char*		ExternalError::BRANCHING_VALUE			  = "for branching value";
    const char*		ExternalError::BREAK_INSIDE_EXPR		  = "the break statement placed here will result in undefined behavior";
    const char*		ExternalError::BREAK_NO_LOOP			  = "the break statement must be placed inside a loop";
    const char*		ExternalError::CALL_RECURSION			  = "the limit of the number of call recursions at compilation time has been reached %(y)"; 
    const char*		ExternalError::CANDIDATE_ARE			  = "candidate % : %";
    const char*		ExternalError::CANNOT_BE_CTE			  = "cannot be compile time executed";
    const char*		ExternalError::CANNOT_OVERRIDE_AS_PRIVATE	  = "the private method %(y) cannot override the method of the ancestor";
    const char*		ExternalError::CANNOT_OVERRIDE_FINAL		  = "cannot override final method %(y)";
    const char*		ExternalError::CANNOT_OVERRIDE_NON_TRAIT_IN_IMPL  = "cannot override the class method %(y) inside a trait implementation";
    const char*		ExternalError::CANNOT_OVERRIDE_TRAIT_OUTSIDE_IMPL = "cannot override the trait method %(y) outside the implementation of the trait"; 
    const char*		ExternalError::CATCH_MULTIPLE_TIME		  = "type %(y) is caught multiple time";
    const char*		ExternalError::CATCH_OUT_OF_SCOPE		  = "catch used as an expression";
    const char*		ExternalError::COMPILATION_END			  = "";
    const char*		ExternalError::COMPILE_TIME_UNKNOWN		  = "the value of this expression is required, but could not be known at the time of compilation";
    const char*		ExternalError::CONDITIONAL_NON_TEMPLATE_CLASS	  = "conditional block in a non template class";
    const char*		ExternalError::CONDITIONAL_NON_TEMPLATE_TRAIT	  = "conditional block in a non template trait";
    const char*		ExternalError::CONFLICTING_DECLARATIONS		  = "these two declarations named %(y) shadow each other";
    const char*		ExternalError::CONFLICT_DECORATOR		  = "conflicting decorator";
    const char*		ExternalError::DECLARED_PROTECTION		  = "declared %(y) here";
    const char*		ExternalError::DECL_VARIADIC_FUNC		  = "C variadic functions can only be external";
    const char*		ExternalError::DECO_OUT_OF_CONTEXT		  = "the decorator %(y) is unusable in this context";
    const char*		ExternalError::DISCARD_CONST			  = "discard the constant qualifier is prohibited";
    const char*		ExternalError::DISCARD_CONST_LEVEL		  = "discard the constant qualifier is prohibited, left operand mutability level is %(y) but must be at most %(y)";
    const char*		ExternalError::DISCARD_CONST_LEVEL_TEMPLATE	  = "discard the constant qualifier is prohibited, left operand mutability level is %(y) but must be exactly %(y)";
    const char*		ExternalError::DOC_FILE_ERROR			  = "writing doc file : '%(y)'";
    const char*		ExternalError::DOLLAR_OUSIDE_CONTEXT		  = "dollar operator without context";
    const char*		ExternalError::DYNAMIC_CAST_FAILED		  = "dynamic cast failed : %";
    const char*		ExternalError::ENUM_EMPTY			  = "enum must have at least on value";
    const char*		ExternalError::EN_NO_VALUE			  = "%(y) was declared in an enumeration, but has no value";
    const char*		ExternalError::EXTERNAL_VAR_DECL_WITHOUT_TYPE	  = "global var %(y) declared external must have a type";
    const char*		ExternalError::EXTERNAL_VAR_WITH_VALUE		  = "global variable %(y) declared external cannot have a value";
    const char*		ExternalError::FAILURE_NO_THROW			  = "failure scope guard will never be triggered";
    const char*		ExternalError::FIELD_NO_DEFAULT			  = "the field %(y) of struct %(y) has no default value";
    const char*		ExternalError::FORGET_TOKEN			  = "did you forget '%(y)' ?";
    const char*		ExternalError::FORWARD_REFERENCE_VAR		  = "the type cannot be infered, as it depends on a forward reference";
    const char*		ExternalError::GLOBAL_VAR_DECL_WITHOUT_VALUE	  = "global var %(y) declared without value";
    const char*		ExternalError::GLOBAL_VAR_DEPENDENCY		  = "the global var %(y) cannot be initialized from the value of %(y)";
    const char*		ExternalError::IMMUTABLE_LVALUE			  = "left operand of type %(y) is immutable";
    const char*		ExternalError::IMPLICIT_ALIAS			  = "implicit alias of type %(y) is not allowed, it will implicitly discard constant qualifier";
    const char*         ExternalError::IMPLICIT_PURE                      = "implicit pure of type %(y) is not allowed, it will implicitly discard constant qualifier";
    const char*		ExternalError::IMPLICIT_OVERRIDE		  = "implicit override of method %(y) is not allowed";
    const char*		ExternalError::IMPLICIT_OVERRIDE_BY_TRAIT	  = "implicit override of method %(y) by trait implementation is not allowed";
    const char*		ExternalError::IMPLICIT_REFERENCE		  = "implicit referencing of type %(y) is not allowed";
    const char*		ExternalError::IMPL_NO_TRAIT			  = "impl statement must be followed by a trait, not %(y)";
    const char*		ExternalError::IMPOSSIBLE_EXTERN		  = "impossible extern declaration";
    const char*		ExternalError::INCOMPATIBLE_TOKENS		  = "incompatible tokens : [%(y)] and [%(y)]";
    const char*		ExternalError::INCOMPATIBLE_TYPES		  = "incompatible types %(y) and %(y)";
    const char*		ExternalError::INCOMPATIBLE_VALUES		  = "incompatible values";
    const char*		ExternalError::INCOMPLETE_TEMPLATE		  = "the template %(y) is incomplete due to previous errors, use -v to show them";
    const char*		ExternalError::INCOMPLETE_TYPE			  = "the type %(y) is not complete";
    const char*		ExternalError::INCOMPLETE_TYPE_CLASS		  = "the type %(y) is not complete due to previous errors";
    const char*		ExternalError::INFINITE_CONSTRUCTION_LOOP	  = "infinite construction loop";
    const char*		ExternalError::INHERIT_FINAL_CLASS		  = "the base class %(y) is marked as final";
    const char*		ExternalError::INHERIT_NO_CLASS			  = "the base of a class must be a class, not a %(y)";
    const char*		ExternalError::INSERT_NO_TABLE			  = "insertion of a symbol in an entity that cannot be a referent";
    const char*		ExternalError::INVALID_MACRO_EVAL		  = "invalid macro evaluation % in %";
    const char*		ExternalError::INVALID_MACRO_RULE		  = "%(y) is not a macro rule";
    const char*		ExternalError::IN_BLOCK_OPEN			  = "in block";
    const char*		ExternalError::IN_COMPILE_TIME_EXEC		  = "in compilation time execution";
    const char*		ExternalError::IN_IMPORT			  = "in importation";
    const char*		ExternalError::IN_MACRO_EXPANSION		  = "in macro expansion";
    const char*		ExternalError::IN_MATCH_DEF			  = "in pattern";
    const char*		ExternalError::IN_TEMPLATE_DEF			  = "in template specialization";
    const char*		ExternalError::IN_TRAIT_VALIDATION		  = "in trait validation";
    const char*		ExternalError::IS_TYPE				  = "cannot declare a variable named %(y), identifier is already used for a type";
    const char*         ExternalError::LOCAL_FIELD_OFFSET_OUT_CLASS	  = "local_field_offsets cannot be used outside class context";
    const char*	        ExternalError::LOCAL_TUPLEOF_OUT_CLASS		  = "local_tupleof cannot be used outside class context";
    const char*		ExternalError::LOCKED_CONTEXT			  = "the mutable element %(y) is locked in this iteration context";
    const char*		ExternalError::MACRO_MULT_NO_VAR		  = "macro multiple expression must be identified inside a var";
    const char*		ExternalError::MACRO_REST			  = "macro validation incomplete, there remains : [%(y)]";
    const char*		ExternalError::MAIN_FUNCTION_ONE_ARG		  = "main function takes at most one argument";
    const char*		ExternalError::MALFORMED_CHAR			  = "malformed literal, number of %(y) is %(y)";
    const char*		ExternalError::MALFORMED_PRAGMA			  = "malformed __pragma %(y)";
    const char*		ExternalError::MATCH_CALL			  = "match call only works on classes or structures, not %(y)";
    const char*		ExternalError::MATCH_FINAL_NO_DEFAULT		  = "final match has no default match case";
    const char*		ExternalError::MATCH_NO_DEFAULT			  = "match of type %(y) has no default match case";
    const char*		ExternalError::MATCH_PATTERN_CLASS		  = "the internal patterns of a class pattern matching must be named expressions";
    const char*		ExternalError::MISMATCH_ARITY			  = "mismatch arity in tuple destructor (%(y)) against (%(y))";
    const char*		ExternalError::MONITOR_NON_CLASS		  = "cannot synchronized unmonitored type %(y)";
    const char*		ExternalError::MOVE_ONLY_CLOSURE		  = "the move qualifier is only usable for lambda closure";
    const char*		ExternalError::MULTIPLE_CATCH			  = "%(y) is caught multiple times, this catcher will never be activated";
    const char*		ExternalError::MULTIPLE_DESTRUCTOR		  = "class can have only one destructor";
    const char*		ExternalError::MULTIPLE_FIELD_INIT		  = "the field %(y) is initialized multiple times";
    const char*		ExternalError::MUST_ESCAPE_CHAR			  = "an escape char must be used for '\n' '\r' or '\t'";
    const char*		ExternalError::MUTABLE_CONST_ITER		  = "an iterator cannot be mutable, if it is not a reference";
    const char*		ExternalError::MUTABLE_CONST_PARAM		  = "a parameter cannot be mutable, if it is not a reference";
    const char*		ExternalError::NEVER_USED			  = "the symbol %(y) declared but never used";
    const char*		ExternalError::NOTHING_TO_CATCH			  = "nothing to catch";
    const char*		ExternalError::NOT_ABSTRACT_NO_OVER		  = "the class %(y) is not abstract, but does not override the empty parent method %(y)";
    const char*		ExternalError::NOT_AN_ALIAS			  = "the type %(y) is not an aliasable type";
    const char*		ExternalError::NOT_A_CLASS			  = "the type %(y) is not a class type";
    const char*		ExternalError::NOT_A_LVALUE			  = "not a lvalue";
    const char*		ExternalError::NOT_A_STRUCT			  = "type %(y) is not a struct type";
    const char*		ExternalError::NOT_CATCH			  = "the exception %(y) might be thrown but is not caught";
    const char*		ExternalError::NOT_CTE_ITERABLE			  = "the type %(y) is not iterable at compile time";
    const char*		ExternalError::NOT_IMPL_TRAIT			  = "the type %(y) does not implement trait %(y)";
    const char*		ExternalError::NOT_ITERABLE			  = "the type %(y) is not iterable";
    const char*		ExternalError::NOT_ITERABLE_WITH		  = "the type %(y) is not iterable with %(y) vars";
    const char*		ExternalError::NOT_OVERRIDE			  = "the method %(y) marked as override does not override anything";
    const char*		ExternalError::NO_ALIAS_EXIST			  = "cannot alias type %(y)";
    const char*		ExternalError::NO_BODY_METHOD			  = "the method %(y) has no body";
    const char*		ExternalError::NO_COPY_EXIST			  = "no copy exists for type %(y)";
    const char*		ExternalError::NO_SIZE_FORWARD_REF		  = "the field has no size because of forward reference";
    const char*		ExternalError::NO_SUCH_FILE			  = "%(y) no such file, or permission denied";
    const char*		ExternalError::NO_SUPER_FOR_CLASS		  = "cannot construct super for class %(y), it has no ancestor";
    const char*		ExternalError::NULL_PTR				  = "try to unref a null pointer";
    const char*		ExternalError::OF				  = "of : %";
    const char*		ExternalError::OTHER_ERRORS			  = "there are other errors, use option -v to show them";
    const char*		ExternalError::OVERFLOW				  = "overflow capacity for type %(y) = %(y)";
    const char*		ExternalError::OVERFLOW_ARITY			  = "tuple access out of bound (%(y)), tuple arity is %(y)";
    const char*		ExternalError::OVERFLOW_ARRAY			  = "array access out of bound (%(y)), array size is %(y)";
    const char*		ExternalError::OVERRIDE_MISMATCH_PROTECTION	  = "the protection %(y) of the overriden method %(y) does not match with the definition of the ancestor class";
    const char*		ExternalError::OVERRIDE_PRIVATE			  = "cannot override private method %(y)";
    const char*		ExternalError::PACKED_AND_UNION			  = "structures could not be packed and union at the same time";
    const char*		ExternalError::PARAMETER_NAME			  = "for parameter % of %";
    const char*		ExternalError::PRIVATE_IN_THIS_CONTEXT		  = "%(B) : % is private within this context";
    const char*		ExternalError::PROTECTION_NO_IMPACT		  = "protection %(y) has no impact on %(y) declaration";
    const char*		ExternalError::REF_NO_EFFECT			  = "the creation of ref has no effect on the left operand";
    const char*		ExternalError::REF_NO_VALUE			  = "%(y) declared as a reference but not initialized";
    const char*		ExternalError::REF_RETURN_TYPE			  = "cannot return a reference type";
    const char*		ExternalError::REF_SELF				  = "cannot reference self";
    const char*		ExternalError::RESERVED_RULE_NAME		  = "%(y) rule name is reserved";
    const char*		ExternalError::RETHROW_NOT_MATCHING_PARENT	  = "rethrowing an exception %(y) that is not rethrowed by parent method is not allowed";
    const char*		ExternalError::RETURN_INSIDE_EXPR		  = "return here will result in an undefined behavior";
    const char*		ExternalError::RETURN_NO_FRAME			  = "it doesn't make sense to return outside of a function";
    const char*		ExternalError::SCOPE_OUT_OF_SCOPE		  = "scope guard used as expression";
    const char*		ExternalError::SHADOWING_DECL			  = "declaration of %(y) shadows another declaration";
    const char*		ExternalError::SPACE_EXTERN_C			  = "extern C cannot be namespaced";
    const char*		ExternalError::SPECIALISATION_WORK_TYPE_BOTH	  = "type match both";
    const char*		ExternalError::SPECIALISATION_WORK_WITH_BOTH	  = "%(y) called with {%(y)} work with both";
    const char*		ExternalError::SPECIALISATION_WORK_WITH_BOTH_PURE = "specialization work with both";
    const char*		ExternalError::SYNTAX_ERROR_AT			  = "[%] expected, when [%(y)] found";	
    const char*		ExternalError::SYNTAX_ERROR_AT_SIMPLE		  = "[%(y)] unexpected";	
    const char*		ExternalError::SYNTAX_ERROR_IF_ON_NON_TEMPLATE	  = "test on a non-template declaration";
    const char*		ExternalError::SYNTAX_ERROR_MISSING_TEMPL_PAR	  = "multiple ! arguments are not allowed";
    const char*		ExternalError::TEMPLATE_IN_TRAIT		  = "cannot declare template inside a trait";
    const char*		ExternalError::TEMPLATE_RECURSION		  = "limit of template recursion reached %(y)";
    const char*		ExternalError::TEMPLATE_REST			  = "template validation incomplete, rest : {%(y)}";
    const char*		ExternalError::TEMPLATE_TEST_FAILED		  = "the test of the template failed with {%(y)} specialization";
    const char*		ExternalError::TEMPLATE_VALUE_TRY		  = "when trying template value resolution";
    const char*		ExternalError::THROWS				  = "throws %(y)";
    const char*		ExternalError::THROWS_IN_LAMBDA			  = "a lambda function must be safe, but there are exceptions that are not caught";
    const char*		ExternalError::THROWS_NOT_DECLARED		  = "the function %(y) might throw an exception of type %(y), but that is not declared in its prototype";
    const char*		ExternalError::THROWS_NOT_USED			  = "the function %(y) prototype informs about a possible throw of an exception of type %(y), but this is not true";
    const char*		ExternalError::TRAIT_NO_METHOD			  = "the trait %(y) does not have any method %(y)";
    const char*		ExternalError::TYPE_NO_FIELD			  = "type %(y) has no fields";
    const char*		ExternalError::UFC_REWRITING			  = "when using uniform function call syntax";
    const char*		ExternalError::UNDEFINED_BIN_OP			  = "undefined operator %(y) for types %(y) and %(y)";
    const char*		ExternalError::UNDEFINED_BRACKETS_OP		  = "index operator is not defined for type %(y) and {%(y)}";
    const char*		ExternalError::UNDEFINED_CA			  = "the custom attribute %(y) does not exist in this context";
    const char*		ExternalError::UNDEFINED_CALL_OP		  = "the call operator is not defined for %(y) and {%(y)}";
    const char*		ExternalError::UNDEFINED_CAST_OP		  = "undefined cast operator for type %(y) to type %(y)";
    const char*		ExternalError::UNDEFINED_ESCAPE			  = "undefined escape sequence";
    const char*		ExternalError::UNDEFINED_FIELD_FOR		  = "undefined field %(y) for element %(y)";
    const char*		ExternalError::UNDEFINED_MACRO_OP		  = "undefined macro operator for element %(y)";
    const char*		ExternalError::UNDEFINED_SCOPE_GUARD		  = "undefined scope guard %(y)";
    const char*		ExternalError::UNDEFINED_SUB_PART_FOR		  = "undefined sub symbol %(y) for element %(y)";
    const char*		ExternalError::UNDEFINED_TEMPLATE_OP		  = "undefined template operator for %(y) and {%(y)}";
    const char*		ExternalError::UNDEFINED_UN_OP			  = "undefined operator %(y) for type %(y)";
    const char*		ExternalError::UNDEF_MACRO_EVAL			  = "undefined macro var %(y)";
    const char*		ExternalError::UNDEF_MACRO_EVAL_IN		  = "undefined macro var %(y) in {%}";
    const char*		ExternalError::UNDEF_TYPE			  = "undefined type %(y)";
    const char*		ExternalError::UNDEF_VAR			  = "undefined symbol %(y)";
    const char*		ExternalError::UNINIT_FIELD			  = "the field %(y) has no initial value";
    const char*		ExternalError::UNION_CST_MULT			  = "the construction of an union requires only one parameter";
    const char*		ExternalError::UNION_INIT_FIELD			  = "default field value of union has no sense";
    const char*		ExternalError::UNKNOWN_LAMBDA_TYPE		  = "lambda function is not complete, missing type for parameter %(y)";
    const char*		ExternalError::UNKNOWN_OPTION_NAME		  = "%(y) is not a valid option name";
    const char*		ExternalError::UNKNOWN_PRAGMA			  = "unknown __pragma %(y)";
    const char*		ExternalError::UNREACHBLE_STATEMENT		  = "unreachable statement";
    const char*		ExternalError::UNRESOLVED_TEMPLATE		  = "unresolved template";
    const char*		ExternalError::UNTERMINATED_SEQUENCE		  = "unterminated escape sequence";
    const char*		ExternalError::UNTRUSTED_CONTEXT		  = "trusted can only be used in std/core/etc modules";
    const char*		ExternalError::UNUSED_MATCH_CALL_OP		  = "unused patterns {%(y)} in struct destructuring";
    const char*		ExternalError::USELESS_CATCH			  = "useless catch of %(y), that cannot be thrown"; 
    const char*		ExternalError::USELESS_DECORATOR		  = "useless decorator";
    const char*		ExternalError::USELESS_EXPR			  = "expression has no effect";
    const char*		ExternalError::USE_AS_TYPE			  = "expression used as a type";
    const char*		ExternalError::USE_AS_VALUE			  = "type expression used as a value";
    const char*		ExternalError::USE_UNIT_FOR_VOID		  = "The block value cannot be void without a unit expression, maybe you forgot the ';'";
    const char*		ExternalError::VALIDATING			  = "when validating %(y)";
    const char*		ExternalError::VAR_DECL_IN_TRAIT		  = "field declaration inside trait is forbidden";
    const char*		ExternalError::VAR_DECL_WITHOUT_VALUE		  = "var declared without value, when necessary";
    const char*		ExternalError::VAR_DECL_WITH_NOTHING		  = "var declaration must at least have a type or a value";
    const char*		ExternalError::VOID_VALUE			  = "value cannot be void";
    const char*		ExternalError::VOID_VAR				  = "cannot declare a var of type void";
    const char*		ExternalError::VOID_VAR_VALUE			  = "cannot declare a var of type void from value %(y)";
    const char*		ExternalError::WRONG_IMPLEMENT			  = "the method %(y) was declared by trait %(y), not %(y)";
    const char*		ExternalError::WRONG_MODULE_NAME		  = "the module named %(y) must be placed in a file named %(y)";
}
