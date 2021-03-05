#include <ymir/utils/Memory.hh>
#include <string.h>
#include <cstdlib>
#include <gc/gc.h>
#include <stdio.h>
#include <execinfo.h>

#ifdef MEM_DEBUG
std::map <void*, std::string> __allocs__;
	
std::string getStackTrace () {
    static bool _in_trace_ = false;
    if (!_in_trace_) {
	_in_trace_ = true;
	void *trace[16];
	char **messages = (char **) NULL;

	auto trace_size = backtrace(trace, 16);
	messages = backtrace_symbols(trace, trace_size);
	/* skip first stack frame (points here) */
	std::string ss;
    
	ss = ss + "[bt] Execution path:\n";
	for (int i=2; i<trace_size; ++i)
	{
	    ss = ss + "[bt] #";
	    ss = ss + std::to_string (i - 1) + "\n";
	    std::string msg = messages [i];
	    ss = ss + "\t" + msg + "\n";
	}
	_in_trace_ = false;
	return ss;
    } else {
	return "";
    }
}
#endif


const no_garbage_collection_t NO_GC;

void* operator new (size_t cbSize, const no_garbage_collection_t&) {
    void *mem = ::operator new (cbSize);
    memset(mem,0,cbSize);
    
#ifdef MEM_DEBUG
    __allocs__.emplace (mem, getStackTrace ());
#endif
    
    return mem;
}

#ifdef MEM_DEBUG
void operator delete (void* data) {
    __allocs__.erase (data);        
    free (data);
}
#endif

void control_memory_leakage () {
#ifdef MEM_DEBUG
    printf ("Not deallocated %i\n : ", __allocs__.size ());
    for (auto it : __allocs__) {
	printf ("LEAK ! %s\n", it.second.c_str ());
	int * x = nullptr;
	*x = 98;
    }
#endif
}
