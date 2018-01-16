
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "gcc.h"
#include "tm.h"
#include "opts.h"
#include <stdio.h>

#ifndef LIBYRUNTIME
#define LIBYRUNTIME "gyruntime"
#endif

#ifndef LIBYMIDGARD
#define LIBYMIDGARD "gymidgard"
#endif

#ifndef LIBGC
#define LIBGC "gc"
#endif

void
lang_specific_driver (struct cl_decoded_option ** in_decoded_options ,
		      unsigned int * in_decoded_options_count,
		      int * in_added_libraries)
{
    uint i, j;
    cl_decoded_option *new_decoded_options;
    uint num_args = 1;
    uint argc = *in_decoded_options_count;
    cl_decoded_option *decoded_options = *in_decoded_options;
    int added_libraries = *in_added_libraries;
    bool need_gc = *in_decoded_options_count != 1;
    bool need_midgard = *in_decoded_options_count != 1;
    bool need_runtime = *in_decoded_options_count != 1;
    
    for (i = 0 ; i < argc ; i++) {
	const char * arg = decoded_options [i].arg;
	if (arg != NULL) {
	switch (decoded_options [i].opt_index) {
	case OPT_l:
	    if ((strcmp (arg, LIBGC) == 0)) need_gc = false;
	    else if (strcmp (arg, LIBYRUNTIME) == 0) need_runtime = false;
	    else if (strcmp (arg, LIBYMIDGARD) == 0) need_midgard = false;
	    break;
	}
	} 
    }

    num_args = argc + need_gc + need_midgard + need_runtime;
    new_decoded_options = XNEWVEC (cl_decoded_option, num_args);
    i = 0; j = 0;

    while (i < argc) {
	new_decoded_options [i] = decoded_options [i];
	i ++;
    }

    if (need_gc) {
	generate_option (OPT_l, LIBGC, 1, CL_DRIVER, &new_decoded_options [i]);
	added_libraries ++;
	i++;
    }
    
    if (need_runtime) {
	generate_option (OPT_l, LIBYRUNTIME, 1, CL_DRIVER, &new_decoded_options [i]);
	added_libraries ++;
	i++;
    }

    if (need_midgard) {
	generate_option (OPT_l, LIBYMIDGARD, 1, CL_DRIVER, &new_decoded_options [i]);
	added_libraries ++;
	i++;
    }    

    *in_decoded_options_count = num_args;
    *in_decoded_options = new_decoded_options;
}
 
/* Called before linking.  Returns 0 on success and -1 on failure.  */
int
lang_specific_pre_link (void)
{
    /* Not used for Tiny.  */
    return 0;
}
 
/* Number of extra output files that lang_specific_pre_link may generate.  */
int lang_specific_extra_outfiles = 0; /* Not used for Tiny.  */
