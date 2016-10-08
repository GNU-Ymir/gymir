#pragma once

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "input.h"
#include "diagnostic.h"


namespace Ymir {
    
    struct Error {

	template <typename ... TArgs>
	static void fatal (location_t locus, const char * format, TArgs ... args) {
	    fatal_error (locus, format, args...);
	}
		
	template <typename ... TArgs>
	static void append (location_t locus, const char * format, TArgs ... args) {
	    error_at (locus, format, args...);
	    nb_errors ++;
	}

	static unsigned long nb_errors;
	
    };
	
}
