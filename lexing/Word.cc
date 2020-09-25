#include <ymir/lexing/Word.hh>
#include <sstream>
#include <algorithm>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/utils/Memory.hh>
#include <ymir/utils/Colors.hh>

namespace lexing {
    Word::Word (location_t locus, const std::string &str) :
	str (str)
    {
	if (locus != UNKNOWN_LOCATION) {
	    this-> locFile = LOCATION_FILE (locus);
	    this-> line = LOCATION_LINE (locus);
	    this-> column = LOCATION_COLUMN (locus);
	}
    }

    Word::Word (const Word & other, const std::string &str) :
	str (str),
	locFile (other.locFile),
	line (other.line),
	column (other.column),
	seek (other.seek),
	isFromString (other.isFromString),
	content (other.content),
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
	content (other.content),
	start (other.start)
    {}


    Word::Word (const Word & other) :
	str (other.str),
	locFile (other.locFile),
	line (other.line),
	column (other.column),
	seek (other.seek),
	_length (other._length),
	isFromString (other.isFromString),
	content (other.content),
	start (other.start)
    {}

    Word& Word::operator=(const Word& other) {
	this-> str = other.str;
	this-> locFile = other.locFile;
	this-> line = other.line;
	this-> column = other.column;
	this-> seek = other.seek;
	this-> _length = other._length;
	this-> isFromString = other.isFromString;
	this-> content = other.content;
	this-> start = other.start;
	return *this;
    }

    Word::Word () :	
	str (""),
	locFile (""),
	line (0),
	column (0)
    {}

    void Word::setLocus (location_t locus) {
	if (locus != UNKNOWN_LOCATION) {
	    this-> locFile = LOCATION_FILE (locus);
	    this-> line = LOCATION_LINE (locus);
	    this-> column = LOCATION_COLUMN (locus);
	}
    }

    void Word::setLocus (std::string filename, ulong line, ulong column, ulong seek) {
	this-> locFile = filename;
	this-> line = line;
	this-> column = column;
	this-> seek = seek;
    }

    void Word::setFromString (const std::string content, ulong start) {
	this-> isFromString = true;
	this-> content = content;
	this-> start = start;
    }
    
    location_t Word::getLocus () const {
	if (this-> isEof ()) {
	    return BUILTINS_LOCATION;
	} else {
	    char * aux = new (Z0) char [this-> locFile.length () + 1];
	    for (auto it : Ymir::r (0, this-> locFile.length ()))
		aux [it] = this-> locFile [it];
	    aux [this-> locFile.length ()] = '\0';
    
	    linemap_add (line_table, LC_ENTER, 0, aux, this-> line);	
	    linemap_line_start (line_table, this-> line, 0);
	    auto ret = linemap_position_for_column (line_table, this-> column);
	    linemap_add (line_table, LC_LEAVE, 0, NULL, 0);
	    return ret;
	}
    }


    std::string Word::toString () const {
	if (this-> isEof ()) return ":\u001B[32meof\u001B[0m()";
	Ymir::OutBuffer buf (Colors::get (BOLD), this-> str, " --> ", this-> locFile.c_str (), ":(", this-> line, ",", this-> column, ")", Colors::get (RESET));
	return buf.str ();
    }

    std::string Word::getFile () const {
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

    bool Word::is (const std::vector <std::string> & values) {
	return std::find (values.begin (), values.end (), this-> str) != values.end ();
    }
    

}

