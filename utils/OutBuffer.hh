#pragma once

#include <ymir/utils/memory.hh>
#include <ymir/utils/Options.hh>
#include <string>
#include <map>
#include <ymir/utils/Range.hh>
#include <vector>
#include <set>
#include <type_traits>

namespace syntax {
    class IExpression;
    typedef IExpression* Expression;
}

struct Word;

namespace semantic {
    class IInfoType;
    typedef IInfoType* InfoType;

    struct Namespace;
    
}

namespace Ymir {
    
    class OutBuffer  {
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

	template <typename T>
	void write_ (const std::set <T> &elem) {
	    int i = 0;
	    write ("{");
	    for (auto &it : elem) {
		if (i != 0) write (", ");
		write_ (it);
		i++;
	    }
	    write ("}");
	}

	template <typename K, typename V>	
	void write_ (const std::map <K, V> &elem) {
	    write ("{");
	    for (auto it = elem.begin () ; it != elem.end ();) {
		write (it-> first, " : ", it-> second);
		it ++;
		if (it != elem.end ())
		    write (", ");
	    }
	    write ("}");
	}
	
	void write_ (const Word& word);	
	
	void write_ (syntax::Expression expr);

	void write_ (semantic::InfoType type);

	void write_ (semantic::Namespace& space);
	
	void write_ (const char * str);
	
	void write_ (const std::string&);

	void write_ (int);
	
	void write_ (long);

	void write_ (ulong);

	void write_ (float);
	
	void write_ (char);

	void write_ (double);
		
	void write_ (bool);
	
	void throwError ();

    private:

	void resize (ulong len);

	
    };
    

}

template <typename ... T>
void println (T ... args) {
    printf ("%s\n", Ymir::OutBuffer (args...).str ().c_str ());
}

template <typename ... T>
void print (T ... args) {
    printf ("%s", Ymir::OutBuffer (args...).str ().c_str ());
}


namespace Ymir {
    template <typename ... T>
    void log (T ... args) {
	if (Options::instance ().isVerbose ())
	    println (args...);
    }
}
