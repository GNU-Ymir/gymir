#pragma once

#include <gc/gc_cpp.h>
#include <string>

namespace Ymir {

    class OutBuffer : public gc {
    private :

	char * current = NULL;
	ulong len = 0;
	ulong capacity = 0;
	
    public:

	template <typename F, typename ... T>
	void write (F f, T ... args) {
	    write (f);
	    write (args...);
	}

	template <typename F, typename ... T>
	void writeln (F f, T ... args) {
	    write (f);
	    write (args...);
	    write ('\n');
	}

	template <typename F> 
	void writeln (F f) {
	    write (f);
	    write ('\n');
	}
	
	void write ();
	
	void write (std::string);
	
	void write (long);

	void write (ulong);

	void write (char);

	void write (double);
	
	std::string str () {
	    return std::string (current, len);
	}
	
    private:

	void resize (ulong len);

	
    };
    

}
