#include <ymir/errors/Error.hh>
#include <string>
#include <ymir/utils/OutBuffer.hh>

namespace Ymir {
    namespace Error {

	std::string substr (const std::string& x, ulong beg, ulong end) {
	    if (end - beg > x.length ()) return "";
	    if (beg > x.length ()) return "";
	    return x.substr (beg, end - beg);
	}
    
	std::string readln (FILE * i) {
	    unsigned long max = 255;
	    std::string final = "";
	    while (1) {
		char * buf = new (Z0) char[max];
		char * aux = fgets(buf, max, i);
		if (aux == NULL) return "";
		std::string ret = std::string (buf);
		final += ret;
		free (buf);
		if (ret.size () != max - 1) return final;
		else max *= 2;      
	    }
	}  
    
	std::string getLine (const location_t& locus, const char * filename) {
	    auto file = fopen (filename, "r");
	    std::string cline;
	    for (auto it = 0 ; it < LOCATION_LINE (locus) ; it ++) {
		cline = readln (file);
	    }
	    if (file) fclose (file);
	    return cline;
	}
	
	ulong computeMid (std::string& mid, const std::string& word, const std::string& line, ulong begin, ulong end) {
	    auto end2 = begin;
	    for (auto it = 0 ; it < (int) word.size () < end - begin ? word.size () : end - begin; it ++) {
		if (word [it] != line [begin + it]) break;
		else end2 ++;
	    }
	    
	    mid = substr (line, begin, end2);
	    return end2;
	}	
    
	std::string center (const std::string &toCenter, ulong nb, char concat) {
	    OutBuffer buf;
	    nb = nb - toCenter.length ();
	    
	    for (int i = 0 ; i < (int) nb / 2 ; i++) buf.write (concat);	    
	    buf.write (toCenter);	    
	    for (int i = 0 ; i < (int) (nb - nb / 2) ; i++) buf.write (concat);
	    
	    return buf.str ();
	}

	std::string rightJustify (const std::string &toJustify, ulong nb, char concat) {
	    nb = nb - toJustify.length ();
	    OutBuffer buf;
	    for (int i = 0 ; i < (int) nb; i++)
		buf.write (concat);
	    
	    buf.write (toJustify);
	    return buf.str ();
	}
    	
	std::string addLine (const std::string & msg, const Word & word) {
	    
	}
	
	void halt (const char * format) {
	    fprintf (stderr, "%s", Ymir::format ("%(r) : %\n", "Assert", "abort").c_str ());
	    raise (SIGABRT);
	}
	
    }
}
