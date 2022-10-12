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
#include <ymir/tree/Tree.hh>

// We use the dlang target hooks to get the target versions
#include <ymir/../d/d-target.h>

/* The context to be used for global declarations.  */
GTY(()) tree __global_context__;

/* Array of all global declarations to pass back to the middle-end.  */
GTY(()) vec <tree, va_gc> *__global_declarations__;

/** The context of the function being declared */
tree __current_function_ctx__ = NULL_TREE;

/** Ymir types */
tree y_global_trees[YTI_MAX];

using namespace global;

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


std::map <std::string, std::string> d_version_to_y_version = {
  { "Windows", CoreNames::get (WINDOWS_VERSION) },
  { "Win32", CoreNames::get (WIN32_VERSION) },
  { "Win64", CoreNames::get (WIN64_VERSION) },
  { "linux", CoreNames::get (LINUX_VERSION) },
  { "OSX", CoreNames::get (OSX_VERSION) },
  { "iOS", CoreNames::get (IOS_VERSION) },
  { "TVOS", CoreNames::get (TVOS_VERSION) },
  { "WatchOS", CoreNames::get (WATCHOS_VERSION) },
  { "FreeBSD", CoreNames::get (FREEBSD_VERSION) },
  { "OpenBSD", CoreNames::get (OPENBSD_VERSION) },
  { "NetBSD", CoreNames::get (NETBSD_VERSION) },
  { "DragonFlyBSD", CoreNames::get (DRAGONFLYBSD_VERSION) },
  { "BSD", CoreNames::get (BSD_VERSION) },
  { "Solaris", CoreNames::get (SOLARIS_VERSION) },
  { "Posix", CoreNames::get (POSIX_VERSION) },
  { "AIS", CoreNames::get (AIS_VERSION) },
  { "Haiku", CoreNames::get (HAIKU_VERSION) },
  { "SkyOS", CoreNames::get (SKYOS_VERSION) },
  { "Hurd", CoreNames::get (GNU_HURD_VERSION) },
  { "Android", CoreNames::get (ANDROID_VERSION) },
  { "Emscripten", CoreNames::get (EMSCRIPTEN_VERSION) },
  { "Playstation", CoreNames::get (PLAYSTATION_VERSION) },
  { "Playstation4", CoreNames::get (PLAYSTATION4_VERSION) },
  { "Cygwin", CoreNames::get (CYGWIN_VERSION) },
  { "MinGW", CoreNames::get (MINGW_VERSION) },
  { "FreeStanding", CoreNames::get (FREESTANDING_VERSION) },
  { "X86", CoreNames::get (X86_VERSION) },
  { "X86_64", CoreNames::get (X86_64_VERSION) },
  { "ARM", CoreNames::get (ARM_VERSION) },
  { "ARM_Thumb", CoreNames::get (ARM_THUMB_VERSION) },
  { "ARM_SoftFloat", CoreNames::get (ARM_SOFTFLOAT_VERSION) },
  { "ARM_HardFloat", CoreNames::get (ARM_HARDFLOAT_VERSION) },
  { "AArch64", CoreNames::get (AARCH64_VERSION) },
  { "AVR", CoreNames::get (AVR_VERSION) },
  { "Epiphany", CoreNames::get (EPIPHANY_VERSION) },
  { "PPC", CoreNames::get (PPC_VERSION) },
  { "PPC_SoftFloat", CoreNames::get (PPC_SOFTFLOAT_VERSION) },
  { "PPC_HardFloat", CoreNames::get (PPC_HARDFLOAT_VERSION) },
  { "PPC64", CoreNames::get (PPC64_VERSION) },
  { "IA64", CoreNames::get (IA64_VERSION) },
  { "MIPS32", CoreNames::get (MIPS32_VERSION) },
  { "MIPS64", CoreNames::get (MIPS64_VERSION) },
  { "MIPS_O32", CoreNames::get (MIPS_O32_VERSION) },
  { "MIPS_O64", CoreNames::get (MIPS_O64_VERSION) },
  { "MIPS_N32", CoreNames::get (MIPS_N32_VERSION) },
  { "MIPS_N64", CoreNames::get (MIPS_N64_VERSION) },
  { "MIPS_EABI", CoreNames::get (MIPS_EABI_VERSION) },
  { "MIPS_SoftFloat", CoreNames::get (MIPS_SOFTFLOAT_VERSION) },
  { "MIPS_HardFloat", CoreNames::get (MIPS_HARDFLOAT_VERSION) },
  { "MSP430", CoreNames::get (MSP430_VERSION) },
  { "NVPTX", CoreNames::get (NVPTX_VERSION) },
  { "NVPTX64", CoreNames::get (NVPTX64_VERSION) },
  { "RISCV32", CoreNames::get (RISCV32_VERSION) },
  { "RISCV64", CoreNames::get (RISCV64_VERSION) },
  { "SPARC", CoreNames::get (SPARC_VERSION) },
  { "SPARC_V8Plus", CoreNames::get (SPARC_V8PLUS_VERSION) },
  { "SPARC_SoftFloat", CoreNames::get (SPARC_SOFTFLOAT_VERSION) },
  { "SPARC_HardFloat", CoreNames::get (SPARC_HARDFLOAT_VERSION) },
  { "SPARC64", CoreNames::get (SPARC64_VERSION) },
  { "S390", CoreNames::get (S390_VERSION) },
  { "SystemZ", CoreNames::get (SYSTEMZ_VERSION) },
  { "HPPA", CoreNames::get (HPPA_VERSION) },
  { "HPPA64", CoreNames::get (HPPA64_VERSION) },
  { "SH", CoreNames::get (SH_VERSION) },
  { "WebAssembly", CoreNames::get (WEBASSEMBLY_VERSION) },
  { "WASI", CoreNames::get (WASI_VERSION) },
  { "Alpha_SoftFloat", CoreNames::get (ALPHA_SOFTFLOAT_VERSION) },
  { "Alpha_HardFloat", CoreNames::get (ALPHA_HARDFLOAT_VERSION) },
  { "ELFv1", CoreNames::get (ELFV1_VERSION) },
  { "ELFv2", CoreNames::get (ELFV2_VERSION) },
  { "D_SoftFloat", CoreNames::get (Y_SOFTFLOAT_VERSION) },
  { "D_HardFloat", CoreNames::get (Y_HARDFLOAT_VERSION) },
  { "CRuntime_Glibc", CoreNames::get (GLIBC_VERSION) },
};


