#pragma once
#include <exception> // for std::bad_alloc
#include <new>
#include <stdio.h>
#include <set>
#include <map>
#include <string>
#include <ymir/utils/Benchmark.hh>


struct no_garbage_collection_t {};

/**
 * Used on new expression to alloc new element with the memory set to 0, but the deletion must be done by the allocator
 */
extern const no_garbage_collection_t NO_GC;

/**
 * Allocate a new segment of data, with the value set to 0
 * implemented in utils/Memory.cc
 */
void* operator new(size_t cbSize, const no_garbage_collection_t&);


void control_memory_leakage ();



namespace Ymir {

    template <typename T>
    class deleter {
    public:
	void operator() (const T * elem) {
	    delete elem;
	}
    };    

}
