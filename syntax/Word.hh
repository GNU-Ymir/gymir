#pragma once

#include "config.h"
#include "system.h"
#include "coretypes.h"

#include <string>
#include <tr1/memory>
#include <stdlib.h>
#include <ymir/utils/memory.hh>
#include "Token.hh"

struct Word {
private:
    
    location_t locus;
    
    std::string str;
    std::string locFile;
    
public:

    Word (location_t locus, std::string str) :
	locus (locus),
	str (str)
    {
	if (locus != UNKNOWN_LOCATION)
	    this-> locFile = LOCATION_FILE (locus);
    }

    Word (location_t locus, const char* str) :
	locus (locus),
	str (str)
    {
	if (locus != UNKNOWN_LOCATION)
	    this-> locFile = LOCATION_FILE (locus);
    }
    
    Word (const Word & other) :
	locus (other.locus),
	str (other.str)
    {
	this-> locFile = other.locFile;
    }

    const Word & operator=(const Word & other) {
	this-> locus = other.locus;
	this-> str = other.str;
	if (this-> locus != UNKNOWN_LOCATION)
	    this-> locFile = LOCATION_FILE (this-> locus);
	return other;
    }
    
    Word () :
	locus (UNKNOWN_LOCATION),
	str ("")
    {}
    
    void setLocus (location_t locus) {
	if (locus != UNKNOWN_LOCATION)
	    this-> locFile = LOCATION_FILE (locus);
	this-> locus = locus;
    }

    location_t getLocus () const {
	return this-> locus;
    }

    std::string getStr () const {
	return this-> str;
    }

    void setStr (std::string other) {
	this-> str = other;	
    }

    std::string getFile () const;  
    
    static Word eof () {
	return Word (UNKNOWN_LOCATION, "");
    }

    bool isEof () const {
	return this-> locus == UNKNOWN_LOCATION;
    }

    void setEof () {
	this-> locus = UNKNOWN_LOCATION;
	this-> str = "";
    }

    bool isToken () const;
    

    friend bool operator== (const Word& elem, const char* sec) {
	return elem.getStr () == sec;
    }
    
    friend bool operator== (const Word& elem, std::string& sec) {
	return elem.getStr () == sec;
    }

    friend bool operator!= (const Word& elem, const char* sec) {
	return elem.getStr () != sec;
    }
    
    friend bool operator!= (const Word& elem, std::string& sec) {
	return elem.getStr () != sec;
    }
    
    std::string toString () const;    
    
};

