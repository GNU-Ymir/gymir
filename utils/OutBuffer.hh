#pragma once

#include <ymir/utils/Colors.hh>
#include <ymir/utils/StringEnum.hh>
#include <string>
#include <map>
#include <ymir/utils/Range.hh>
#include <vector>
#include <list>
#include <set>
#include <type_traits>
#include <ymir/global/State.hh>
#include <ymir/global/Core.hh>

namespace lexing {
    struct Word;
}

typedef char ubyte ;

namespace Ymir {

    /**
     * \class OutBuffer
     * This class is used to append string content     
     * implemented in OutBuffer.cc
     */
    class OutBuffer  {
    private :
	
	std::vector<char> current;
	ulong len = 0;
	ulong capacity = 0;
	std::vector <std::string> _currentColor;
	std::vector <std::string> _entabing;
	bool _willEntab = false;
	
    public:

	template <typename ... T>
	OutBuffer (T ... args) {
	    write (args ...);
	}

	~OutBuffer ();
	
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
	    return std::string (current.data (), len);
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
			case 'r' : this-> _currentColor.push_back (Colors::get (RED)); break;
			case 'b' : this-> _currentColor.push_back (Colors::get (BLUE)); break;
			case 'y' : this-> _currentColor.push_back (Colors::get (YELLOW)); break;
			case 'g' : this-> _currentColor.push_back (Colors::get (GREEN)); break;
			case 'B' : this-> _currentColor.push_back (Colors::get (BOLD)); break;
			}
			write (this-> _currentColor.back ());
			write (f);
			write (Colors::get (RESET));
			this-> _currentColor.pop_back ();
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
		if (it < (int) elem.size () - 1) {
		    if (this-> _currentColor.size () != 0)
			write (Colors::get (RESET));
		    write (", ");
		    if (this-> _currentColor.size () != 0)
			write (this-> _currentColor.back ());
		}
	    }
	}

	template <typename T>
	void write_ (const std::list <T> &elem) {
	    int i = 0;
	    for (auto it : elem) {
		if (i != 0) write_ (", ");
		write_ (it);
		i += 1;
	    }
	}

	
	template <typename T>
	void write_ (const std::set <T> &elem) {
	    int i = 0;
	    write ("{");
	    for (auto &it : elem) {
		if (i != 0) {
		    if (this-> _currentColor.size () != 0)
			write (Colors::get (RESET));
		    write (", ");
		    if (this-> _currentColor.size () != 0)
			write (this-> _currentColor.back ());
		}
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
		if (it != elem.end ()) {
		    if (this-> _currentColor.size () != 0)
			write (Colors::get (RESET));
		    write (", ");
		    if (this-> _currentColor.size () != 0)
			write (this-> _currentColor.back ());
		}
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

	void write_ (uint8_t);

	void write_ (__int128_t);
	
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

template <typename ... T>
void log (T ... args) {
    if (global::State::instance ().isVerboseActive ()) 
	printf ("%s\n", Ymir::OutBuffer (args...).str ().c_str ());
}



namespace Ymir {
    std::string entab (const std::string & value);
}

// namespace Ymir {
//     template <typename ... T>
//     void log (T ... args) {
// 	if (Options::instance ().isVerbose ())
// 	    println (args...);
//     }
// }
