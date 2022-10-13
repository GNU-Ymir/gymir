#include <ymir/lexing/Word.hh>
#include <sstream>
#include <algorithm>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/utils/Memory.hh>
#include <ymir/utils/Colors.hh>
#include <ymir/errors/Error.hh>

namespace lexing {
    
    std::map <std::string, char*> Word::__filenames__;

    Word Word::__empty__ = Word {new (NO_GC) IWord ("", "", 0, 0, 0, 0, false, lexing::File::empty (), 0, 0)};

    IWord::IWord (const std::string & str, const std::string & locFile, uint64_t line, uint64_t col, uint64_t seek, int64_t len, bool isFromString, const lexing::File & file, uint64_t start, uint64_t self_seek) :	
	str (str),
	locFile (locFile),
	line (line),
	column (col),
	seek (seek),
	self_seek (self_seek),
	length (len),
	isFromString (isFromString),
	file (file),
	start (start)
    {}
    
    Word::Word (IWord * elem) : RefProxy<IWord, Word> (elem)
    {}

    
    Word Word::eof (const std::string & file) {
    	return Word {new (NO_GC) IWord ("", file, 0, 0, 0, -1, false, lexing::File::empty (), 0, 0)};
    }

    Word Word::eof () {
	return __empty__;
    }

    Word Word::init (const std::string & str, const lexing::File & file, uint64_t line, uint64_t col, uint64_t seek) {
	return Word {new (NO_GC) IWord (str, "", line, col, seek, -1, false, file, 0, seek)};
    }


    Word Word::init (const std::string & str, const lexing::File & file, uint64_t line, uint64_t col, uint64_t seek, bool isFromString, uint64_t start, uint64_t self_seek) {
	return Word {new (NO_GC) IWord (str, "", line, col, seek, -1, isFromString, file, start, self_seek)};
    }
    
    Word Word::init (const lexing::Word & other, const std::string & str) {
	return Word {new (NO_GC) IWord (str, other._value-> locFile, other._value-> line, other._value-> column, other._value-> seek, other._value-> length, other._value-> isFromString, other._value-> file, other._value-> start, other._value-> self_seek)};	
    }
    
    Word Word::init (const lexing::Word & other, const std::string & str, uint64_t length) {
	return Word {new (NO_GC) IWord (str, other._value-> locFile, other._value-> line, other._value-> column, other._value-> seek, length, other._value-> isFromString, other._value-> file, other._value-> start, other._value-> self_seek)};	
    }
    
    location_t Word::getLocation () const {
	if (this-> isEof ()) {
	    return BUILTINS_LOCATION;
	} else {
	    auto it = __filenames__.find (this-> getFilename ());
	    char * name = nullptr;
	    if (it == __filenames__.end ()) {
		char * aux = new char [this-> getFilename ().length () + 1];

		for (auto it : Ymir::r (0, this-> getFilename ().length ()))
		    aux [it] = (this-> getFilename ()) [it];
		aux [this-> getFilename ().length ()] = '\0';
		__filenames__.emplace (this-> getFilename (), aux);
		name = aux;
	    } else name = it-> second;
	    
	    linemap_add (line_table, LC_ENTER, 0, name, this-> _value-> line);
	    linemap_line_start (line_table, this-> _value-> line, 0);
	    auto ret = linemap_position_for_column (line_table, this-> _value-> column);
	    linemap_add (line_table, LC_LEAVE, 0, NULL, 0);
	    
	    return ret;
	}
    }

    const std::string & Word::getStr () const {
	return this-> _value-> str;
    }

    int64_t Word::length () const  {	
	if (this-> _value-> length == -1) return this-> _value-> str.length ();
	return this-> _value-> length;
    }
    
    uint64_t Word::getColumn () const {	
	return this-> _value-> column;	
    }

    uint64_t Word::getLine () const {	
	return this-> _value-> line;
    }

    uint64_t Word::getSeek () const {	
	return this-> _value-> seek;
    }

    uint64_t Word::getSelfSeek () const {
	return this-> _value-> self_seek;
    }
    
    bool Word::isEof () const {
	return this-> _value-> file.isEmpty ();
    }
    

    std::string Word::toString () const {
	if (this-> isEof ()) return ":\u001B[32meof\u001B[0m()";
	
	Ymir::OutBuffer buf (Colors::BOLD, this-> _value-> str, " --> ", this-> getFilename (), ":(", this-> _value-> line, ",", this-> _value-> column, ")", Colors::RESET);
	return buf.str ();
    }

    lexing::File Word::getFile () const {
	return this-> _value-> file;
    }

    uint64_t Word::getStart () const {	
	return this-> _value-> start;
    }

    
    const std::string & Word::getFilename () const {	
	if (this-> isEof ()) return this-> _value-> locFile;
	else return this-> _value-> file.getFilename ();
    }

    bool Word::isToken () const {	
	auto & mem = Token::members ();
	return std::find (mem.begin (), mem.end (), this-> _value-> str) != mem.end ();
    }

    bool Word::isSame (const Word & other) const {
       	return this-> _value-> str == other._value-> str &&
	    this-> getFilename () == other.getFilename () &&
	    this-> _value-> line == other._value-> line &&
	    this-> _value-> column == other._value-> column;    
    }

    bool Word::is (const std::vector <std::string> & values) const {	
	return std::find (values.begin (), values.end (), this-> _value-> str) != values.end ();
    }
    
    
    void Word::purge () {
	for (auto & it : __filenames__)
	    delete it.second;
    }
    
}

