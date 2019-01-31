#pragma once

#include <ymir/utils/Colors.hh>
#include <ymir/utils/StringEnum.hh>
#include <ymir/utils/Memory.hh>
#include <string>
#include <map>
#include <ymir/utils/Range.hh>
#include <vector>
#include <set>
#include <type_traits>

namespace lexing {
    struct Word;
}

namespace Ymir {

    /**
     * \class OutBuffer
     * This class is used to append string content     
     * implemented in OutBuffer.cc
     */
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
	void writefln (const char * s, F f, T ... args) {
	    mwritef (s, f, args...);
	    write ('\n');
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
		    if (*s == '*') {
			s++;
			writeMult (s, f, args...);
		    } else if (*s == '(') { // %(r)
			auto color = *(s + 1);
			s += 3; // (r)
			switch (color) {
			case 'r' : write (Colors::get (RED)); break;
			case 'b' : write (Colors::get (BLUE)); break;
			case 'y' : write (Colors::get (YELLOW)); break;
			case 'g' : write (Colors::get (GREEN)); break;
			case 'B' : write (Colors::get (BOLD)); break;
			}
			write (f);
			write (Colors::get (RESET));
			mwritef (s, args...);
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

	template <typename T>
	void write_ (const T* elem) {
	    if (elem == NULL) write ("null");
	    else
		write ("*(", *elem, ")");
	}
	
	void write_ (const lexing::Word& word);	
		
	void write_ (const char * str);
	
	void write_ (const std::string&);

	void write_ (uint);
	
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


// namespace Ymir {
//     template <typename ... T>
//     void log (T ... args) {
// 	if (Options::instance ().isVerbose ())
// 	    println (args...);
//     }
// }
