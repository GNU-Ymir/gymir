#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "target.h"
#include "tree.h"
#include "gimple-expr.h"
#include "diagnostic.h"
#include "opts.h"
#include "fold-const.h"
#include "gimplify.h"
#include "stor-layout.h"
#include "debug.h"
#include "convert.h"
#include "langhooks.h"
#include "langhooks-def.h"
#include "common/common-target.h"
#include <ymir/ymir1.hh>
#include <ymir/generic/types.hh>
#include <ymir/parsing/Parser.hh>
#include <ymir/global/State.hh>
#include <ymir/global/Core.hh>
#include <ymir/utils/Path.hh>

/* The context to be used for global declarations.  */
GTY(()) tree __global_context__;

/* Array of all global declarations to pass back to the middle-end.  */
GTY(()) vec <tree, va_gc> *__global_declarations__;

/** The context of the function being declared */
tree __current_function_ctx__ = NULL_TREE;

/* Language-dependent contents of a type.  */
 
struct GTY (()) lang_type
{
    char dummy;
};
 
/* Language-dependent contents of a decl.  */
 
struct GTY (()) lang_decl
{
    char dummy;
};
 
/* Language-dependent contents of an identifier.  This must include a
   tree_identifier.  */
 
struct GTY (()) lang_identifier
{
    struct tree_identifier common;
};
 
/* The resulting tree type.  */
 
union GTY ((desc ("TREE_CODE (&%h.generic) == IDENTIFIER_NODE"),
            chain_next ("CODE_CONTAINS_STRUCT (TREE_CODE (&%h.generic), "
                        "TS_COMMON) ? ((union lang_tree_node *) TREE_CHAIN "
                        "(&%h.generic)) : NULL"))) lang_tree_node
{
    union tree_node GTY ((tag ("0"), desc ("tree_node_structure (&%h)"))) generic;
    struct lang_identifier GTY ((tag ("1"))) identifier;
};
 
/* We don't use language_function.  */
 
struct GTY (()) language_function
{

    int dummy;
};
 
/* Language hooks.  */
 
static bool
ymir_langhook_init (void)
{        
    /* NOTE: Newer versions of GCC use only:
       build_common_tree_nodes (false);
       See Eugene's comment in the comments section. */
    build_common_tree_nodes (false);
 
    /* I don't know why this has to be done explicitly.  */
    void_list_node = build_tree_list (NULL_TREE, void_type_node);
 
    build_common_builtin_nodes ();
 
    return true;
}


static void
ymir_init_options (unsigned int argc ATTRIBUTE_UNUSED, cl_decoded_option * decoded_options ATTRIBUTE_UNUSED)
{
    global::State::instance ().setExecutable (decoded_options [0].arg);
    for (unsigned int i = 0 ; i < argc ; i++) {
	//const char * arg = decoded_options [i].arg;
	switch (decoded_options [i].opt_index) {
	case OPT_g :
	case OPT_ggdb :
	    global::State::instance ().activateDebug (true);
	    global::State::instance ().activateVersion (global::CoreNames::get (global::DEBUG_VERSION));
	    break;
	case OPT_v : global::State::instance ().activateVerbose (true); break;
	case OPT_nostdinc : global::State::instance ().activateStandalone (true); break;
	}
    }
    
    // Options OPT_l and cie...
}

static void
ymir_init_options_struct (gcc_options *opts) {
    opts->x_flag_exceptions = 0;
    opts->x_warn_return_type = 0;
    opts->x_warn_return_local_addr = 1;
  
    /* Avoid range issues for complex multiply and divide.  */
    opts->x_flag_complex_method = 2;

    /* Unlike C, there is no global 'errno' variable.  */
    opts->x_flag_errno_math = 0;
    opts->frontend_set_flag_errno_math = true;

    /* Keep in sync with existing -fbounds-check flag.  */
    //opts->x_flag_bounds_check = global.params.useArrayBounds;

    /* D says that signed overflow is precisely defined.  */
    opts->x_flag_wrapv = 1;  
}

static unsigned int
ymir_option_lang_mask (void) {
    return CL_YMIR;
}
    
//size_t, const char*, long long int, int, location_t, const cl_option_handlers*

