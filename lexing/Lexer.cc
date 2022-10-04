#include <ymir/lexing/Lexer.hh>

#include "config.h"
#include "coretypes.h"
#include "input.h"
#include "diagnostic.h"
#include "safe-ctype.h"
#include <ymir/utils/OutBuffer.hh>
#include <ymir/utils/Memory.hh>
#include <ymir/errors/_.hh>
#include <ymir/global/State.hh>
#include <algorithm>
#include <ymir/utils/Benchmark.hh>

namespace lexing {

    using namespace Ymir;
  
    Lexer::Lexer () :
	line (0),
	column (0),
	lineStart (0),
	enableComment (true),
	_tokenizer (Token::members ()),
	current (-1),
	file (lexing::File::empty ()),       
	_fileLocus (Word::eof ()),
	__eof__ (Word::eof (""))
    {
	this-> filename = "";
    }
    
    Lexer :: Lexer (const char * filename,
		    const lexing::File & file, 
		    const std::vector <std::string> &skips,
		    const std::map <std::string, std::pair <std::string, std::string> > &comments
    ) :
	line (1),
	column (1),
	lineStart (0),
	enableComment (true),
	_tokenizer (Token::members ()),	
	current (-1),
	file (file),
	_fileLocus (Word::init ("", file, 0, 0, 0)),
	__eof__ (Word::eof (filename))
    {
	
	for (auto & it : skips) {
	    this-> skips [it] = true;
	}
	
	this-> filename = filename;
	this-> comments = comments;	
	this-> disposed = false;
    }

    
    Lexer Lexer::initFromString (const lexing::File & file, const std::string & filename, const std::vector <std::string> &skips, const std::map <std::string, std::pair <std::string, std::string> > &comments, ulong start) {
	auto lex = Lexer (filename.c_str (), file, skips, comments);
	
	lex.isFromString = true;
	lex.line = start + 1;
	lex.start = start;

	return lex;
    }
   

    Word Lexer::fileLocus () {
	return this-> _fileLocus;
    }

    std::string Lexer::getFilename () const {
	return this-> filename;
    }

    lexing::File Lexer::getFile () const {
	return this-> file;
    }

    void Lexer::skipEnable (const std::string &elem, bool on) {
	this-> skips [elem] = on;
    }

    void Lexer::skipEnable (const std::vector<std::string> &elem, bool on) {
	for (auto it : elem)
	    this-> skips [it] = on;
    }

    
    void Lexer::commentEnable (bool on) {
	this-> enableComment = on;
    }

    Lexer& Lexer::next (Word &word) {
	if (this-> current >= (long) (this-> reads.size ()) - 1) {
	    return this-> get (word);
	    return *this;
	} else {
	    do {
		this-> current ++;
		word = this-> reads [this-> current];
	    } while (isSkip (word) && this-> current < (long) (this-> reads.size ()) - 1);
	    if (isSkip (word)) {
		this-> get (word);
	    }
	    return *this;
	}
    }

    Lexer& Lexer::nextWithDocs (std::string & docs, Word &word) {
	if (this-> current >= (long) (this-> reads.size ()) - 1) {
	    return this-> getWithDocs (docs, word);
	} else {
	    do {
		this-> current ++;
		word = this-> reads [this-> current];
		docs = this-> docs [this-> current];
	    } while (isSkip (word) && this-> current < (long) (this-> reads.size ()) - 1);
	    if (isSkip (word)) return this-> getWithDocs (docs, word);	    
	    return *this;
	}
    }
    
    Word Lexer::nextWithDocs (std::string & docs) {
	Word word = this-> __eof__;
	this-> nextWithDocs (docs, word);
	return word;
    }
    
    Word Lexer::next () {
	Word word = this-> __eof__;
	this-> next (word);
	return word;
    }

    std::string join (const std::vector <std::string> &elems) {
	OutBuffer buf;
	int i = 0;
	for (auto it : elems) {
	    buf.write ("'", it, "'", (i == (int) elems.size () - 1 ? "" : " "));
	    i++;
	}
	return buf.str ();
    }
    
    Word Lexer::next (const std::vector <std::string> &mandatories) {
	Word word = this-> __eof__;
	this-> next (word);
	if (std::find (mandatories.begin (), mandatories.end (), word.getStr ()) != mandatories.end ())
	    return word;	
	
	this-> rewind ();
	std::string val = word.getStr ();
	if (word.isEof ()) val = "\u001B[32mEOF\u001B[0m()";
	Error::occur (word, ExternalError::SYNTAX_ERROR_AT, join (mandatories).c_str (), val);
	
	return this-> __eof__;
    }

    Word Lexer::nextWithDocs (std::string & docs, const std::vector <std::string> &mandatories) {
	Word word = this-> __eof__;
	this-> nextWithDocs (docs, word);
	
	if (std::find (mandatories.begin (), mandatories.end (), word.getStr ()) != mandatories.end ())
	    return word;	
	
	this-> rewind ();
	std::string val = word.getStr ();
	if (word.isEof ()) val = "\u001B[32mEOF\u001B[0m()";
	Error::occur (word, ExternalError::SYNTAX_ERROR_AT, join (mandatories).c_str (), val);
	
	return this-> __eof__;
    }

