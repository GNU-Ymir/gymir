#include <ymir/errors/Error.hh>
#include <string>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/utils/Memory.hh>
#include <ymir/utils/Colors.hh>
#include <execinfo.h>

using namespace lexing;

namespace Ymir {
    namespace Error {

	
	void ErrorList::print () const {
	    for (auto it : this-> errors) {
		fprintf (stderr, "%s\n", it.computeMessage ().c_str ());
	    }
	}

	void FatalError::print () const {
	    fprintf (stderr, "%s\n", this-> msg.c_str ());	    
	}
	
	std::string substr (const std::string& x, ulong beg, ulong end) {
	    if (end - beg > x.length ()) return "";
	    if (beg > x.length ()) return "";
	    return x.substr (beg, end - beg);
	}

	std::string shorten (const std::string & str, ulong max = 50) {
	    if (str.length () < max) return str;
	    else {
		return str.substr (0, max/2 - 3) + "[...]" + str.substr (str.length () - max/2 + 2);
	    }
	}
	
    
	std::string getLine (int line, lexing::File file, int start) {
	    if (file.isClosed ()) {
		auto aux = file.clone (); // We can't modify the file 
		std::string cline;
		for (auto it = start ; it < line ; it ++) {
		    cline = aux.readln ();
		}
		return cline;
	    } else {
		auto cursor = file.tell ();
		file.seek (0);
		std::string cline;
		for (auto it = start ; it < line ; it ++) {
		    cline = file.readln ();
		}		
		file.seek (cursor);
		return cline;
	    }
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
	    std::string leftLine = center (format ("%", word.getLine ()), 3, ' ');
	    auto padd = center ("", leftLine.length (), ' ');
	    buf.write (format ("\n% --> %:(%,%)%\n%% ┃ %\n",
			       Colors::get (BOLD),
			       word.getFilename (),
			       word.getLine (),
			       word.getColumn (),
			       Colors::get (RESET),
			       Colors::get (BOLD),
			       padd,
			       Colors::get (RESET)
			   ));

	    buf.write ("\n");
	}
	
	
	void addLine (OutBuffer & buf, const Word & word) {
	    std::string line = getLine (word.getLine (), word.getFile (), word.getStart ());

	    if (line.length () > 0) {
		auto wordLength = word.length ();
		auto leftLine = center (format ("%", word.getLine ()), 3, ' ');
		auto padd = center ("", leftLine.length (), ' ');
		buf.write (format ("\n% --> %:(%,%)%\n%% ┃ %\n",
				   Colors::get (BOLD),
				   word.getFilename ().c_str (),
				   word.getLine (),
				   word.getColumn (),
				   Colors::get (RESET),
				   Colors::get (BOLD),
				   padd,
				   Colors::get (RESET)));

		auto column = word.getColumn ();
		if (wordLength != 0 && line [column - 1] != word.getStr () [0]) {
		    wordLength = 1;
		}

		auto begin = shorten (line.substr (0, column - 1));
		auto wordStr = shorten (substr (line, column - 1, column + wordLength - 1));
		auto between = shorten (substr (line, column + wordLength - 1, line.length ()));
		column = begin.length () + 1;
		buf.write (format ("%% ┃ %%%%(y)%%",
				   Colors::get (BOLD), leftLine, Colors::get (RESET),
				   begin,
				   Colors::get (UNDERLINE),
				   wordStr,
				   Colors::get (RESET),
				   between
			       ));

		if (line [line.length () - 1] != '\n') buf.write ('\n');
		buf.write (format ("%% ┃ %", Colors::get (BOLD), padd, Colors::get (RESET)));
		for (ulong it = 0 ; it < column - 1 ; it++) {
		    if (line [it] == '\t') buf.write ('\t');
		    else buf.write (' ');
		}
		
		buf.write (rightJustify ("", wordStr.length (), '^'));
		buf.write ('\n');
	    } else addLineEof (buf, word);
	}
	
	std::string addLine (const std::string & msg, const lexing::Word & word) {
	    OutBuffer buf;
	    buf.write (msg);
	    addLine (buf, word);
	    return buf.str ();
	}

