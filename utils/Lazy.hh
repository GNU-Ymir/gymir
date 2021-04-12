#pragma once


#include <memory>

namespace Ymir {


    template <typename T, class U> class Lazy {
	
	U* _obj;
	
	T (U::*_foo) () const;

	std::shared_ptr <T> _value;
	
    public :
	
	Lazy<T, U> (U* obj, T (U::*foo) () const) : _obj (obj), _foo (foo), _value (nullptr) {}
	Lazy<T, U> (U* obj, T (U::*foo) ()) : _obj (obj), _foo ((T (U::*) () const) foo), _value (nullptr) {}

	const T & getValue () const {
	    if (this-> _value == nullptr) {
		((Lazy<T, U>*) (this))-> _value = std::shared_ptr <T> (new T (((this-> _obj)->*(this-> _foo)) ()));
	    }

	    return *this-> _value;
	}

	void unvalidate () const {
	    ((Lazy<T, U>*) (this))-> _value = nullptr;
	}
	
    };
}
