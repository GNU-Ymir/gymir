#include <ymir/utils/Memory.hh>
#include <string.h>
#include <cstdlib>
#include <gc/gc.h>

std::set<void*> Memory::__allocated__;
std::map<void*, int> Memory::__references__;
std::set<void*> Memory::__deleted__;
std::map<std::string, int> Memory::__counts__;

int Memory::__nb__ = 0;

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

void addRef (void * a) {
    if (a != nullptr) {
	auto it = Memory::__references__.find (a);
	if (it == Memory::__references__.end ()) {
	    Memory::__references__.emplace (a, 1);
	} else {
	    it-> second ++;		
	}
    }
}

bool removeRef (void * a) {
    if (a == nullptr) return false;
    auto it = Memory::__references__.find (a);
    if (it != Memory::__references__.end ()) {
	it-> second --;
	if (it-> second == 0) {	       		
	    Memory::__references__.erase (it-> first);
	    return true;
	}
    }
    return false;
}