	std::vector <std::string> splitString (const std::string & str, const std::string & delims) {
	    std::vector <std::string> cont;
	    std::size_t current, previous = 0;
	    current = str.find_first_of(delims);
	    while (current != std::string::npos) {
		cont.push_back (str.substr(previous, current - previous));
		previous = current + 1;
		current = str.find_first_of(delims, previous);
	    }
	    cont.push_back(str.substr(previous, current - previous));
	    return cont;
	}
	
	std::string addNote (const Word & word, const std::string & msg, const std::string & note, bool fst, bool oneLine) {
	    auto leftLine = center (format ("%", word.getLine ()), 3, ' ');
	    auto padd = center ("", leftLine.length (), ' ');
	    auto padd2 = center ("", leftLine.length (), '-');
	    OutBuffer buf;
	    buf.write (msg);
	    
	    auto lines = splitString (note, "\n");
	    if (!fst && !oneLine) {
		auto l = format ("%% ┃ %\n", Colors::get (BOLD), padd, Colors::get (RESET));
		buf.write (l);
	    }
	    
	    for (auto it : Ymir::r (0, lines.size ())) {
		if (lines [it].length() != 0) {
		    auto l = format ("%% ┃ %%\n", Colors::get (BOLD), padd, Colors::get (RESET), lines [it]);
		    buf.write (l);
		}
	    }
	    
	    return buf.str ();
	}
	
	void addTwoLines (OutBuffer & buf, const Word & word, const Word & end) {
	    std::string line = getLine (word.getLine (), word.getFile (), word.getStart ());
	    
	    if (line.length () > 0) {
		auto leftLine = center (format ("%", word.getLine ()), 3, ' ');
		auto padd = center ("", leftLine.length (), ' ');
		buf.write (format ("\n% --> %:(%,%)%\n%% ┃ %\n",
				   Colors::get (BOLD),
				   word.getFilename ().c_str (),
				   word.getLine (),
				   word.getColumn (),
				   Colors::get (RESET),
				   Colors::get (BOLD),
				   padd,
				   Colors::get (RESET)));

		auto column = word.getColumn ();
		auto begin =   shorten (line.substr (0, column - 1));
		auto wordStr = shorten (substr (line, column - 1, column + word.length () - 1));
		auto between =  shorten (substr (line, column + word.length () - 1, line.length ()));
		
		column = begin.length () + 1;
		buf.write (format ("%% ┃ %%%%(y)%%",
				   Colors::get (BOLD), leftLine, Colors::get (RESET),
				   begin,
				   Colors::get (UNDERLINE),
				   wordStr,
				   Colors::get (RESET),
				   between
			       ));

		if (line [line.length () - 1] != '\n') buf.write ('\n');
		buf.write (format ("%%...%", Colors::get (BOLD), padd, Colors::get (RESET)));
		for (ulong it = 0 ; it < column - 1 ; it++) {
		    if (line [it] == '\t') buf.write ('\t');
		    else buf.write (' ');
		}
		
		buf.write (rightJustify ("", wordStr.length (), '^'));
		addLine (buf, end);
	    } else addLineEof (buf, word);
	}
	
