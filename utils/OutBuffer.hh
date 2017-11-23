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
	    write_ (f);
	    write (args...);
	}

	template <typename F, typename ... T>
	void writeln (F f, T ... args) {
	    write_ (f);
	    write (args...);
	    write ('\n');
	}

	template <typename F> 
	void writeln (F f) {
	    write (f);
	    write ('\n');
	}
	

	std::string str () {
	    return std::string (current, len);
	}

    private :

	void write ();

	void write_ (const char * str);
	
	void write_ (std::string);

	void write_ (int);
	
	void write_ (long);

	void write_ (ulong);

	void write_ (char);

	void write_ (double);
	
	
    private:

	void resize (ulong len);

	
    };
    

}
