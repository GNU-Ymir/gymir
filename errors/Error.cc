#include <ymir/errors/Error.hh>
#include <string>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/utils/Memory.hh>
#include <ymir/global/State.hh>
#include <ymir/utils/Colors.hh>
#include <execinfo.h>

using namespace lexing;

namespace Ymir {
    namespace Error {

	unsigned long MAX_ERROR_DEPTH = 4;	
	
	void ErrorList::print () const {
	    Ymir::OutBuffer buf;
	    for (auto it : this-> errors) {
		it.computeMessage (buf, 0, it.computeMaxDepth ());
		buf.writeln ("");
	    }
	    
	    fprintf (stderr, "%s\n", buf.str ().c_str ());
	}

	void FatalError::print () const {
	    fprintf (stderr, "%s\n", this-> msg.c_str ());	    
	}
	
	std::string substr (const std::string& x, ulong beg, ulong end) {
	    if (end - beg > x.length ()) return "";
	    if (beg > x.length ()) return "";
	    return x.substr (beg, end - beg);
	}

	std::string shorten (const std::string & str, ulong max = 100) {
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

	std::string center (const std::string &toCenter, ulong nb, const char * concat) {
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
		buf.write (format ("\n% --> %:(%,%)%\n",
				   Colors::get (BOLD),
				   word.getFilename ().c_str (),
				   word.getLine (),
				   word.getColumn (),
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
		buf.write (format ("%% ╋ %", Colors::get (BOLD), padd, Colors::get (RESET)));
		for (ulong it = 0 ; it < column - 1 ; it++) {
		    if (line [it] == '\t') buf.write ('\t');
		    else buf.write (' ');
		}
		
		buf.write (rightJustify ("", wordStr.length (), '^'));
		buf.write ('\n');
	    } else addLineEof (buf, word);
	}
	
	void addLine (Ymir::OutBuffer & buf, const std::string & msg, const lexing::Word & word) {
	    buf.write (msg);
	    addLine (buf, word);
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
	
	void addNote (Ymir::OutBuffer & buf, const Word & word, const std::string & note) {
	    auto leftLine = center (format ("%", word.getLine ()), 3, ' ');
	    auto padd = center ("", leftLine.length (), ' ');
	    auto padd2 = center ("", leftLine.length (), '-');
	    
	    auto lines = splitString (note, "\n");
	    // auto l = format ("%% ┃ %\n", Colors::get (BOLD), padd, Colors::get (RESET));
	    // buf.write (l);	
	    
	    for (auto it : Ymir::r (0, lines.size ())) {
		if (lines [it].length() != 0) {
		    auto l = format ("%% ┃ %%\n", Colors::get (BOLD), padd, Colors::get (RESET), lines [it]);
		    buf.write (l);
		}
	    }	    
	}
	
	void addTwoLines (OutBuffer & buf, const Word & word, const Word & end) {
	    std::string line = getLine (word.getLine (), word.getFile (), word.getStart ());
	    
	    if (line.length () > 0) {
		auto leftLine = center (format ("%", word.getLine ()), 3, ' ');
		auto padd = center ("", leftLine.length (), ' ');
		buf.write (format ("\n% --> %:(%,%)%\n",
				   Colors::get (BOLD),
				   word.getFilename ().c_str (),
				   word.getLine (),
				   word.getColumn (),
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
		buf.write (format ("\n% --> %:(%,%)%\n",
				   Colors::get (BOLD),
				   word.getFilename ().c_str (),
				   word.getLine (),
				   word.getColumn (),
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
		buf.write (format ("%% ╋ %", Colors::get (BOLD), padd, Colors::get (RESET)));
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
	
	void addLine (Ymir::OutBuffer & buf, const std::string & msg, const lexing::Word & word, const lexing::Word & end) {
	    buf.write (msg);
	    addLine (buf, word, end);
	}
	
	ErrorMsg createNote (const lexing::Word & word) {
	    std::string aux = format ("%(b) : ", "Note");
	    return ErrorMsg (word, aux);
	}


	ErrorMsg::ErrorMsg (const lexing::Word & begin, const std::string & msg) :
	    begin (begin),
	    end (lexing::Word::eof ()),
	    msg (msg),
	    one_line (false),	    
	    windable (false)
	{}

	ErrorMsg::ErrorMsg (const lexing::Word & begin, const lexing::Word & end, const std::string & msg) :
	    begin (begin),
	    end (end),
	    msg (msg),
	    one_line (false),
	    windable (false)
	{}

	ErrorMsg::ErrorMsg (const std::string & msg) :
	    begin (lexing::Word::eof ()),
	    end (lexing::Word::eof ()),
	    msg (msg),
	    one_line (true),
	    windable (false)
	{}

	void ErrorMsg::addNote (const ErrorMsg & other) {
	    if (!other.isEmpty ()) {
		this-> notes.push_back (other);
	    }
	}

	void ErrorMsg::setWindable (bool is) {
	    this-> windable = is;
	}
	
	unsigned long ErrorMsg::computeMaxDepth () const {
	    unsigned long depth = 0;
	    for (auto & it : this-> notes) {
		if (!it.isEmpty ()) {
		    auto nDepth = it.computeMaxDepth ();
		    if (nDepth + 1 > depth) depth = nDepth + 1;
		}
	    }
	    return depth;
	}
		
	void ErrorMsg::computeMessage (Ymir::OutBuffer & buf, unsigned long depth, unsigned long max_depth, bool writtenSub, bool windable) const {
	    windable = windable || this-> windable;
	    Ymir::OutBuffer noteBuf;
	    bool notOneLine = false, addedNote = false;
	    unsigned long int max_padd = 0;
	    unsigned long jt = 1;
	    auto enpadding = depth < (Error::MAX_ERROR_DEPTH) || global::State::instance ().isVerboseActive () || max_depth - depth < Error::MAX_ERROR_DEPTH;
	    auto writeSub = !enpadding && depth == Error::MAX_ERROR_DEPTH && !global::State::instance ().isVerboseActive () && this-> notes.size () != 0 && !writtenSub;
	    for (auto & it : this-> notes) {
		addedNote = true;
		if (jt != this-> notes.size () && jt == Error::MAX_ERROR_DEPTH && !global::State::instance ().isVerboseActive () && windable) {
		    noteBuf.writef ("     : %(B)\n", "...");
		    noteBuf.writef ("%(b) : %\n", "Note", ExternalError::get (OTHER_ERRORS));		    
		} else if (jt < Error::MAX_ERROR_DEPTH || jt == this-> notes.size () || global::State::instance ().isVerboseActive () || !windable) {
		    it.computeMessage (noteBuf, depth + 1, max_depth, writeSub, windable);
		    notOneLine = !it.one_line || it.notes.size () != 0;
		    max_padd = it.begin.getLine ();
		    if (it.end.getLine () > max_padd) {
			max_padd = it.end.getLine ();
		    }
		}
		jt += 1;
	    }
	    
	    if (writeSub) {
		buf.writef ("     : %(B)\n", "...");
		buf.writef ("%(b) : %\n", "Note", ExternalError::get (OTHER_ERRORS));
		buf.write (noteBuf.str ());
	    } else if (enpadding) {
		if (this-> begin.isEof ()) buf.writeln (msg);
		else if (this-> end.isEof ()) {
		    addLine (buf, this-> msg, this-> begin);
		} else {
		    addLine (buf, this-> msg, this-> begin, this-> end);
		}
		
		Ymir::Error::addNote (buf, this-> begin, noteBuf.str ());
		if (addedNote) { // Added some notes
		    auto leftLine = center (format ("%", this-> begin.getLine ()), 3, ' ');
		    auto padd = center ("", leftLine.length (), ' ');
		    auto leftLine2 = center (format ("%", max_padd), 3, ' ');
		    auto padd2 = center ("", leftLine2.length (), "━");
		
		    if (notOneLine) {
			buf.write (format ("%% ┗━%━┻━ %\n", Colors::get (BOLD), padd, padd2, Colors::get (RESET)));
		    } else {
			buf.write (format ("%% ┗━%━━ %\n", Colors::get (BOLD), padd, padd2, Colors::get (RESET)));
		    }
		}
	    } else buf.write (noteBuf.str ());
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

	std::list <ErrorMsg>  ErrorMsg::getNotes () const {
	    return this-> notes;
	}
	
    }
    
}
