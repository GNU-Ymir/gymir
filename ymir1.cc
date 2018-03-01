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
#include <ymir/semantic/tree/Tree.hh>
#include <ymir/utils/Options.hh>
#include "Parser.hh"


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
ymir_init_options (unsigned int argc, cl_decoded_option * decoded_options)
{
    for (unsigned int i = 0 ; i < argc ; i++) {
	//const char * arg = decoded_options [i].arg;
	switch (decoded_options [i].opt_index) {
	case OPT_g :
	case OPT_ggdb : Options::instance ().isDebug () = true; break;
	case OPT_v : Options::instance ().isVerbose () = true; break;
	}
    }
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

static bool
ymir_langhook_handle_option (size_t scode, const char *arg, int value ATTRIBUTE_UNUSED,
		   int kind ATTRIBUTE_UNUSED, location_t loc ATTRIBUTE_UNUSED,
		   const struct cl_option_handlers *handlers ATTRIBUTE_UNUSED)
{
    opt_code code = (opt_code) scode;
    if (code == OPT_I)
	Options::instance ().addIncludeDir (arg);
    else if (code == OPT_iprefix) 
	Options::instance ().setPrefix (arg);
    else if (code == OPT_v) {
	Options::instance ().isVerbose () = true;
    }
    else {
	return false;
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
 
/* Record a builtin function.  We just ignore builtin functions.  */
 
static tree
ymir_langhook_builtin_function (tree decl)
{
  return decl;
}
 
static bool
ymir_langhook_global_bindings_p (void)
{
    gcc_unreachable ();
    return true;
}
 
static tree
ymir_langhook_pushdecl (tree decl ATTRIBUTE_UNUSED)
{
  gcc_unreachable ();
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
