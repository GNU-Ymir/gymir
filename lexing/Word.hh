#pragma once

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "File.hh"

#include <string>
#include <tr1/memory>
#include <stdlib.h>
#include <ymir/lexing/File.hh>
#include <ymir/lexing/Token.hh>
#include <map>

namespace lexing {
    /**
     * \struct Word 
     * A word is a string with location information
     */
    struct Word {

    private : 
	
	static std::map <std::string, char*> __filenames__;
	
	/// The content of the word
	std::string str;

	/// The file where the word is located
	std::string locFile;

	/// The line where the word is located
	ulong line;

	/// The column where the word is located
	ulong column;

	/** the cursor in file*/
	ulong seek;

	/// The length of the word (may differ with str.length ())
	long _length = -1;

	/// is from a string or a file
	bool isFromString = false;
	
	lexing::File file;

	ulong start = 0;
	
    public:

	/**
	 * EOF
	 */
	Word ();
	
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
    
		
        /**
	 * Create a new word with another location
	 */
	Word setLocation (const lexing::File & file, location_t locus) const;

	/**
	 * Create a new word with another location
	 */
	Word setLocation (const lexing::File & file, std::string filename, ulong line, ulong column, ulong seek) const;

	/**
	 * Create a new word that came from a string file
	 */
	Word setFromString (ulong start) const;

	/**
	 * Get the location of the word for GCC internals
	 */
	location_t getLocation () const;

	/**
	 * Get the content of the word
	 */
	const std::string & getStr () const ;
	
	/**
	 * Get the length of the word
	 */
	long length () const;

	/**
	 * Create a new word with a different str content
	 */
	Word setStr (const std::string & other) const;

	/**
	 * Create a new word with a different col content
	 */
	Word setColumn (ulong col) const;

	/**
	 * @return: the column location of the word
	 */
	ulong getColumn () const;

	/**
	 * Create a new word with a different line content
	 */
	Word setLine (ulong line) const;
	
	/**
	 * @return: the line location of the word
	 */
	ulong getLine () const;

	/**
	 * @return: the cursor position of the word within the file
	 */
	ulong getSeek () const;
	
	/**
	 * Get the file that created the word
	 */
	lexing::File getFile () const;  

	/**
	 * Get the start line of the file containing the word
	 */
	ulong getStart () const;
	
	/**
	 * Get the name of the file that created the word
	 */
	const std::string & getFilename () const;

	/**
	 * Create a an empty word 
	 */
	static Word eof (const std::string & file) {
	    auto ret = Word {};
	    ret.str = "";
	    ret.line = 0;
	    ret.locFile = file;
	    return ret;
	}

	/**
	 * Create an empty word
	 */
	static Word eof () {
	    auto ret = Word {};
	    ret.str = "";
	    ret.line = 0;
	    return ret;
	}
    
	bool isEof () const;
    

	bool isToken () const;
    
	bool isSame (const Word& other) const;

	bool is (const std::vector<std::string> & vals) const;
	
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
	
	std::string toString () const;    

	static void purge ();
	
    };

}
