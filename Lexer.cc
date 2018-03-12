#include "syntax/Lexer.hh"
#include <ymir/syntax/FakeLexer.hh>

#include "config.h"
#include "coretypes.h"
#include "input.h"
#include "diagnostic.h"
#include "safe-ctype.h"
#include <sstream>
#include "errors/_.hh"

namespace lexical {
       
    std::string readln (FILE * i) {
	unsigned long max = 255;
	std::string final = "";
	while (1) {
	    char * buf = new (Z0) char[max];
	    char * aux = fgets(buf, max, i);
	    if (aux == NULL) return "";
	    std::string ret = std::string (buf);
	    final += ret;
	    delete [] buf;
	    if (ret.size () != max - 1) return final;
	    else max *= 2;      
	}
    }  
    
    Lexer :: Lexer (const char * filename,
		    FILE * file, 
		    std::vector <std::string> skips,
		    std::vector <std::pair <std::string, std::string> > comments
    ) :
	line (1),
	column (1),
	enableComment (true),
	current (-1)
    {
	
	for (auto it : skips) {
	    this-> skips [it] = true;
	}
	
	this-> filename = filename;
	this-> tokens = Token::members ();
	this-> comments = comments;	
	this -> file = file;
	this-> disposed = false;
    }

    void Lexer::dispose () {
    }
    
    Lexer :: ~Lexer () {
	dispose ();
    }


    Word Lexer::fileLocus () {
	Word loc;
	loc.setLocus (this-> filename, 0, 0);
	return loc;
    }

    std::string Lexer::getFilename () const {
	return this-> filename;
    }

    void Lexer::skipEnable (std::string elem, bool on) {
	this-> skips [elem] = on;
    }

    void Lexer::commentEnable (bool on) {
	this-> enableComment = on;
    }

    Lexer& Lexer::next (Word &word) {
	if (this-> current >= (long) (this-> reads.size ()) - 1) {
	    return this-> get (word);
	} else {
	    this-> current ++;
	    word = this-> reads [this-> current];
	    return *this;
	}
    }

    Word Lexer::next () {
	Word word;
	this-> next (word);
	return word;
    }


    std::string join (std::vector <std::string> elems) {
	std::stringstream ss;
	int i = 0;
	for (auto it : elems) {
	    ss << "'" << it << "'" << (i == (int) elems.size () - 1 ? "" : " ");
	    i++;
	}
	return ss.str ();
    }
    
    Word Lexer::next (std::vector <std::string> mandatories) {
	Word word;
	this-> next (word);
	for (auto it : mandatories) {
	    if (it == word.getStr ()) return word;
	}

	Ymir::Error::syntaxError (word, join (mandatories).c_str ());
	
	return Word::eof ();
    }

    Lexer& Lexer::rewind (ulong nb) {
	this-> current -= nb;
	if (this-> current < -1) this-> current = -1;
	return *this;
    }

    ulong Lexer::tell () {
	return this-> current;
    }

    void Lexer::seek (ulong where) {
	this-> current = where;
    }

    Lexer& Lexer::get (Word &word) {
	do {
	    if (!getWord (word)) {
		word.setEof ();
		break;
	    } else {
		std::string com;
		while (isComment (word, com) && this-> enableComment) {
		    do {
			getWord (word);
		    } while (word.getStr () != com && !word.isEof ());
		    getWord (word);
		}
	    }
	} while (isSkip (word) && !word.isEof ());

	this-> reads.push_back (word);
	this-> current ++;
	return *this;
    }

    bool Lexer::isComment (Word elem, std::string & retour) {
	for (auto it : this-> comments) {
	    if (it.first == elem.getStr ()) {
		retour = it.second;
		return true;
	    }
	}
	return false;
    }

    bool Lexer::isSkip (Word elem) {
	for (auto it : this-> skips) {
	    if (it.first == elem.getStr ()) return it.second;
	}
	return false;
    }

    bool Lexer::getWord (Word & word) {
	if (feof (this-> file) != 0) return false;
	auto where = ftell (this-> file);
	auto line = readln (this-> file);
	if (line == "") return false;
	ulong max = 0, beg = line.length ();
	for (auto it : this-> tokens) {
	    auto id = line.find (it);
	    if (id != std::string::npos) {
		if (id == beg && it.length () > max) max = it.length ();
		else if (id < beg) {
		    beg = id;
		    max = it.length ();
		}
	    }
	}
	constructWord (word, beg, max, line, where);
	if (word.getStr () == "\n" || word.getStr () == "\r") {
	    this-> line ++;
	    this-> column = 1;
	} else {
	    this-> column += word.getStr ().length ();
	}
	return true;
    }

    ulong Lexer::min (ulong u1, ulong u2) {
	return u1 < u2 ? u1 : u2;
    }

    void Lexer::constructWord (Word & word, ulong beg, ulong max, std::string line, ulong where) {
	if (beg == line.length () + 1) word.setStr (line);
	else if (beg == 0) {
	    word.setStr (line.substr (0, min (max, line.length ())));
	    fseek (this-> file, where + max, SEEK_SET);	    
	} else if (beg > 0) {
	    word.setStr (line.substr (0, min (beg, line.length ())));
	    fseek (this-> file, where + beg, SEEK_SET);
	}
	word.setLocus (this-> filename, this-> line, this-> column);
    }
    

    FakeLexer::FakeLexer (const std::vector <Word> & words) :
	Lexer ("", NULL,
	       {Token::SPACE, Token::RETURN, Token::RRETURN, Token::TAB},
	       {
		   {Token::LCOMM1, Token::RCOMM1},
		       {Token::LCOMM2, Token::RETURN},
			   {Token::LCOMM3, Token::RCOMM3}
	       }),
	words (words),
	fake_current (0)	
    {}

    bool FakeLexer::getWord (Word & word) {
	if (this-> fake_current < this-> words.size ()) {
	    word = this-> words [fake_current];
	    fake_current ++;
	    return true;
	} else fake_current++;	
	return false;
    }

    void FakeLexer::cutCurrentWord (ulong beg) {
	this-> reads [this-> current].setStr (
	    this-> reads [this-> current].getStr ().substr (beg)
	);
	this-> reads [this-> current].column += beg;
	this-> rewind ();
    }
    
    std::string FakeLexer::toString () {
	return Ymir::OutBuffer ("[", this-> fake_current, "] ", this-> words).str ();
    }
    
}



