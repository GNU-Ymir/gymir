#pragma once

#include <ymir/semantic/tree/Tree.hh>

namespace semantic {

    class InternalFunction {
	
	static Ymir::Tree __fnMalloc__;	

    public:
	
	static Ymir::Tree getMalloc ();
	
    };
    
}
