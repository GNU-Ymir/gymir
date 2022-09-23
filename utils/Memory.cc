#include <ymir/utils/Memory.hh>
#include <string.h>
#include <cstdlib>
//#include <gc/gc.h>
#include <stdio.h>
//#include <execinfo.h>
#include <ymir/utils/Benchmark.hh>



const no_garbage_collection_t NO_GC;

void* operator new (size_t cbSize, const no_garbage_collection_t&) {
    char *mem = new char[cbSize];
    // memset(mem,0,cbSize);
    
    for (size_t i = 0; i < cbSize; i += 4096) mem[i] = 0;
    mem[cbSize - 1] = 0;

    return (void*) mem;
}



void control_memory_leakage () {
}
