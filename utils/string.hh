#include <string>
#include <ymir/utils/OutBuffer.hh>

namespace Ymir {

    // trim from start
    static inline std::string ltrim(const std::string &s) {
	if (s.length () == 0) return s;
	uint64_t i = 0;
	while (i < s.length () && (s [i] == ' ' || s [i] == '\n' || s [i] == '\r')) {
	    i += 1;
	}		

	return s.substr (i);
    }

    // trim from end
    static inline std::string rtrim(const std::string &s) {
	if (s.length () == 0) return s;
	uint64_t i = s.length () ;
	while (i > 0 && (s [i - 1] == ' ' || s [i - 1] == '\n' || s[i - 1] == '\r')) {
	    i -= 1;
	}
	
	return s.substr (0, i);
    }

    // trim from both ends
    static inline std::string trim(const std::string &s) {
	return ltrim(rtrim (s));
    }

    static inline std::string toUpper (const std::string & str) {
	OutBuffer res;
	for (auto s : str) {
	    if (s >= 'a' && s <= 'z') {
		res.write ((char) ((s - 'a') + 'A')); // C++ and its automatic ugly casts...
	    } else {
		res.write (s);
	    }
	}
	
	return res.str ();
    }


    static inline std::string entab (const std::string & text, const std::string & tab, bool fst = true) {
	OutBuffer buf;	
	bool need_Tab = fst;
	for (auto s : text) {
	    if (need_Tab) {
		buf.write (tab);
		need_Tab = false;
	    }
	    
	    buf.write (s);
	    if (s == '\n')
		need_Tab = true;
	}
	
	return buf.str ();
    }

   
    static inline std::string replace (const std::string & str, char f, char t) {
	OutBuffer buf;
	for (auto s : str) {
	    if (s == f) buf.write (t);
	    else buf.write (s);
	}
	return buf.str ();
    }

    static inline std::string replace (const std::string & str, const std::string & b, const std::string & c) {
	OutBuffer stream;
	uint64_t i = 0;
	while (i < str.length ()) {
	    if (i + b.length () <= str.length ()) {
		if (str.substr (i, b.length ()) == b) {
		    stream.write (c);
		    i += b.length ();
		} else {
		    stream.write (str[i]);
		    i += 1;
		}
	    } else {
		stream.write (str.substr (i));
		break;	    
	    }	    
	}

	return stream.str ();
    }

    struct str_slice {
	const char * data;
	uint64_t len;		
    };
    
}
