#pragma once

#include <ymir/semantic/tree/Tree.hh>

namespace semantic {

    class InternalFunction {
	
	static Ymir::Tree __fnMalloc__;
	static Ymir::Tree __y_newArray__;	
	static Ymir::Tree __y_Init_int__;
	static Ymir::Tree __y_memcpy__;
	
    public:
	
	static Ymir::Tree getMalloc ();
	static Ymir::Tree getYNewArray ();
	static Ymir::Tree getYInitInt ();
	static Ymir::Tree getYMemcpy ();
	
    };
    
}
