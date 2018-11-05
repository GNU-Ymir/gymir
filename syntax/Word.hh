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
    
    //    location_t locus;    
    std::string str;
    std::string locFile;
    ulong line;
    ulong column;
    long _length = -1;
    
public:

    Word (location_t locus, std::string str);

    Word (const Word & other, std::string str);

    Word (const Word & other, std::string str, long len);
    
    Word (location_t locus, const char* str);
    
    Word (const Word & other);
    
    Word ();
    
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
    
    std::string toString () const;    
    
};

