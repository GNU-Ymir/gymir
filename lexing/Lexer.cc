#include <ymir/lexing/Lexer.hh>

#include "config.h"
#include "coretypes.h"
#include "input.h"
#include "diagnostic.h"
#include "safe-ctype.h"
#include <ymir/utils/OutBuffer.hh>
#include <ymir/utils/Memory.hh>
#include <ymir/errors/_.hh>

namespace lexing {

    using namespace Ymir;
    
    std::string readln (FILE * i) {
	unsigned long max = 255;
	std::string final = "";
	while (1) {
	    char * buf = new (Z0) char[max];
	    char * aux = fgets(buf, max, i);
	    if (aux == NULL) {
		delete buf;
		return "";
	    }
	    
	    std::string ret = std::string (buf);
	    final += ret;
	    
	    delete  buf;
	    
	    if (ret.size () != max - 1) return final;
	    else max *= 2;      
	}
    }

    std::string read (FILE * i) {
	auto tell = ftell (i);
	fseek (i, 0, SEEK_END);
	auto length = ftell (i) - tell;
	char * buf = new (Z0) char[length];
	fseek (i, tell, SEEK_SET);
	
	auto size = fread (buf, 1, length, i);
	buf [size] = '\0';
	std::string ret = std::string (buf);
	
	delete  buf;
	return ret;
    }

    Lexer::Lexer () :
	line (0),
	column (0),
	enableComment (false),
	current (-1)
    {
	this-> filename = "";
	this-> file = NULL;
    }
    