/**
 * Dlang target callback for target information in __traits(getTargetInfo)
 */
void d_add_target_info_handlers (const d_target_info_spec * handlers ATTRIBUTE_UNUSED) {
  // we have no use to that for the moment
}

/**
 * Thanks to dlang we can use their version system for our version system !
 */
void d_add_builtin_version (const char* v) {
  auto it = d_version_to_y_version.find (std::string (v));
  if (it != d_version_to_y_version.end ()) {
    State::instance ().activateVersion (it-> second);
  }
}


static void
ymir_init_builtins () {
  y_bool_type = make_unsigned_type (1);
  TREE_SET_CODE (y_bool_type, BOOLEAN_TYPE);
  
  y_u8_type = make_unsigned_type (8);
  y_i8_type = make_signed_type (8);
  
  y_u16_type = make_unsigned_type (16);
  y_i16_type = make_signed_type (16);

  y_u32_type = make_unsigned_type (32);
  y_i32_type = make_signed_type (32);

  y_u64_type = make_unsigned_type (64);
  y_i64_type = make_signed_type (64);
    
  {
    machine_mode type_mode = TYPE_MODE (size_type_node);
    size_type_node = lang_hooks.types.type_for_mode (type_mode, 1);
    y_usize_type = lang_hooks.types.type_for_mode (type_mode, 1);
    y_isize_type = lang_hooks.types.type_for_mode (type_mode, 0);    
  }

  y_c8_type = make_unsigned_type (8);
  TYPE_STRING_FLAG (y_c8_type) = 1;

  y_c16_type = make_unsigned_type (16);
  TYPE_STRING_FLAG (y_c16_type) = 1;
  
  y_c32_type = make_unsigned_type (32);
  TYPE_STRING_FLAG (y_c32_type) = 1;

  y_f32_type = build_distinct_type_copy (float_type_node);
  y_f64_type = build_distinct_type_copy (double_type_node);
}

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
 
    ymir_init_builtins ();
    build_common_builtin_nodes ();

    using_eh_for_cleanups ();
	
    global::State::instance ().activateVersion (global::CoreNames::get (global::GNU_VERSION));
    global::State::instance ().activateVersion (global::CoreNames::get (global::YMIR_VERSION));

    if (BYTES_BIG_ENDIAN) {
      global::State::instance ().activateVersion (global::CoreNames::get (global::BIG_ENDIAN_VERSION));
    } else {
      global::State::instance ().activateVersion (global::CoreNames::get (global::LITTLE_ENDIAN_VERSION));
    }
    
    /* Initialize target info tables, the keys required by the language are added
       last, so that the OS and CPU handlers can override.  */
    targetdm.d_register_cpu_target_info ();
    targetdm.d_register_os_target_info ();
    
    /* Emit all target-specific version identifiers.  */
    targetdm.d_cpu_versions ();
    targetdm.d_os_versions ();
    
    return true;
}


