#include "Lexer.hh"

#include "config.h"
#include "coretypes.h"
#include "input.h"
#include "diagnostic.h"
#include "safe-ctype.h"

namespace Lexical {

    Lexer :: Lexer (const char * filename, FILE * input)
	: currentLine(1),
	  currentColumn (1),
	  currentWord (-1),
	  line_map (NULL)
    {
	line_map = ::linemap_add (::line_table, ::LC_ENTER,
				  /* sysp */0, filename,
				  /* currentLine */ 1);
	this -> input = input;
    }

    Lexer :: ~Lexer () {
	::linemap_add (::line_table, ::LC_LEAVE,
		       /* sysp */ 0,
		       /* filename */ NULL,
		       /* to_line */0);
    }

    location_t Lexer::getCurrentLocation () {
	return linemap_position_for_column (::line_table, currentColumn);
    }

    TokenPtr Lexer::next () {
	if (this->currentWord >= (long)read.size () - 1) {
	    TokenPtr ret;
	    do {
		ret = Token::makeEof();
		ret = get ();
		TokenId com;
		if ((com = isComment (ret)) != Token::EOF_TOKEN () && commentOn) {
		    do {
			ret = Token::makeEof ();
			ret = get ();	    
		    } while (ret->getId () != com && ret->getId () != Token::EOF_TOKEN());
		    ret = get ();
		}
	    } while (isSkip (ret) && ret->getId () != Token::EOF_TOKEN());
	    currentWord ++;
	    read.push_back (ret);
	    return ret;
	} else {	    
	    this->currentWord ++;
	    return read [currentWord];
	}
    }

    TokenId Lexer::isComment (TokenPtr tok) {
	auto val = tok-> getStr ();
	for (auto & it : comments) {
	    if (it.first == val) {
		return (getFromStr (it.second));
	    }
	}
	return Token::EOF_TOKEN ();
    }

    bool Lexer::isSkip (TokenPtr tok) {       
	for (auto & it : skip) {
	    if (it == tok->getStr()) return true;
	}
	return false;
    }

    void Lexer::setSkip (std::vector < std::string > skip) {
	this->skip = skip;
    }

    void Lexer::removeSkip (std::string elem) {
	std::vector <std::string> aux;
	for (auto & it : skip) {
	    if (it != elem) aux.push_back (it);
	}
	skip = aux;
    }

    void Lexer::addSkip (std::string elem) {
	skip.push_back (elem);
    }
    
    void Lexer::setKeys (std::vector <std::string> keys) {
	this->keys = keys;
    }
    
    void Lexer::rewind (unsigned long nb) {
	auto re = (long)this->currentWord - (long)nb;
	if (re < 0) currentWord = -1;
	else currentWord = re;
    }
  
    void Lexer::setComments (std::vector < std::pair < std::string, std::string > > comments) {
	this->comments = comments;
    }

    void Lexer::setComments (bool on) {
	this->commentOn = on;
    }
  
    std::string Lexer::readln (FILE * i) {
	unsigned long max = 255;
	std::string final = "";
	while (1) {
	    char * buf = new char[max];
	    char * aux = fgets(buf, max, i);
	    if (aux == NULL) return "";
	    std::string ret = std::string (buf);
	    final += ret;
	    free (buf);
	    if (ret.size () != max - 1) return final;
	    else max *= 2;      
	}
    }
  
    TokenPtr Lexer::get () {
	ulong where = this->tellg ();
	std::string line = readln (input);
	TokenPtr token = Token::makeEof();
	if (this->eof () || line == "") return Token::makeEof ();
	ulong max = 0, beg = line.length ();
	for (auto & it : keys) {
	    long id = line.find (it);
	    if (id != -1) {
		if ((ulong)id < beg) { beg = id; max = it.length(); }
		else if ((ulong)id == beg && it.length() > max)
		    max = it.length();
	    }
	}
	
	if (beg == line.length() + 1) {
	    token = Token::make (line, this->getCurrentLocation());
	    this->seek (line, where + line.length());
	} else if (beg == 0) {
	    std::string info = line.substr (0, max);
	    token = Token::make (info, this->getCurrentLocation());	    
	    this->seek (info, where + max);
	} else if (beg > 0) {
	    std::string info = line.substr (0, beg);
	    token = Token::make(info, this->getCurrentLocation ());
	    this->seek (info, where + beg);
	}
	return token;
    }

  
    void Lexer::seek (const std::string & info, long pos) {
	if (info == "\n" || info == "\r") {
	    this -> currentLine++;
	    this -> currentColumn = 1;
	    linemap_line_start (::line_table, currentLine, maxColumnHint);
	} else this->currentColumn += info.length ();
	fseek (input, pos, SEEK_SET);
    }

    bool Lexer::eof () {
	return feof (input) != 0;
    }

    long Lexer::tellg () {
	return ftell (input);
    }


}
