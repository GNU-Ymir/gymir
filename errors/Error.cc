#include <ymir/errors/Error.hh>
#include <string>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/utils/Colors.hh>
#include <execinfo.h>

using namespace lexing;

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
	    for (auto it = 0 ; it < (int) (word.size () < end - begin) ? word.size () : end - begin; it ++) {
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

	void addLineEof (OutBuffer & buf, const Word & word) {
	    auto locus = word.getLocus ();
	    std::string leftLine = center (format ("%", LOCATION_LINE (locus)), 3, ' ');
	    auto padd = center ("", leftLine.length (), ' ');
	    buf.write (format ("\n% --> %:(%,%)%\n%% | %\n",
			       Colors::get (BOLD),
			       word.getFile (),
			       LOCATION_LINE (locus),
			       LOCATION_COLUMN (locus),
			       Colors::get (RESET),
			       Colors::get (BOLD),
			       padd,
			       Colors::get (RESET)
	    ));

	    buf.write (format ("%% | %%\n", Colors::get (BOLD), padd, Colors::get (RESET), "'EOF'"));
	    buf.write (format ("%% | %\n", Colors::get (BOLD), padd, Colors::get (RESET)));
	}
	
	void addLine (OutBuffer & buf, const Word & word) {
	    auto locus = word.getLocus ();
	    auto line = getLine (locus, word.getFile ().c_str ());
	    if (line.length () > 0) {
		auto leftLine = center (format ("%", LOCATION_LINE (locus)), 3, ' ');
		auto padd = center ("", leftLine.length (), ' ');
		buf.write (format ("\n% --> %:(%,%)%\n%% | %\n",
				   Colors::get (BOLD),
				   word.getFile ().c_str (),
				   LOCATION_LINE (locus),
				   LOCATION_COLUMN (locus),
				   Colors::get (RESET),
				   Colors::get (BOLD),
				   padd,
				   Colors::get (RESET)));

		auto column = LOCATION_COLUMN (locus);
		buf.write (format ("%% | %%%(y)%",
				   Colors::get (BOLD), leftLine, Colors::get (RESET),
				   line.substr (0, column - 1),
				   substr (line, column - 1, column + word.length () - 1),
				   substr (line, column + word.length () - 1, line.length ())
		));

		if (line [line.length () - 1] != '\n') buf.write ('\n');
		buf.write (format ("%% | %", Colors::get (BOLD), padd, Colors::get (RESET)));
		for (auto it = 0 ; it < column - 1 ; it++) {
		    if (line [it] == '\t') buf.write ('\t');
		    else buf.write (' ');
		}
		
		buf.write (rightJustify ("", word.length (), '^'));
		buf.write ('\n');
	    } else addLineEof (buf, word);
	}

	
	std::string addLine (const std::string & msg, const lexing::Word & word) {
	    OutBuffer buf;
	    buf.write (msg);
	    addLine (buf, word);
	    return buf.str ();
	}
	
    }


    bool runCommand (char *sys) {
	auto fp = popen (sys, "r");
	if (fp == NULL) return false;	
    
	char path[255];
	memset (path, 0, 255 - 1);    
	fprintf (stderr, "in function : ");
	auto func = fgets (path, 255 - 1, fp);
	fprintf (stderr, "%s", func);
	fprintf (stderr, "\t%s", fgets (path, 255 - 1, fp));    
	pclose (fp);
	return true;
    }

    void bt_print () {    		
	void *trace[16];
	char **messages = (char **)NULL;
	int i, trace_size = 0;

	trace_size = backtrace(trace, 16);
	messages = backtrace_symbols(trace, trace_size);
	/* skip first stack frame (points here) */
	fprintf(stderr, "[bt] Execution path:\n");
	for (i=2; i<trace_size; ++i)
	    {
		fprintf(stderr, "[bt] #%d ", i - 1);
		/* find first occurence of '(' or ' ' in message[i] and assume
		 * everything before that is the file name. (Don't go beyond 0 though
		 * (string terminator)*/
		size_t p = 0;
		while(messages[i][p] != '(' && messages[i][p] != ' '
		      && messages[i][p] != 0)
		    ++p;

		char syscom[256];
		snprintf(syscom, 256, "addr2line %p -f -e %.*s", trace[i], (int) p, messages[i]);
		if (!runCommand (syscom))
		    fprintf (stderr, "%s %p\n", messages [i], trace [i]);
	    }

    }

}
