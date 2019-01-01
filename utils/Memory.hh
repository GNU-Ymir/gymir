#pragma once
#include <exception> // for std::bad_alloc
#include <new>
#include <stdio.h>

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
