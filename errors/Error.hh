#pragma once

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "input.h"
#include "diagnostic.h"
#include <ymir/lexing/Word.hh>
#include <cmath>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/errors/ListError.hh>
#include <ymir/errors/Exception.hh>

namespace Ymir {
    

    /**
       \brief format a string with simple system
       \brief replace all the % token with the first parameter (recursivly)
     */
    template <typename ... T> 
    std::string format (std::string left, T... params) {
	OutBuffer buf;
	buf.writef (left.c_str (), params...);
	return buf.str ();
    }

    /** 
       \brief format a string with simple system
       \brief replace all the % token with the first parameter (recursivly)
     */
    template <typename ... T> 
    std::string format (const char* left, T... params) {
	OutBuffer buf;
	buf.writef (left, params...);
	return buf.str ();
    }
    
    namespace Error {

	/**
	   \struct ErrorMsg
	   A error message
	*/
	struct ErrorMsg {
	    /// the content of the message 
	    std::string msg;

	    /// This message will bring to a compilation abort 
	    bool isFatal;

	    /// This error is only a warning information (unused)
	    bool isWarning;       	
	};
	
	/** 
	    \brief add the line information on the error
	    \brief Only add underline information if the word string is equals to the line at the correct location
	    \param the chain to which the line information is append
	    \param word the location of the line	
	    \return a new string, with line information
	*/
	std::string addLine (const std::string&, const Word& word);

	/**
	   \brief cause the compiler to abort due to internal error
	   \param the message, can be NULL
	*/
	void halt (const char* format = NULL);

	template <typename ... TArgs>
	void occur (const Word & loc, const std::string &content, TArgs ... args) {
	    auto msg = format ("%(r) : " + content, "Error", args...);
	    msg = addLine (msg, loc);
	    fprintf (stderr, "%s", msg.c_str ());
	    THROW ((int) ErrorCode::EXTERNAL);
	}
	
	/**
	   \brief Cause a throw FAIL_ERROR
	   \param format_ the content of the error
	   \param args the parameters of the format
	*/
	template <typename ... TArgs>
	void fail (const Word & loc, const std::string & content, TArgs ... args) {
	    auto msg = format (std::string ("%(r) : ") + content, "Error", args...);
	    msg = addLine (msg, loc);
	    fprintf (stderr, "%s", msg.c_str ());
	    THROW ((int) ErrorCode::FAIL);
	}

	/**
	   \brief Stop the compilation (fatal_error, no throw)
	   \param format_ the message
	   \param args the parameters to use in format for the message
	*/
	template <typename ... TArgs>
	void end (const char * format_, TArgs ... args) {	    
	    fatal_error (UNKNOWN_LOCATION, format_, args...);
	}       

	/**
	   \brief throw a FATAL_ERROR
	   \param word the location 
	   \param format_ the message
	   \param args the parameters to pass to format	   
	*/
	template <typename ... TArgs>
	void fatal (const Word& word, const std::string & content, TArgs ... args) {
	    std::string aux = format (std::string ("%(r) : ") + content, "Error", args...);
	    aux = addLine (aux, word);
	    fprintf (stderr, "%s", aux.c_str ());
	    THROW ((int) ErrorCode::FATAL);
	}

	/**
	   \brief append a new error, (neither fatal nor fail)
	 */
	template <typename ... TArgs>
	void append (const Word& word, const std::string& format_, TArgs ... args) {
	    std::string aux = format ("%(r) : " + format_, "Error", args...);
	    aux = addLine (aux, word);
	    fprintf (stderr, "%s", aux.c_str ());
	}

	/**
	   \brief Print a note error on the error stream
	   \param word the location of the note
	   \param format_ the content of the note
	   \param args the parameter of the format
	 */
	template <typename ... TArgs>
	void note (const Word& word, const std::string& format_, TArgs ... args) {
	    std::string aux = format ("%(b) : " + format_, "Note", args...);
	    aux = addLine (aux, word);
	    fprintf (stderr, "%s\n", aux.c_str ());	    
	}
	
	/**
	   \brief Cause the compiler to abort due to internal error
	   \param format_ a string to format with parameters
	   \param args the parameters
	*/
	template <typename ... TArgs>
	void halt (const std::string & content, TArgs ... args) {
	    auto msg = format ("%(r) : " + content, "Assert", args...);
	    fprintf (stderr, "%s\n", msg.c_str ());
	    raise (SIGABRT);
	}
	
    }
    
}