    Word Lexer::consumeIf (const std::vector <std::string> & optional) {
	auto word = this-> next ();
	if (std::find (optional.begin (), optional.end (), word.getStr ()) != optional.end ())
	    return word;
	
	this-> rewind ();
	return this-> __eof__;
    }
    
    Lexer& Lexer::rewind (ulong nb) {
	this-> current -= nb;
	if (this-> current < -1) this-> current = -1;
	return *this;
    }

    ulong Lexer::tell () const {
	return this-> current;
    }

    void Lexer::seek (ulong where) {
	this-> current = where;
    }

    Lexer& Lexer::get (Word &word) {
	OutBuffer buf;
	do {
	    if (!getWord (word)) {
		word = this-> __eof__;
		break;
	    } else {
		std::string com, ign;
		bool line_break = false;
		while (isComment (word, com, ign) && this-> enableComment) {
		    do {
			word = this-> __eof__;
			getWord (word);
			if (word.getStr () != com && !word.isEof ()) {
			    if (global::State::instance ().isDocDumpingActive ())  {
				if (word.getStr () == Token::RETURN) {
				    buf.write ("\\n"); line_break = true;
				} else if (!line_break || (word.getStr () != ign && line_break)) {
				    line_break = line_break ? word.getStr () == Token::SPACE : false;
				    buf.write (word.getStr ());
				}
			    }
			}
		    } while (word.getStr () != com && !word.isEof ());
		    getWord (word);
		}
	    }
	} while (isSkip (word) && !word.isEof ());

	this-> reads.push_back (word);
	this-> docs.push_back (buf.str ());
	this-> current ++;
	return *this;
    }

    Lexer& Lexer::getWithDocs (std::string & docs, Word &word) {
	docs = "";
	OutBuffer buf;
	do {
	    if (!getWord (word)) {
		word = this-> __eof__;
		break;
	    } else {
		std::string com, ign;
		bool line_break = false;
		while (isComment (word, com, ign) && this-> enableComment) {
		    do {
			word = this-> __eof__;
			getWord (word);
			if (word.getStr () != com && !word.isEof ()) {
			    if (global::State::instance ().isDocDumpingActive ()) {
				if (word.getStr () == Token::RETURN) {
				    buf.write ("\\n");
				    line_break = true;
				} else if ((line_break && word.getStr () != ign) || !line_break) {
				    line_break = line_break ? word.getStr () == Token::SPACE : false;
				    buf.write (word.getStr ());
				}
			    }
			}
		    } while (word.getStr () != com && !word.isEof ());
		    getWord (word);
		}
	    }
	} while (isSkip (word) && !word.isEof ());

	docs = buf.str ();
	this-> reads.push_back (word);
	this-> docs.push_back (docs);
	this-> current ++;
	return *this;
    }
    
    bool Lexer::isComment (const Word& elem, std::string & retour, std::string & ignore) const {
	auto end_comm = this-> comments.find (elem.getStr ());	
	if (end_comm == this-> comments.end ()) return false;
	else {
	    retour = end_comm-> second.first;
	    ignore = end_comm-> second.second;
	    return true;
	}
    }

    bool Lexer::isSkip (const Word &elem) const {
	auto it = this-> skips.find (elem.getStr ());
	if  (it == this-> skips.end ()) return false;
	else return it-> second;
    }

    bool Lexer::getWord (Word& word) {
	if (this-> _cache.empty ()) {
	    this-> _cache = this-> readLine ();
	}

	if (this-> _cache.empty ()) {
	    return false;	    
	} else {	    
	    word = this-> _cache.front ();
	    this-> _cache.pop_front ();
	    return true;
	}
    }
    
    std::list <Word> Lexer::readLine () {
	if (this-> file.isEof ()) return {};
	// auto where = this-> file.tell ();
	// auto line_start = where;
	auto line  = std::move (this-> file.readln ());
	if (line == "") return {};
	
	ulong start = 0;
	std::list <Word> result;
	ulong where = this-> lineStart;
	while (start < line.length ()) {
	    auto len = this-> _tokenizer.next (start, line);
	    auto it = std::move (line.substr (start, len));
	    result.push_back (Word::init (it, this-> file, this-> line, this-> column, this-> lineStart, this-> isFromString, this-> start, where));
	    start += len;
	    where = where + len;
	    if (it  == "\n") {
		this-> line ++;
		this-> column = 1;
	    } else {
		this-> column += len; 
	    }	    
	}
	
	this-> lineStart += line.length ();
	return result;
    }

    std::string Lexer::formatRestOfFile () {
	auto tell = this-> file.tell ();
	if (this-> current < (long) (this-> reads.size ()) - 1) {
	    if (this-> reads [this-> current + 1].isEof ()) return "";
	    this-> file.seek (this-> reads [this-> current + 1].getSeek ());
	} 

	auto end = this-> file.readAll ();
	this-> file.seek (tell);
	return end;
    }

    
}



