#pragma once
#include <exception> // for std::bad_alloc
#include <new>
#include <stdio.h>

struct zeromemory_t{};
extern const zeromemory_t Z0;
void* operator new(size_t cbSize, const zeromemory_t&);
void* operator new[](size_t cbSize, const zeromemory_t&);
