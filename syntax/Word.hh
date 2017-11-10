#pragma once

#include "config.h"
#include "system.h"
#include "coretypes.h"

#include <string>
#include <tr1/memory>
#include <stdlib.h>
#include <gc/gc_cpp.h>
#include <sstream>

struct Word {
private:
    
    location_t locus;
    std::string str;

public:

    Word (location_t locus, std::string str) :
	locus (locus),
	str (str)
    {}

    Word (location_t locus, const char* str) :
	locus (locus),
	str (str)
    {}
    
    Word (const Word & other) :
	locus (other.locus),
	str (other.str)
    {}

    Word () :
	locus (UNKNOWN_LOCATION),
	str ("")
    {}
    
    void setLocus (location_t locus) {
	this-> locus = locus;
    }

    location_t getLocus () const {
	return this-> locus;
    }

    std::string getStr () {
	return this-> str;
    }

    void setStr (std::string other) {
	this-> str = other;	
    }

    static Word eof () {
	return Word (UNKNOWN_LOCATION, "");
    }

    bool isEof () {
	return this-> locus == UNKNOWN_LOCATION;
    }

    void setEof () {
	this-> locus = UNKNOWN_LOCATION;
	this-> str = "";
    }

    friend bool operator== (Word elem, std::string sec) {
	return elem.getStr () == sec;
    }

    friend bool operator!= (Word elem, std::string sec) {
	return elem.getStr () != sec;
    }
    
    std::string toString () {
	if (this-> isEof ()) return ":\u001B[32meof\u001B[0m()";
	auto reset = "\u001B[0m";
	auto purple = "\u001B[36m";
	auto green = "\u001B[32m";
	std::stringstream out;
	out << ":" << green << this-> str << reset << "(" 
	    << LOCATION_FILE (this-> locus) << " -> "
	    << purple<< LOCATION_LINE (this-> locus)
	    << reset << ", "
	    << purple << LOCATION_COLUMN (this-> locus) << reset << ")";
	return out.str ();
    }
    
};

