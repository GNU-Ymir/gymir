#pragma once

#include <gc/gc_cpp.h>
#include <algorithm>
#include <vector>
#include <stdio.h>
#include <ymir/utils/Range.hh>
#include <ymir/utils/OutBuffer.hh>
#include <cstring>

namespace Ymir {

    void assert ();
    
    template <typename T>
    class Iterator {
	T* data;

    public:

	Iterator (T* data) : data (data) {}

	const T operator*() const {
	    return *this-> data;
	}
	
	T& operator* () {
	    return *this-> data;
	}

	void operator++ () {
	    this-> data ++;
	}

	bool operator!= (const Iterator<T>& ot) {
	    return this-> data != ot.data;
	}

	bool operator== (const Iterator<T>& ot) {
	    return this-> data == ot.data;
	}
	
	T* ptr () {
	    return this-> data;
	}
	
    };       
    
    
}

template<typename T, typename T2>
auto find (std::vector <T> vec, T2 elem) {
    return std::find (vec.begin (), vec.end (), elem);
}

template<typename T, typename T2>
auto canFind (std::vector <T> vec, T2 elem) {
    return std::find (vec.begin (), vec.end (), elem) != vec.end ();
}