static bool
ymir_langhook_handle_option (size_t scode, const char *arg, HOST_WIDE_INT value ATTRIBUTE_UNUSED,
                             int kind ATTRIBUTE_UNUSED, location_t loc ATTRIBUTE_UNUSED,
                             const struct cl_option_handlers *handlers ATTRIBUTE_UNUSED)
{
    opt_code code = (opt_code) scode;
    if (code == OPT_I) {
	// Add include dir
	global::State::instance ().addIncludeDir (arg);    
    } else if (code == OPT_iprefix) {
	global::State::instance ().setPrefix (arg);
    } else if (code == OPT_v) {
	return false;
	// set verbose
    } else if (code == OPT_nostdinc)  {
	// no std include
	global::State::instance ().activateStandalone (true);
    } else if (code == OPT_fdoc) {
	// dump documentation
	global::State::instance ().activateDocDumping (true);
    } else if (code == OPT_fdependency) {
	// print the dependency of the current file compilations
	global::State::instance ().activateDependencyDumping (true);
    } else if (code == OPT_funittest) {
	global::State::instance ().activateIncludeTesting (true);
    } else if (code == OPT_fversion_) {
	global::State::instance ().activateVersion (arg);
    } else if (code == OPT_imultilib) {
	// set multilib
    } else {
	println ("CODE : ", code, " ", OPT_MM, " ", OPT_M, " ", OPT_MMD, " ", OPT_MD);
	switch (code) {
	case OPT_MM :
	    global::State::instance ().activateDepSkip ();
	    // fall through
	case OPT_M:
	    global::State::instance ().activateDeps ();
	    break;
	case OPT_MMD:
	    global::State::instance ().activateDepSkip ();
	    // fall through
	case OPT_MD:
	    global::State::instance ().activateDeps ();
	    global::State::instance ().activateDepFilename ();
	    break;

	case OPT_MF:
	    global::State::instance ().setDepFilenameUser (arg);
	    break;

	case OPT_MP :
	    global::State::instance ().setDepPhony (true);
	    break;
	    
	case OPT_MQ :
	    global::State::instance ().addDepTarget (arg, true);
	    break;

	case OPT_MT:
	    global::State::instance ().addDepTarget (arg, false);
	    break;
	default :
	    return false;
	}       
    }
    
    return true;
}

static void
ymir_langhook_parse_file (void)
{
    ymir_parse_files (num_in_fnames, in_fnames);
}
 
static tree
ymir_langhook_type_for_mode (enum machine_mode mode, int unsignedp)
{
  
    if (mode == TYPE_MODE (float_type_node))
    return float_type_node;
  
    if (mode == TYPE_MODE (double_type_node))
    return double_type_node;
 
    if (mode == TYPE_MODE (intQI_type_node))
    return unsignedp ? unsigned_intQI_type_node : intQI_type_node;
    if (mode == TYPE_MODE (intHI_type_node))
    return unsignedp ? unsigned_intHI_type_node : intHI_type_node;
    if (mode == TYPE_MODE (intSI_type_node))
    return unsignedp ? unsigned_intSI_type_node : intSI_type_node;
    if (mode == TYPE_MODE (intDI_type_node))
    return unsignedp ? unsigned_intDI_type_node : intDI_type_node;
    if (mode == TYPE_MODE (intTI_type_node))
    return unsignedp ? unsigned_intTI_type_node : intTI_type_node;
 
    if (mode == TYPE_MODE (integer_type_node))
    return unsignedp ? unsigned_type_node : integer_type_node;
 
    if (mode == TYPE_MODE (long_integer_type_node))
    return unsignedp ? long_unsigned_type_node : long_integer_type_node;
 
    if (mode == TYPE_MODE (long_long_integer_type_node))
    return unsignedp ? long_long_unsigned_type_node
	: long_long_integer_type_node;
 
    if (COMPLEX_MODE_P (mode))
    {
	if (mode == TYPE_MODE (complex_float_type_node))
	return complex_float_type_node;
	if (mode == TYPE_MODE (complex_double_type_node))
	return complex_double_type_node;
	if (mode == TYPE_MODE (complex_long_double_type_node))
	return complex_long_double_type_node;
	if (mode == TYPE_MODE (complex_integer_type_node) && !unsignedp)
	return complex_integer_type_node;
    }
 
    /* gcc_unreachable */
    return NULL;
}
 