	void addLine (OutBuffer & buf, const Word & word, const Word & end) {
	    if (word.getLine () != end.getLine ()) {
		addTwoLines (buf, word, end);
		return;
	    } else if (end.getColumn () < word.getColumn ()) {
		addLine (buf, word);
		return;	    
	    }
	    
	    if (word.getLine () == end.getLine () && end.getColumn () == word.getColumn ()) {
		addLine (buf, word);
		return;
	    }
	    
	    std::string line = getLine (word.getLine (), word.getFile (), word.getStart ());
	    
	    if (line.length () > 0) {
		auto leftLine = center (format ("%", word.getLine ()), 3, ' ');
		auto padd = center ("", leftLine.length (), ' ');
		buf.write (format ("\n% --> %:(%,%)%\n%% ┃ %\n",
				   Colors::get (BOLD),
				   word.getFilename ().c_str (),
				   word.getLine (),
				   word.getColumn (),
				   Colors::get (RESET),
				   Colors::get (BOLD),
				   padd,
				   Colors::get (RESET)));


		auto column = word.getColumn ();
		auto column2 = end.getColumn ();
		auto wordStr = shorten (substr (line, column - 1, column + word.length () - 1));
		auto between = shorten (substr (line, column + word.length () - 1, column2 - 1));
		auto wordStr2 = shorten (substr (line, column2 - 1, column2 + end.length () - 1));
		buf.write (format ("%% ┃ %%%%(y)%%%%(y)%%",
				   Colors::get (BOLD), leftLine, Colors::get (RESET),
				   line.substr (0, column - 1),
				   Colors::get (UNDERLINE),
				   wordStr,
				   Colors::get(RESET),
				   between,
				   Colors::get (UNDERLINE),
				   wordStr2,
				   Colors::get (RESET),
				   shorten (substr (line, column2 + end.length () - 1, line.length ()))
		));
		
		if (line [line.length () - 1] != '\n') buf.write ('\n');
		buf.write (format ("%% ┃ %", Colors::get (BOLD), padd, Colors::get (RESET)));
		for (ulong it = 0 ; it < column - 1 ; it++) {
		    if (line [it] == '\t') buf.write ('\t');
		    else buf.write (' ');
		}

		column2 = column + between.length () + wordStr.length () - 1;
		buf.write (rightJustify ("", wordStr.length (), '^'));
		for (ulong it = column + wordStr.length () - 1 ; it < column2 ; it++) {
		    if (line [it] == '\t') buf.write ('\t');
		    else buf.write (' ');
		}
		
		buf.write (rightJustify ("", wordStr2.length (), '^'));				
		buf.write ('\n');
	    } else addLineEof (buf, word);	    
	}	
	
	std::string addLine (const std::string & msg, const lexing::Word & word, const lexing::Word & end) {
	    OutBuffer buf;
	    buf.write (msg);
	    addLine (buf, word, end);
	    return buf.str ();
	}
	
	ErrorMsg createNote (const lexing::Word & word) {
	    std::string aux = format ("%(b) : ", "Note");
	    return ErrorMsg (word, aux);
	}


	ErrorMsg::ErrorMsg (const lexing::Word & begin, const std::string & msg) :
	    begin (begin),
	    end (lexing::Word::eof ()),
	    msg (msg),
	    one_line (false)
	{}

	ErrorMsg::ErrorMsg (const lexing::Word & begin, const lexing::Word & end, const std::string & msg) :
	    begin (begin),
	    end (end),
	    msg (msg),
	    one_line (false)
	{}

	ErrorMsg::ErrorMsg (const std::string & msg) :
	    begin (lexing::Word::eof ()),
	    end (lexing::Word::eof ()),
	    msg (msg),
	    one_line (true)
	{}

	void ErrorMsg::addNote (const ErrorMsg & other) {
	    this-> notes.push_back (other);
	}

	std::string ErrorMsg::computeMessage () const {
	    std::string ret;
	    if (this-> begin.isEof ()) ret = msg + "\n";
	    else if (this-> end.isEof ()) {
		ret = addLine (this-> msg, this-> begin);
	    } else {
		ret = addLine (this-> msg, this-> begin, this-> end);
	    }

	    int z = 0;
	    bool oneLine = false;
	    for (auto & it : this-> notes) {
		if (!it.isEmpty ()) {
		    ret = Ymir::Error::addNote (this-> begin, ret, it.computeMessage (), z == 0, oneLine);
		    z += 1;
		    oneLine = it.one_line;
		}		
	    }
	    
	    if (z != 0) { // Added some notes
		auto leftLine = center (format ("%", this-> begin.getLine ()), 3, ' ');
		auto padd = center ("", leftLine.length (), ' ');
		ret = ret + format ("%% ┗%* %\n", Colors::get (BOLD), padd, 30, "━", Colors::get (RESET));
	    }
	    
	    		    
	    return ret;
	}
	
	bool ErrorMsg::isEmpty () const {
	    return this-> msg == "";
	}

	const lexing::Word & ErrorMsg::getLocation () const {
	    return this-> begin;
	}

	const std::string & ErrorMsg::getMessage () const {
	    return this-> msg;
	}
	
    }
    
}
