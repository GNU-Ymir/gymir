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
#include <memory>

namespace lexing {

    class Word;

    /**
     * \struct IWord 
     * A word is a string with location information
     */
    class IWord {
    private : 
       	
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

	ulong self_seek;
	
	/// The length of the word (may differ with str.length ())
	long length = -1;

	/// is from a string or a file
	bool isFromString = false;
	
	lexing::File file;

	ulong start = 0;

	/**
	 * Create a word with all field initialized
	 */
	IWord (const std::string & str, const std::string & locFile, ulong line, ulong col, ulong seek, long len, bool isFromString, const lexing::File & file, ulong start, ulong self_seek);
	
	
	friend Word;

	friend bool operator == (const IWord & elem, const char * sec) {
	    return elem.str == sec;
	}

	friend bool operator == (const IWord & elem, const std::string & sec) {
	    return elem.str == sec;
	}

	friend bool operator == (const IWord & elem, const IWord & sec) {
	    return elem.str == sec.str;
	}	
	
    };

    class Word : public RefProxy <IWord, Word> {
    private : 

	static Word __empty__;
	
	static std::map <std::string, char*> __filenames__;
	
	Word (IWord * val);
	
    public : 

	/**
	 * Create a an empty word 
	 */
	static Word eof (const std::string & file);
	
	/**
	 * Create an empty word
	 */
	static Word eof ();

	/**
	 * Create a new word
	 */
	static Word init (const std::string & str, const lexing::File & file, ulong line, ulong col, ulong seek);

	/**
	 * Create a new from string
	 */
	static Word init (const std::string & str, const lexing::File & file, ulong line, ulong col, ulong seek, bool isFromString, ulong start, ulong self_seek);
	

	/**
	 * Create a copy of other with a different string content
	 */
	static Word init (const Word & other, const std::string & str);

	/**
	 * Create a copy of other with a different string content, and length
	 */
	static Word init (const Word & other, const std::string & str, ulong length);
	
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
	 * @return: the column location of the word
	 */
	ulong getColumn () const;
	
	/**
	 * @return: the line location of the word
	 */
	ulong getLine () const;

	/**
	 * @return: the cursor position of the line containing the word in the file
	 */
	ulong getSeek () const;

	/**
	 * @return the cursor position of the word in the file
	 */
	ulong getSelfSeek () const;
	
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
	 * Is this word pointing to nowhere
	 */
	bool isEof () const;    

	/**
	 * Is this word containing a string that can be found in Token::members ()?
	 */
	bool isToken () const;

	/**
	 * Complete equality between two Words (str and location)
	 */
	bool isSame (const Word& other) const;

	/**
	 * Is this word containing a string that can be found in vals?
	 */
	bool is (const std::vector<std::string> & vals) const;
	
	friend bool operator== (const Word& elem, const char* sec) {
	    return *(elem._value) == sec;
	}
    
	friend bool operator== (const Word& elem, const std::string& sec) {
	    return *(elem._value) == sec;
	}

	friend bool operator== (const Word& elem, const Word& sec) {
	    return *(elem._value) == *(sec._value);
	}

	friend bool operator!= (const Word& elem, const Word & sec) {
	    return !(elem == sec);
	}

	friend bool operator!= (const Word& elem, const char* sec) {
	    return !(elem == sec);
	}
    
	friend bool operator!= (const Word& elem, const std::string& sec) {
	    return !(elem == sec);
	}
	
	std::string toString () const;    

	static void purge ();	
	
    };
    
}
