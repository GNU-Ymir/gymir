#include <ymir/utils/Memory.hh>
#include <string.h>
#include <cstdlib>

const zeromemory_t Z0;

void* operator new (size_t cbSize, const zeromemory_t&) {
    void *mem = ::operator new (cbSize);
    memset(mem,0,cbSize);
    return mem;
}

void* operator new[] (size_t cbSize, const zeromemory_t&) {
    void *mem = ::operator new(cbSize);
    memset(mem,0,cbSize);
    return mem;
}
