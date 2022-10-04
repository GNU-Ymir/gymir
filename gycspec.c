
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "opt-suggestions.h"
#include "gcc.h"
#include "tm.h"
#include "opts.h"
#include "cppdefault.h"
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <string>

#ifndef LIBYMIDGARD
#define LIBYMIDGARD "gymidgard"
#endif

#ifndef LIBGC
#define LIBGC "gc"
#endif

#ifndef LIBM
#define LIBM "m"
#endif

#ifndef LIBPTHREAD
#ifdef _WIN32
#define LIBPTHREAD "winpthread"
#else
#define LIBPTHREAD "pthread"
#endif
#endif

typedef unsigned int uint;

void
lang_specific_driver (struct cl_decoded_option ** in_decoded_options ,
		      unsigned int * in_decoded_options_count,
		      int * in_added_libraries)
{
    uint i;
    cl_decoded_option *new_decoded_options;
    uint num_args = 1;
    uint argc = *in_decoded_options_count;
    cl_decoded_option *decoded_options = *in_decoded_options;
    int added_libraries = *in_added_libraries;
    bool need_gc = *in_decoded_options_count != 1;
    bool need_pthread = *in_decoded_options_count != 1;
    bool need_libs = true, need_m = true;
    /* bool need_midgard = *in_decoded_options_count != 1; */
    /* bool need_runtime = *in_decoded_options_count != 1; */
    bool yr_file_found = false;
    std::vector <int> toRemove;
    std::vector <std::string> includes;
    
    for (i = 0 ; i < argc ; i++) {
	const char * arg = decoded_options [i].arg;
	if (decoded_options [i].opt_index == OPT_l) {
	    if (arg != NULL && (strcmp (arg, LIBGC) == 0)) need_gc = false;
	    if (arg != NULL && (strcmp (arg, LIBPTHREAD) == 0)) need_pthread = false;
	    if (arg != NULL && (strcmp (arg, LIBM) == 0)) need_m = false;
	} else if (decoded_options [i].opt_index == OPT_SPECIAL_input_file) {
	    yr_file_found = true;
	} else if (decoded_options [i].opt_index == OPT_nomidgardlib) {
	    need_gc = false;
	    need_libs = false;
	    need_pthread = false;
	}	    	
    }
    
    if (yr_file_found) {
	num_args = argc + ((need_gc + need_pthread + need_libs + need_m)) + includes.size () - toRemove.size ();
	new_decoded_options = XNEWVEC (cl_decoded_option, num_args);
	i = 0;

	while (i < argc) {
	    if (std::find (toRemove.begin (), toRemove.end (), i) == toRemove.end ())
		new_decoded_options [i] = decoded_options [i];
	    i ++;
	}
	
	if (need_libs) {
	    generate_option (OPT_l, LIBYMIDGARD, 1, CL_DRIVER, &new_decoded_options [i]);
	    added_libraries ++;
	    i++;
	}

	if (need_m) {
	    generate_option (OPT_l, LIBM, 1, CL_DRIVER, &new_decoded_options [i]);
	    added_libraries ++;
	    i++;
	}
	
	if (need_gc) {
	    generate_option (OPT_l, LIBGC, 1, CL_DRIVER, &new_decoded_options [i]);
	    added_libraries ++;
	    i++;
	}

	if (need_pthread) {
	    generate_option (OPT_l, LIBPTHREAD, 1, CL_DRIVER, &new_decoded_options [i]);
	    added_libraries ++;
	    i++;
	}
	
	*in_decoded_options_count = num_args;
	*in_decoded_options = new_decoded_options;
    }
    
}
 
/* Called before linking.  Returns 0 on success and -1 on failure.  */
int
lang_specific_pre_link (void)
{
    /* Not used for Ymir.  */
    return 0;
}
 
/* Number of extra output files that lang_specific_pre_link may generate.  */
int lang_specific_extra_outfiles = 0; /* Not used for Ymir.  */

