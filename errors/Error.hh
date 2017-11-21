#pragma once

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "input.h"
#include "diagnostic.h"
#include "../syntax/Word.hh"
#include <cmath>

#define INCLUDE_STRING

namespace Ymir {
    	    
    enum ErrorType {
	NotATemplate,
	TakeAType,
	SyntaxError,
       	SyntaxError2,
	EscapeChar,
	EndOfFile,
	Unterminated,
	TemplateSpecialisation,
	TemplateCreation,
	And
    };

    enum Language {
	FR,
	EN
    };
    
    std::string addLine (std::string, Word word);
    
    const char * getString (ErrorType, Language ln = EN);
    
    template <typename ... T> 
    std::string format (std::string left, T... params) {
	auto len = snprintf (NULL, 0, left.c_str (), params...);
	auto aux = new char [len + 1];
	sprintf (aux, left.c_str (), params...);
	aux [len] = '\0';
	std::string ret = std::string (aux);
	free (aux);
	return ret;
    }

    struct Error {

	static const char*	RESET;	//= "\u001B[0m";
	static const char*	PURPLE;	//= "\u001B[1;35m";
	static const char*	BLUE;	//= "\u001B[1;36m";
	static const char*	YELLOW;	//= "\u001B[1;33m";
	static const char*	RED;	//= "\u001B[1;31m";
	static const char*	GREEN;	// = "\u001B[1;32m";
	static const char*	BOLD;	// = "\u001B[1;50m";
	
	static unsigned long nb_errors;// (0);
       	
	template <typename ... TArgs>
	void fatal_ (Word word, const char * format_, TArgs ... args) {
	    std::string aux = format (format_, args...);
	    aux = std::string (RED) + "Error" + std::string (RESET) + " : " + aux;
	    aux = addLine (aux, word);
	    printf ("%s", aux.c_str ());
	    
	    fatal_error (UNKNOWN_LOCATION, "");
	}
		
	template <typename ... TArgs>
	static void fatal (Word word, const char * format_, TArgs ... args) {
	    __instance__.fatal_ (word, format_, args...);
	}

	template <typename ... TArgs>
	static void fatal (Word word, ErrorType type, TArgs ... args) {
	    __instance__.fatal_ (word, getString (type), args...);
	}	
	
	template <typename ... TArgs>
	void append_ (Word word, const char * format_, TArgs ... args) {
	    std::string aux = format (format_, args...);
	    aux = std::string (RED) + "Error" + std::string (RESET) + " : " + aux;
	    aux = addLine (aux, word);
	    printf ("%s", aux.c_str ());
	    nb_errors ++;
	}

	template <typename ... TArgs>
	static void append (Word word, const char * format_, TArgs ... args) {
	    __instance__.append_ (word, format_, args...);
	}

	template <typename ... TArgs>
	static void append (Word word, ErrorType type, TArgs ... args) {
	    __instance__.append_ (word, getString (type), args...);
	}
	
	template <typename ... TArgs>
	void note_ (Word word, const char * format_, TArgs ... args) {
	    std::string aux = format (format_, args...);
	    aux = std::string (BLUE) + "Note" + std::string (RESET) + " : " + aux;
	    aux = addLine (aux, word);
	    printf ("%s", aux.c_str ());
	    
	    inform (UNKNOWN_LOCATION, "");
	}

	template <typename ... TArgs>
	static void note (Word word, ErrorType type, TArgs ... args) {
	    __instance__.note_ (word, getString (type), args...);
	}

	template <typename ... TArgs>
	static void note (Word word, const char * format_, TArgs ... args) {
	    __instance__.note_ (word, format_, args...);
	}
	
	template <typename ... TArgs>
	void assert_ (const char * format_, TArgs ... args) {
	    fatal_error (UNKNOWN_LOCATION, format_, args...);
	}

	template <typename ... TArgs>
	static void assert (const char * format_, TArgs ... args) {
	    return __instance__.assert_ (format_, args...);
	}
	
	static bool thrown () {
	    return nb_errors != 0;
	}

	static Error instance () {
	    return __instance__;
	}	
	
    private:

	static Error __instance__;
	
    };
    
}
