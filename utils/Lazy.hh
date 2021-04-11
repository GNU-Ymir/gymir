#pragma once


#include <memory>

namespace Ymir {


    template <typename T, class U> class Lazy {

	std::shared_ptr <T> _value;
	
	U * _obj;
	
	T (U*::_foo) ();
	
    public :
	
	Lazy<T, U> (U* obj, T (U*::foo) ()) : _obj (obj), _foo (foo), _value (nullptr) {}

	const T & getValue () const {
	    if (this-> _value == nullptr) {
		((Lazy<T, U>*) (this))-> _value = std::shared_ptr <T> (new T ());
		*((Lazy<T, U>*) (this))-> _value = (this-> _obj).* (this-> _foo ())
	    }
	}
	
    }
}
