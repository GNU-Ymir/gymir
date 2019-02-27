#pragma once

#include <ymir/utils/OutBuffer.hh>
#include <map>

class RefTable {
public : 
    static std::map <void*, int> __refsCount__;
};

/**
 * Implementation of a proxy design pattern
 * This allows to use polymorphism with hidden allocation, and ensure that all allocation or freed when unused
 * Each instance of the proxy have their own data
 * \param T inner type that will be collected
 * \param I the heir type that is the real proxy
 */
template <typename T, class I> class RefProxy {    
protected:

    T * _value;
    
public:
    
    RefProxy<T, I> (T * a) {
	this-> _value = a;
	addRef (this-> _value);
    }

    RefProxy<T, I> (const RefProxy<T, I> & ot) {
	this-> _value = ot._value;	
	addRef (this-> _value);
    }

    RefProxy<T, I>& operator=(const RefProxy<T, I> & ot) {
	dispose (this-> _value);	
	
	this-> _value = ot._value;
	addRef (this-> _value);
	
	return *this;
    }

    T* getRef () const {
	return this-> _value;
    }
    
    virtual ~RefProxy<T, I> () {
	dispose (this-> _value);
    }

private :

    static void dispose (T * value) {
	if (value == nullptr) return;
	
	auto ptr = RefTable::__refsCount__.find (value);
	if (ptr == RefTable::__refsCount__.end ()) delete value;
	else {
	    ptr-> second -= 1;
	    auto nb = ptr-> second;
	    if (nb <= 0) delete value;	    
	}
    }

    static void addRef (T * value) {
	if (value == nullptr) return;
	
	auto ptr = RefTable::__refsCount__.find (value);
	if (ptr == RefTable::__refsCount__.end ()) {
	    RefTable::__refsCount__.emplace (value, 1);
	} else ptr-> second += 1;
    }

    
};