    Lexer :: Lexer (const char * filename,
		    FILE * file, 
		    const std::vector <std::string> &skips,
		    const std::map <std::string, std::pair <std::string, std::string> > &comments
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


    Lexer :: Lexer (const Lexer & lex) {
	this-> line = lex.line;
	this-> column = lex.column;
	this-> enableComment = lex.enableComment;
	this-> disposed = lex.disposed;
	this-> filename = lex.filename;
	this-> skips = lex.skips;
	this-> tokens = lex.tokens;
	this-> comments = lex.comments;
	this-> reads = lex.reads;
	this-> docs = lex.docs;
	this-> current = lex.current;

	if (lex.isFromString) {
	    auto name = tmpnam (new char [L_tmpnam]);	
	    this-> file = fopen (name, "w");
	    fseek (this-> file, 0, SEEK_SET);
	    fwrite (lex.content.c_str (), lex.content.length (), sizeof (char), this-> file);
	    fclose (this-> file);

	    this-> file = fopen (name, "r");
	    fseek (this-> file, ftell (lex.file), SEEK_SET);
	    this-> string_name = name;
	    delete name;
	} else this-> file = lex.file;
	
	this-> line_map = lex.line_map;
	this-> content = lex.content;
	this-> isFromString = lex.isFromString;
	this-> start = lex.start;
    }

    const Lexer & Lexer :: operator= (const Lexer & lex) {
	this-> line = lex.line;
	this-> column = lex.column;
	this-> enableComment = lex.enableComment;
	this-> disposed = lex.disposed;
	this-> filename = lex.filename;
	this-> skips = lex.skips;
	this-> tokens = lex.tokens;
	this-> comments = lex.comments;
	this-> reads = lex.reads;
	this-> docs = lex.docs;
	this-> current = lex.current;

	if (lex.isFromString) {
	    auto name = tmpnam (new char [L_tmpnam]);	
	    this-> file = fopen (name, "w");
	    fseek (this-> file, 0, SEEK_SET);
	    fwrite (lex.content.c_str (), lex.content.length (), sizeof (char), this-> file);
	    fclose (this-> file);

	    this-> file = fopen (name, "r");
	    fseek (this-> file, ftell (lex.file), SEEK_SET);
	    this-> string_name = name;
	    
	    delete name;
	} else {
	    this-> file = lex.file;
	}
	
	this-> line_map = lex.line_map;
	this-> content = lex.content;
	this-> isFromString = lex.isFromString;
	this-> start = lex.start;
	return lex;
    }
    
    Lexer Lexer::initFromString (const std::string & content, const std::string & filename, const std::vector <std::string> &skips, const std::map <std::string, std::pair <std::string, std::string> > &comments, ulong start) {
	auto name = tmpnam (new char [L_tmpnam]);
	FILE * tmp = fopen (name, "w");
	fwrite (content.c_str (), content.length (), sizeof (char), tmp);

	fclose (tmp);
	tmp = fopen (name, "r");
	
	auto lex = Lexer (filename.c_str (), tmp, skips, comments);
	
	lex.content = content;
	lex.isFromString = true;
	lex.line = start;
	lex.start = start;
	lex.string_name = name;	

	delete name;	
	return lex;
    }

    void Lexer::dispose () {
	if (isFromString) {
	    fclose (this-> file);
	    remove (this-> string_name.c_str ());
	}
    }
    
    Lexer :: ~Lexer () {
	dispose ();
    }


    Word Lexer::fileLocus () {
	Word loc;
	loc.setLocus (this-> filename, 0, 0, 0);
	return loc;
    }

    std::string Lexer::getFilename () const {
	return this-> filename;
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
	} else {
	    do {
		this-> current ++;
		word = this-> reads [this-> current];
	    } while (isSkip (word) && this-> current < (long) (this-> reads.size ()) - 1);
	    if (isSkip (word)) return this-> get (word);	    
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
	Word word;
	this-> nextWithDocs (docs, word);
	return word;
    }
    
    Word Lexer::next () {
	Word word;
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
	Word word;
	this-> next (word);
	for (auto it : mandatories) {
	    if (it == word.getStr ()) return word;
	}
	
	this-> rewind ();
	Error::occur (word, ExternalError::get (SYNTAX_ERROR_AT), join (mandatories).c_str (), word.str);
	
	return Word::eof (this-> filename);
    }

    Word Lexer::nextWithDocs (std::string & docs, const std::vector <std::string> &mandatories) {
	Word word;
	this-> nextWithDocs (docs, word);
	for (auto it : mandatories) {
	    if (it == word.getStr ()) return word;
	}
	
	this-> rewind ();
	Error::occur (word, ExternalError::get (SYNTAX_ERROR_AT), join (mandatories).c_str (), word.str);
	
	return Word::eof (this-> filename);
    }

    Word Lexer::consumeIf (const std::vector <std::string> & optional) {
	auto word = this-> next ();
	for (auto it : optional) {
	    if (it == word.getStr ()) return word;
	}
	
	this-> rewind ();
	return {word, ""};
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
		word.setEof (this-> filename);
		break;
	    } else {
		std::string com, ign;
		bool line_break = false;
		while (isComment (word, com, ign) && this-> enableComment) {
		    do {
			word.setEof (this-> filename);
			getWord (word);
			if (word.getStr () != com && !word.isEof ()) {
			    if (word.getStr () == Token::RETURN) {
				buf.write ("\\n"); line_break = true;
			    } else if (!line_break || (word.getStr () != ign && line_break)) {
				line_break = line_break ? word.getStr () == Token::SPACE : false;
				buf.write (word.getStr ());
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
		word.setEof (this-> filename);
		break;
	    } else {
		std::string com, ign;
		bool line_break = false;
		while (isComment (word, com, ign) && this-> enableComment) {
		    do {
			word.setEof (this-> filename);
			getWord (word);
			if (word.getStr () != com && !word.isEof ()) {
			    if (word.getStr () == Token::RETURN) {
				buf.write ("\\n");
				line_break = true;
			    } else if ((line_break && word.getStr () != ign) || !line_break) {
				line_break = line_break ? word.getStr () == Token::SPACE : false;
				buf.write (word.getStr ());
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
    
    bool Lexer::isComment (const Word& elem, std::string & retour, std::string & ignore) {
	auto end_comm = this-> comments.find (elem.getStr ());
	if (end_comm == this-> comments.end ()) return false;
	else {
	    retour = end_comm-> second.first;
	    ignore = end_comm-> second.second;
	    return true;
	}
    }

    bool Lexer::isSkip (const Word &elem) {
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
	if (word.getStr () == "\n") {
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

    void Lexer::constructWord (Word & word, ulong beg, ulong max, const std::string& line, ulong where) {
	if (this-> isFromString) {
	    word.setFromString (this-> content, this-> start);	    
	}

	if (beg == line.length () + 1) word.setStr (line);
	else if (beg == 0) {
	    word.setStr (line.substr (0, min (max, line.length ())));
	    fseek (this-> file, where + max, SEEK_SET);	    
	} else if (beg > 0) {
	    word.setStr (line.substr (0, min (beg, line.length ())));
	    fseek (this-> file, where + beg, SEEK_SET);
	}
	word.setLocus (this-> filename, this-> line, this-> column, where);
    }

    std::string Lexer::formatRestOfFile () {
	auto tell = ftell (this-> file);
	if (this-> current < (long) (this-> reads.size ()) - 1) {
	    if (this-> reads [this-> current + 1].isEof ()) return "";
	    fseek (this-> file, this-> reads [this-> current + 1].seek, SEEK_SET);
	} 

	auto end = read (this-> file);
	fseek (this-> file, tell, SEEK_SET);
	return end;
    }   

    const std::string & Lexer::getContent () const {
	return this-> content;
    }

    const std::string & Lexer::getStringName () const {
	return this-> string_name;
    }
    
}



