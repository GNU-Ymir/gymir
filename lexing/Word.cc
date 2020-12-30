#include <ymir/lexing/Word.hh>
#include <sstream>
#include <algorithm>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/utils/Memory.hh>
#include <ymir/utils/Colors.hh>
#include <ymir/errors/Error.hh>

namespace lexing {
    
    std::map <std::string, char*> Word::__filenames__;

    Word::Word (const Word & other, const std::string &str) :
	str (str),
	locFile (other.locFile),
	line (other.line),
	column (other.column),
	seek (other.seek),
	isFromString (other.isFromString),
	file (other.file),
	start (other.start)
    {}    

    Word::Word (const Word & other, std::string str, long len) :
	str (str),
	locFile (other.locFile),
	line (other.line),
	column (other.column),
	seek (other.seek),
	_length (len),
	isFromString (other.isFromString),
	file (other.file),
	start (other.start)
    {}

    Word::Word () :	
	str (""),
	locFile (""),
	line (0),
	column (0),
	isFromString (false),
	file (lexing::File::empty ()),
	start (0)
    {}

    Word Word::setLocation (const lexing::File & file, location_t locus) const {
	if (locus != UNKNOWN_LOCATION) {
	    Word ret (*this);
	    ret.locFile = LOCATION_FILE (locus);
	    ret.line = LOCATION_LINE (locus);
	    ret.column = LOCATION_COLUMN (locus);
	    ret.file = file;
	    return ret;
	}
	return *this;
    }

    Word Word::setLocation (const lexing::File & file, std::string filename, ulong line, ulong column, ulong seek) const {
	Word ret (*this);
	ret.locFile = filename;
	ret.line = line;
	ret.column = column;
	ret.seek = seek;
	ret.file = file;
	
	return ret;
    }

    Word Word::setFromString (ulong start) const {
	Word ret (*this);
	ret.isFromString = true;
	ret.start = start;
	return ret;
    }
    
    location_t Word::getLocation () const {
	if (this-> isEof ()) {
	    return BUILTINS_LOCATION;
	} else {
	    auto it = __filenames__.find (this-> locFile);
	    char * name = nullptr;
	    if (it == __filenames__.end ()) {
		char * aux = new char [this-> locFile.length () + 1];

		for (auto it : Ymir::r (0, this-> locFile.length ()))
		    aux [it] = this-> locFile [it];
		aux [this-> locFile.length ()] = '\0';
		__filenames__.emplace (this-> locFile, aux);
		name = aux;
	    } else name = it-> second;
	    
	    linemap_add (line_table, LC_ENTER, 0, name, this-> line);
	    linemap_line_start (line_table, this-> line, 0);
	    auto ret = linemap_position_for_column (line_table, this-> column);
	    linemap_add (line_table, LC_LEAVE, 0, NULL, 0);
	    
	    return ret;
	}
    }

    const std::string & Word::getStr () const {
	return this-> str;
    }

    long Word::length () const  {
	if (this-> _length == -1) return this-> str.length ();
	return this-> _length;
    }
    

    Word Word::setStr (const std::string & s) const {
	Word ret (*this);
	ret.str = s;
	return ret;
    }


    Word Word::setColumn (ulong col) const {
	Word ret (*this);
	ret.column = col;
	return ret;
    }

    ulong Word::getColumn () const {
	return this-> column;
    }

    Word Word::setLine (ulong line) const {
	Word ret (*this);
	ret.line = line;
	return ret;
    }

    ulong Word::getLine () const {
	return this-> line;
    }

    ulong Word::getSeek () const {
	return this-> seek;
    }
    
    bool Word::isEof () const {
	return this-> file.isEmpty ();
    }

    

    std::string Word::toString () const {
	if (this-> isEof ()) return ":\u001B[32meof\u001B[0m()";
	Ymir::OutBuffer buf (Colors::get (BOLD), this-> str, " --> ", this-> locFile.c_str (), ":(", this-> line, ",", this-> column, ")", Colors::get (RESET));
	return buf.str ();
    }

    lexing::File Word::getFile () const {
	return this-> file;
    }

    ulong Word::getStart () const {
	return this-> start;
    }
    
    const std::string & Word::getFilename () const {
	return this-> locFile;
    }

    bool Word::isToken () const {
	auto mem = Token::members ();
	return std::find (mem.begin (), mem.end (), this-> str) != mem.end ();
    }

    bool Word::isSame (const Word & other) const {
	return this-> str == other.str &&
	    this-> locFile == other.locFile &&
	    this-> line == other.line &&
	    this-> column == other.column;    
    }

    bool Word::is (const std::vector <std::string> & values) const {
	return std::find (values.begin (), values.end (), this-> str) != values.end ();
    }
    
    
    void Word::purge () {
	for (auto & it : __filenames__)
	    delete it.second;
    }
    
}

