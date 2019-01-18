#pragma once

#include "config.h"
#include "system.h"
#include "coretypes.h"

#include <string>
#include <tr1/memory>
#include <stdlib.h>
#include <ymir/utils/Memory.hh>
#include <ymir/lexing/Token.hh>

namespace lexing {
    /**
     * \struct Word 
     * A word is a string with location information
     */
    struct Word {

	/// The content of the word
	std::string str;

	/// The file where the word is located
	std::string locFile;

	/// The line where the word is located
	ulong line;

	/// The column where the word is located
	ulong column;

	/// The length of the word (may differ with str.length ())
	long _length = -1;
    
    public:

	/**
	 * Construct a word from a GCC internal location information
	 * \param locus the location
	 * \param str the content
	 */
	Word (location_t locus, const std::string &str);

	/**
	 * Copy a second word location 
	 * \param str the content of the word
	 */
	Word (const Word & other, const std::string &str);

	/**
	 * Copy a second word location 
	 * \param str the content of the word
	 * \param len the length overriding word content length
	 */
	Word (const Word & other, std::string str, long len);
    
	Word (const Word & other);

	/**
	 * EOF
	 */
	Word ();
    
	Word& operator=(const Word&);
    
	void setLocus (location_t locus);

	void setLocus (std::string filename, ulong line, ulong column);
    
	location_t getLocus () const;
    
	const std::string &getStr () const {
	    return this-> str;
	}

	long length () const {
	    if (this-> _length == -1) return this-> str.length ();
	    return this-> _length;
	}
    
	void setStr (std::string other) {
	    this-> str = other;	
	}

	std::string getFile () const;  
    
	static Word eof (std::string file) {
	    auto ret = Word {};
	    ret.str = "";
	    ret.line = 0;
	    ret.locFile = file;
	    return ret;
	}

	static Word eof () {
	    auto ret = Word {};
	    ret.str = "";
	    ret.line = 0;
	    return ret;
	}
    
	bool isEof () const {
	    return this-> line == 0 && this-> str == "";
	}

	void setEof () {
	    this-> line = 0;
	    this-> str = "";
	}
    
	void setEof (std::string file) {
	    this-> line = 0;
	    this-> str = "";
	    this-> locFile = file;
	}

	bool isToken () const;
    
	bool isSame (const Word& other) const;

	bool is (const std::vector<std::string> & vals);
	
	friend bool operator== (const Word& elem, const char* sec) {
	    return elem.getStr () == sec;
	}
    
	friend bool operator== (const Word& elem, std::string& sec) {
	    return elem.getStr () == sec;
	}

	friend bool operator== (const Word& elem, const Word& sec) {
	    return elem.getStr () == sec.getStr ();
	}

	friend bool operator!= (const Word& elem, const Word& sec) {
	    return elem.getStr () != sec.getStr ();
	}

	friend bool operator!= (const Word& elem, const char* sec) {
	    return elem.getStr () != sec;
	}
    
	friend bool operator!= (const Word& elem, std::string& sec) {
	    return elem.getStr () != sec;
	}

	friend Word operator+ (const Word & left, const Word & right) {
	    if (!left.isEof ()) 
		return {left, left.str + right.str};
	    return right;
	}

	Word& operator+= (const Word & right) {
	    if (!this-> isEof ()) {
		this-> str += right.str;
	    } else {
		this-> str = right.str;
		this-> locFile = right.locFile;
		this-> line = right.line;
		this-> column = right.column;
		this-> _length = right._length;
	    }
	    return *this;
	}
	
	std::string toString () const;    
    
    };

}
