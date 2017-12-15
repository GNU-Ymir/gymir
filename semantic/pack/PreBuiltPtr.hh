#pragma once

#include <ymir/semantic/tree/Tree.hh>

namespace semantic {

    class InternalFunction {
	
	static Ymir::Tree __fnMalloc__;	
	
	static Ymir::Tree getMalloc ();
	
    };
    
}
