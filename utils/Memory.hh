#pragma once
#include <exception> // for std::bad_alloc
#include <new>
#include <stdio.h>
#include <set>
#include <map>
#include <string>
#include <ymir/errors/Error.hh>

class Memory {
public : 
    static std::set <void*> __allocated__;
    static std::set <void*> __deleted__;   
    static std::map <void*, int> __references__;
#if defined(__GXX_RTTI) // joie
    static std::map <std::string, int> __counts__;
#endif
    
    static int __nb__;
};


/**
 * Used only one time to define Z0
 */
struct zeromemory_t{};

/**
 * Used on new expression to alloc a new element with the memory set to 0
 * implemented in utils/Memory.cc
 */
extern const zeromemory_t Z0;

/**
 * Allocate a new segment of data, with the value set to 0
 * implemented in utils/Memory.cc
 */
void* operator new(size_t cbSize, const zeromemory_t&);

/**
 * Allocate a new array of data, with values set to 0 
 * implemented in utils/Memory.cc
 */
void* operator new[](size_t cbSize, const zeromemory_t&);


void addRef (void * a);
bool removeRef (void * a);

template <typename T>
class SmartPointer {
    
    
    T * _value = nullptr;
    
public :

    SmartPointer<T> (T * a) {
	if (a != nullptr) {	    
	    addRef (a);
	}
	this-> _value = a;
    }

    SmartPointer<T> (const SmartPointer<T> & ot) {
	if (ot._value != nullptr) {
	    addRef (ot._value);
	}
	this-> _value = ot._value;
    }

    const SmartPointer<T> & operator= (const SmartPointer<T> & ot) {
	auto aux = this-> _value;
	if (ot._value != nullptr) {	    
	    addRef (ot._value);
	} 	
	
	this-> _value = ot._value;
	if (removeRef (aux)) delete aux;
	
	return ot;
    }
    
    bool operator== (const T * ot) const {
	return ot == _value;
    }

    bool operator!= (const T * ot) const {
	return ot != _value;
    }

    bool operator!= (std::nullptr_t) const {
	return _value != nullptr;
    }

    bool operator== (std::nullptr_t) const {
	return _value == nullptr;
    }

    T* operator-> () {
	return this-> _value;
    }

    const T* operator-> () const {
	return this-> _value;
    }
    
    T* get () {
	return this-> _value;
    }

    const T* get () const {
	return this-> _value;
    }
    
    ~SmartPointer<T> () {
	if (this-> _value != nullptr) {	    	    
	    if (removeRef (this-> _value)) 
		delete this-> _value;
	}
    }


};
