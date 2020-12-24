#pragma once
#include <exception> // for std::bad_alloc
#include <new>
#include <stdio.h>
#include <set>
#include <map>
#include <string>
#include <ymir/errors/Error.hh>

// Uncomment the following line to analyse memory allocation deallocation
// This is very very slow, but effective
// #define MEM_DEBUG

/**
 * Used only one time to define Z0
 */
struct garbage_collection_t {};

/**
 * Used on new expression to alloc a new element with the memory set to 0, and allocated with the Garbage collector
 * implemented in utils/Memory.cc
 */
extern const garbage_collection_t GC;


struct no_garbage_collection_t {};

/**
 * Used on new expression to alloc new element with the memory set to 0, but the deletion must be done by the allocator
 */
extern const no_garbage_collection_t NO_GC;

/**
 * Allocate a new segment of data, with the value set to 0
 * implemented in utils/Memory.cc
 */
void* operator new(size_t cbSize, const garbage_collection_t &);

/**
 * Allocate a new array of data, with values set to 0 
 * implemented in utils/Memory.cc
 */
void* operator new[](size_t cbSize, const garbage_collection_t &);


/**
 * Allocate a new segment of data, with the value set to 0
 * implemented in utils/Memory.cc
 */
void* operator new(size_t cbSize, const no_garbage_collection_t&);


void control_memory_leakage ();

