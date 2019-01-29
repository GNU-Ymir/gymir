#pragma once

#include <functional>

namespace Ymir {
    class Scope {

	std::function <void()> _func;
    
    public :
    
	Scope (std::function <void ()> ptr) :
	    _func (ptr) 
	{}


	~Scope () {
	    this-> _func ();
	}        

    };
}
