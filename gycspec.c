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
#include <set>
#include <algorithm>
#include <string>

/* Major.minor of the midgard stdlib this driver links against.  Single
   source of truth is ymir/bootstrap/YMIR_VERSION's MIDGARD_VERSION,
   threaded in via -DLIBYMIDGARD_VERSION in Make-lang.in.  */
#ifndef LIBYMIDGARD_VERSION
#define LIBYMIDGARD_VERSION "1.2"
#endif

/* Full versions reported by --version: the midgard release above, and the
   ymirc release this driver was built from (bootstrap's gyllir.toml version,
   the same value ymirc reports as __YMIR_VERSION_FULL__).  Both are threaded
   in by Make-lang.in - never hand-edit them here.  */
#ifndef MIDGARD_FULL_VERSION
#define MIDGARD_FULL_VERSION LIBYMIDGARD_VERSION
#endif

#ifndef YMIR_FULL_VERSION
#define YMIR_FULL_VERSION ""
#endif

/* Dates of the tags those two versions were cut from, YYYYMMDD, like GCC's own
   DATESTAMP.  Either is empty when the build had no way to look it up, and the
   version is then reported on its own.  */
#ifndef YMIR_VERSION_DATE
#define YMIR_VERSION_DATE ""
#endif

#ifndef MIDGARD_VERSION_DATE
#define MIDGARD_VERSION_DATE ""
#endif

/* A build that could not read one of the version files would otherwise print
   a bare, empty version line.  */
#define VERSION_OR_UNKNOWN(v) ((v)[0] != '\0' ? (v) : "unknown")

#ifndef LIBYMIDGARD_DEBUG
#define LIBYMIDGARD_DEBUG "gymidgard-debug_" LIBYMIDGARD_VERSION
#endif

#ifndef LIBYMIDGARD_RELEASE
#define LIBYMIDGARD_RELEASE "gymidgard-release_" LIBYMIDGARD_VERSION
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

#ifndef LIBDWARF
#define LIBDWARF "dwarf"
#endif

#ifndef LIBUNITTEST
#define LIBUNITTEST "gymidgard-tests_" LIBYMIDGARD_VERSION
#endif

typedef unsigned int uint;

/* Report one "<name> version <version> <date>" line, leaving the date out when
   the build had no way to look it up.  */
static void
print_version_line (FILE * out, const char * name, const char * version,
					const char * date)
{
	if (date[0] != '\0') {
		fprintf (out, "%s version %s %s\n", name, VERSION_OR_UNKNOWN (version), date);
	} else {
		fprintf (out, "%s version %s\n", name, VERSION_OR_UNKNOWN (version));
	}
}

/* Report the versions of the Ymir frontend and of the midgard stdlib bundled
   with it.  The GCC version line, the copyright and the warranty notice are
   deliberately left to the driver (gcc.cc), which prints them right after
   lang_specific_driver returns - hence these lines come first, and hence
   --version is passed through rather than consumed here.  */
static void
print_ymir_version (FILE * out)
{
	print_version_line (out, "Ymir", YMIR_FULL_VERSION, YMIR_VERSION_DATE);
	print_version_line (out, "Midgard", MIDGARD_FULL_VERSION, MIDGARD_VERSION_DATE);
}

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
	bool need_libs = true, need_unittest = false;
	
#ifdef __linux__
	bool need_m = true, need_dwarf = true;
#endif
	
	bool yr_file_found = false;
	bool in_debug = false;
	bool for_yil = false;
	bool version_asked = false;
	bool verbose = false;

	for (i = 0 ; i < argc ; i++) {
		const char * arg = decoded_options [i].arg;
		if (decoded_options [i].opt_index == OPT_l) {
			if (arg != NULL && (strcmp (arg, LIBGC) == 0)) need_gc = false;
			if (arg != NULL && (strcmp (arg, LIBPTHREAD) == 0)) need_pthread = false;
#ifdef __linux__
			if (arg != NULL && (strcmp (arg, LIBM) == 0)) need_m = false;
			if (arg != NULL && (strcmp (arg, LIBDWARF) == 0)) need_dwarf = false;
#endif
		}

		if (decoded_options [i].opt_index == OPT_SPECIAL_input_file) {
			yr_file_found = true;
		}
		
		if (decoded_options [i].opt_index == OPT_fyil) {
			need_gc = false;
			need_libs = false;
			need_pthread = false;
			need_unittest = false;
#ifdef __linux__
			need_dwarf = false;
			need_m = false;
#endif
			for_yil = true;
		}

		if (decoded_options [i].opt_index == OPT_nomidgardlib) {
			need_gc = false;
			need_libs = false;
			need_pthread = false;
			need_unittest = false;
		}

		if (decoded_options [i].opt_index == OPT_funittest) {
			need_unittest = true;
		}

		if (decoded_options [i].opt_index == OPT_g) {
			in_debug = true;
		}

		if (decoded_options [i].opt_index == OPT__version) {
			version_asked = true;
		}

		if (decoded_options [i].opt_index == OPT_v) {
			verbose = true;
		}
	}

	/* --version goes to stdout, alongside the driver's own version block; -v
	   goes to stderr, alongside the configuration dump gcc.cc prints there.  */
	if (version_asked) {
		print_ymir_version (stdout);
	} else if (verbose) {
		print_ymir_version (stderr);
	}

	if (yr_file_found) {
#ifdef __linux__		
		num_args = argc + (need_gc + need_pthread + need_libs + need_m + need_dwarf + need_unittest + for_yil);
#else
		num_args = argc + (need_gc + need_pthread + need_libs + need_unittest + for_yil);
#endif
		new_decoded_options = XNEWVEC (cl_decoded_option, num_args);

		i = 0;
		while (i < argc) {
			new_decoded_options [i] = decoded_options [i];						
			i ++;
		}

		// add fsyntax_only for the backend to generate only yil object files
		if (for_yil) { 
			generate_option (OPT_fsyntax_only, NULL, 1, CL_DRIVER, &new_decoded_options [i]);
			i ++;
		}
		
		if (need_libs) {
			if (in_debug) {
				generate_option (OPT_l, LIBYMIDGARD_DEBUG, 1, CL_DRIVER, &new_decoded_options [i]);
			} else {
				generate_option (OPT_l, LIBYMIDGARD_RELEASE, 1, CL_DRIVER, &new_decoded_options [i]);
			}
			added_libraries ++;
			i++;
		}

		if (need_unittest) {
			generate_option (OPT_l, LIBUNITTEST, 1, CL_DRIVER, &new_decoded_options [i]);
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
		
#ifdef __linux__
		if (need_m) {
			generate_option (OPT_l, LIBM, 1, CL_DRIVER, &new_decoded_options [i]);
			added_libraries ++;
			i++;
		}

		if (need_dwarf) {
			generate_option (OPT_l, LIBDWARF, 1, CL_DRIVER, &new_decoded_options [i]);
			added_libraries ++;
			i++;
		}
#endif

		*in_decoded_options_count = num_args;
		*in_decoded_options = new_decoded_options;
		*in_added_libraries += added_libraries;
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
