#pragma once


#include <map>
#include <memory>
#include <ymir/utils/Memory.hh>

/**
 * Implementation of a proxy design pattern
 * This allows to use polymorphism with hidden allocation, and ensure that all allocation or freed when unused
 * Each instance of the proxy have their own data
 * \param T inner type that will be collected
 * \param I the heir type that is the real proxy
 */
template <typename T, class I> class RefProxy {    
protected:

    std::shared_ptr <T> _value;
    
public:
    
    RefProxy<T, I> (std::shared_ptr<T> a) : _value (a) {}   

    RefProxy<T, I> (T * a) : _value (a, Ymir::deleter<T> ()) {}

    RefProxy<T, I> (const RefProxy<T, I> & ot) : _value (ot._value) {}

    const RefProxy<T, I>& operator=(const RefProxy<T, I> & ot) {	
	this-> _value = ot._value;	
	return ot;
    }

    const std::shared_ptr<T> & getPtr () const {
	return this-> _value;
    }
    
    std::shared_ptr<T> & getPtr () {
	return this-> _value;
    }
    
};