static tree
ymir_langhook_type_for_size (unsigned int bits,
                             int unsignedp)
{
    if (bits <= TYPE_PRECISION (byte_type_node))
    return unsignedp ? ubyte_type_node : byte_type_node;

    if (bits <= TYPE_PRECISION (short_type_node))
    return unsignedp ? ushort_type_node : short_type_node;

    if (bits <= TYPE_PRECISION (int_type_node))
    return unsignedp ? uint_type_node : int_type_node;
    
    if (bits <= TYPE_PRECISION (long_type_node))
    return unsignedp ? ulong_type_node : long_type_node;
    
    return NULL;
}

#include <execinfo.h>
#include <iostream>

std::string getStackTrace () {
    static bool _in_trace_ = false;
    if (!_in_trace_) {
	_in_trace_ = true;
	void *trace[100];
	char **messages = (char **) NULL;

	auto trace_size = backtrace(trace, 100);
	messages = backtrace_symbols(trace, trace_size);
	/* skip first stack frame (points here) */
	std::string ss;

	ss = ss + "[bt] Execution path:\n";
	for (int i=2; i<trace_size; ++i)
	{
	    ss = ss + "[bt] #";
	    ss = ss + std::to_string (i - 1) + "\n";
	    std::string msg = messages [i];
	    ss = ss + "\t" + msg + "\n";
	}
	_in_trace_ = false;
	return ss;
    } else {
	return "";
    }
}

/* Record a builtin function.  We just ignore builtin functions.  */
 
static tree
ymir_langhook_builtin_function (tree decl)
{
    return decl;
}
 
static bool
ymir_langhook_global_bindings_p (void)
{
    return (__current_function_ctx__ == NULL_TREE);
}

tree ymir_get_global_context(void)
{
    if (!__global_context__) {
	__global_context__ = build_translation_unit_decl (NULL_TREE);
    }

    return __global_context__;
}


static tree
ymir_langhook_pushdecl (tree decl ATTRIBUTE_UNUSED)
{
    gcc_unreachable ();
    return decl;
}
 
static tree
ymir_langhook_getdecls (void)
{
    return NULL;
}

#undef LANG_HOOKS_NAME
#undef LANG_HOOKS_INIT
#undef LANG_HOOKS_INIT_OPTIONS
#undef LANG_HOOKS_INIT_OPTIONS_STRUCT
#undef LANG_HOOKS_PARSE_FILE
#undef LANG_HOOKS_TYPE_FOR_MODE
#undef LANG_HOOKS_TYPE_FOR_SIZE
#undef LANG_HOOKS_BUILTIN_FUNCTION
#undef LANG_HOOKS_GLOBAL_BINDINGS_P
#undef LANG_HOOKS_PUSHDECL
#undef LANG_HOOKS_GETDECLS
#undef LANG_HOOKS_HANDLE_OPTION
#undef LANG_HOOKS_OPTION_LANG_MASK

#define LANG_HOOKS_NAME "Ymir" 
#define LANG_HOOKS_INIT ymir_langhook_init 
#define LANG_HOOKS_INIT_OPTIONS	 ymir_init_options
#define LANG_HOOKS_INIT_OPTIONS_STRUCT	    ymir_init_options_struct
#define LANG_HOOKS_OPTION_LANG_MASK ymir_option_lang_mask
#define LANG_HOOKS_HANDLE_OPTION ymir_langhook_handle_option
#define LANG_HOOKS_PARSE_FILE ymir_langhook_parse_file
#define LANG_HOOKS_TYPE_FOR_MODE ymir_langhook_type_for_mode
#define LANG_HOOKS_TYPE_FOR_SIZE ymir_langhook_type_for_size
#define LANG_HOOKS_BUILTIN_FUNCTION ymir_langhook_builtin_function
#define LANG_HOOKS_GLOBAL_BINDINGS_P ymir_langhook_global_bindings_p
#define LANG_HOOKS_PUSHDECL ymir_langhook_pushdecl
#define LANG_HOOKS_GETDECLS ymir_langhook_getdecls


struct lang_hooks lang_hooks = LANG_HOOKS_INITIALIZER;
 
#include "gt-ymir-ymir1.h"
#include "gtype-ymir.h"