static bool
ymir_post_options (const char ** fn)
{
  global_options.x_flag_reorder_blocks_and_partition = 0;
  global_options.x_flag_exceptions = 1;

  return false;
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
	case OPT_funittest : 
	    global::State::instance ().activateIncludeTesting (true);
	    global::State::instance ().activateVersion (global::CoreNames::get (global::UNITTEST_VERSION));
	    break;
	case OPT_fno_reflect : 
	    global::State::instance ().activateReflection (false);
	    global::State::instance ().activateVersion (global::CoreNames::get (global::REFLECT_VERSION));
	    break;
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
	global::State::instance ().activateVersion (global::CoreNames::get (global::UNITTEST_VERSION));
    } else if (code == OPT_fno_reflect) {	
	global::State::instance ().activateReflection (false);
	global::State::instance ().activateVersion (global::CoreNames::get (global::REFLECT_VERSION));
    } else if (code == OPT_fversion_) {
	global::State::instance ().activateVersion (arg);
    } else if (code == OPT_imultilib) {
	// set multilib
    } else {
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
  if (mode == QImode)
    return unsignedp ? y_u8_type : y_i8_type;

  if (mode == HImode)
    return unsignedp ? y_u16_type : y_i16_type;

  if (mode == SImode)
    return unsignedp ? y_u32_type : y_i32_type;

  if (mode == DImode)
    return unsignedp ? y_u64_type : y_i64_type;

  if (mode == TYPE_MODE (y_isize_type))
    return unsignedp ? y_usize_type : y_isize_type;

  if (mode == TYPE_MODE (float_type_node))
    return float_type_node;

  if (mode == TYPE_MODE (double_type_node))
    return double_type_node;

  if (mode == TYPE_MODE (long_double_type_node))
    return long_double_type_node;

  if (mode == TYPE_MODE (build_pointer_type (y_c8_type)))
    return build_pointer_type (y_c8_type);

  if (mode == TYPE_MODE (build_pointer_type (y_i32_type)))
    return build_pointer_type (y_i32_type);

  for (int i = 0; i < NUM_INT_N_ENTS; i ++)
    {
      if (int_n_enabled_p[i] && mode == int_n_data[i].m)
	{
	  if (unsignedp)
	    return int_n_trees[i].unsigned_type;
	  else
	    return int_n_trees[i].signed_type;
	}
    }
  
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
    } else if (VECTOR_MODE_P (mode)) {
      machine_mode inner_mode = (machine_mode) GET_MODE_INNER (mode);
      tree inner_type = ymir_langhook_type_for_mode (inner_mode, unsignedp);
      if (inner_type != NULL_TREE)
	return build_vector_type_for_mode (inner_type, mode);
    }

    /* gcc_unreachable */
    return NULL;
}
 
static tree
ymir_langhook_type_for_size (unsigned int bits,
                             int unsignedp)
{
    if (bits <= TYPE_PRECISION (y_u8_type))
    return unsignedp ? y_u8_type : y_i8_type;

    if (bits <= TYPE_PRECISION (y_i16_type))
    return unsignedp ? y_u16_type : y_u16_type;

    if (bits <= TYPE_PRECISION (y_i32_type))
    return unsignedp ? y_u32_type : y_i32_type;
    
    if (bits <= TYPE_PRECISION (y_i64_type))
    return unsignedp ? y_u64_type : y_i64_type;

    for (int i = 0; i < NUM_INT_N_ENTS; i ++)
      {
	if (int_n_enabled_p[i] && bits == int_n_data[i].bitsize)
	  {
	    if (unsignedp)
	      return int_n_trees[i].unsigned_type;
	    else
	      return int_n_trees[i].signed_type;
	  }
      }

    return NULL;
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

static GTY(()) tree ymir_eh_personality_decl;

static tree
ymir_eh_personality (void) {
    if (!ymir_eh_personality_decl) {
	ymir_eh_personality_decl = build_personality_function ("gyc");	
    }

    return ymir_eh_personality_decl;
}

static tree
ymir_build_eh_runtime_type (tree type) {
  return generic::Tree::buildPtrCst (lexing::Word::eof (), 0).getTree ();
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
#undef LANG_HOOKS_EH_PERSONALITY
#undef LANG_HOOKS_EH_RUNTIME_TYPE
#undef LANG_HOOKS_POST_OPTIONS

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
#define LANG_HOOKS_EH_PERSONALITY	    ymir_eh_personality
#define LANG_HOOKS_EH_RUNTIME_TYPE	    ymir_build_eh_runtime_type
#define LANG_HOOKS_POST_OPTIONS		    ymir_post_options

struct lang_hooks lang_hooks = LANG_HOOKS_INITIALIZER;
 
#include "gt-ymir-ymir1.h"
#include "gtype-ymir.h"
