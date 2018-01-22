#pragma once

#include <gc/gc_cpp.h>
#include <string>
#include <ymir/utils/Range.hh>
#include <vector>

namespace syntax {
    class IExpression;
    typedef IExpression* Expression;
}

namespace Ymir {
    
    class OutBuffer : public gc {
    private :
	
	char * current = NULL;
	ulong len = 0;
	ulong capacity = 0;
	
    public:

	template <typename ... T>
	OutBuffer (T ... args) {
	    write (args ...);
	}
	
	void writef (const char* s) {
	    write (s);
	}
	
	template <typename F, typename ... T>
	void writef (const char * s, F f, T ... args) {
	    mwritef (s, f, args...);
	}
	
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

	void mwritef (const char * s) {
	    write (s);
	}	
	template <typename W,  typename ... T>
	void writeMult (const char *& s, int nb, W what, T ... args) {
	    for (int i = 0 ; i < nb; i++)
		write (what);
	    mwritef (s, args...);
	}
	
	template <typename I, typename W,  typename ... T>
	void writeMult (const char *&, I , W , T ... ) {
	    throwError ();
	}

	template <typename I>
	void writeMult (const char *&, I) {
	    throwError ();
	}
	
	void writeMult (const char *&) {
	    throwError ();
	}

	void writeMult (const char*&, int) {
	    throwError ();
	}
	
	template <typename F, typename ... T>
	void mwritef (const char *& s, F f, T ... args) {
	    while (*s) {
		if (*s == '\\') {
		    ++s ;
		} else if (*s == '%') {
		    s++;
		    if (*(s + 1) == '*') {
			s++;
			writeMult (s, f, args...);
		    } else {			
			write (f);
			mwritef (s, args...);
		    }
		    break;
		}
		write (*s);
		s++;
	    }
	}
		
	void write ();
	
	template <typename T>
	void write_ (const std::vector <T> &elem) {
	    for (auto it : Ymir::r (0, elem.size ())) {
		write_ (elem [it]);
		if (it < (int) elem.size () - 1)
		    write (", ");
	    }
	}

	void write_ (syntax::Expression expr);
	
	void write_ (const char * str);
	
	void write_ (const std::string&);

	void write_ (int);
	
	void write_ (long);

	void write_ (ulong);

	void write_ (char);

	void write_ (double);
		
	void throwError ();

    private:

	void resize (ulong len);

	
    };
    

}
