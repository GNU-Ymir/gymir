#pragma once

#include <ymir/semantic/tree/Tree.hh>
#include <map>

namespace semantic {

    class InternalFunction {

	static std::map <std::string, Ymir::Tree> __funcs__;
	static Ymir::Tree __fnMalloc__;
	static Ymir::Tree __y_newArray__;	
	static Ymir::Tree __y_memcpy__;
	static Ymir::Tree __y_memset__;
	static Ymir::Tree __y_run_main__;
	static Ymir::Tree __y_main__;
	static Ymir::Tree __y_error__;
	static Ymir::Tree __abort__;
	
    public:
	
	static Ymir::Tree getMalloc ();
	static Ymir::Tree getYNewArray ();
	static Ymir::Tree getYMemcpy ();
	static Ymir::Tree getYMemset ();
	static Ymir::Tree getYInitType (const char*);
	static Ymir::Tree getYMainPtr ();
	static Ymir::Tree getYRunMain ();
	static Ymir::Tree getYError ();
	static Ymir::Tree getAbort ();
    };
    
}
