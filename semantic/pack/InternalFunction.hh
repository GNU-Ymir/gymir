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
	
    public:
	
	static Ymir::Tree getMalloc ();
	static Ymir::Tree getYNewArray ();
	static Ymir::Tree getYMemcpy ();
	static Ymir::Tree getYMemset ();
	static Ymir::Tree getYInitType (const char*);
	
    };
    
}
