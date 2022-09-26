#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-iterator.h"
#include "options.h"
#include "stmt.h"
#include "fold-const.h"
#include "diagnostic.h"
#include "stringpool.h"
#include "function.h"
#include "toplev.h"


/* The context to be used for global declarations.  */
extern tree __global_context__;

/* Array of all global declarations to pass back to the middle-end.  */
extern vec <tree, va_gc> *__global_declarations__;

extern tree __current_function_ctx__;

tree ymir_get_global_context ();
