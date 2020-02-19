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
    
    /**
     * \brief print the backtrace into stderr
     */
    void bt_print ();

    namespace Error {
	
	/** 
	    \brief add the line information on the error
	    \brief Only add underline information if the word string is equals to the line at the correct location
	    \param the chain to which the line information is append
	    \param word the location of the line	
	    \return a new string, with line information
	*/
	std::string addLine (const std::string&, const lexing::Word& word);

	/** 
	    \brief add the line information on the error
	    \brief Only add underline information if the word string is equals to the line at the correct location
	    \param the chain to which the line information is append
	    \param word the location of the line	
	    \return a new string, with line information
	*/
	std::string addLine (const std::string&, const lexing::Word& word, const lexing::Word & end);

	/**
	 * \brief Add a note to an error
	 */
	std::string addNote (const lexing::Word&, const std::string &, const std::string&);
	
	/**
	   \brief cause the compiler to abort due to internal error
	   \param the message, can be NULL
	*/
	void halt (const char* format = NULL);

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

	/**
	   \brief Cause the compiler to abort due to internal error
	   \param format_ a string to format with parameters
	   \param args the parameters
	*/
	template <typename ... TArgs>
	void halt (const lexing::Word & loc, const std::string & content, TArgs ... args) {
	    auto msg = format ("%(r) : " + content, "Assert", args...);
	    msg = addLine (msg, loc);
	    fprintf (stderr, "%s\n", msg.c_str ());	    
	    raise (SIGABRT);
	}

	
	template <typename ... TArgs>
	void occur (const lexing::Word & loc, const std::string &content, TArgs ... args) {
	    auto msg = format ("%(r) : " + content, "Error", args...);
	    msg = addLine (msg, loc);
	    THROW ((int) ErrorCode::EXTERNAL, msg);	    
	}

	template <typename ... TArgs>
	void occur (const lexing::Word & loc, const lexing::Word & end, const std::string &content, TArgs ... args) {
	    auto msg = format ("%(r) : " + content, "Error", args...);
	    msg = addLine (msg, loc, end);
	    THROW ((int) ErrorCode::EXTERNAL, msg);
	}


	template <typename ... TArgs>
	std::string makeOccur (const lexing::Word & loc, const lexing::Word & end, const std::string &content, TArgs ... args) {
	    auto msg = format ("%(r) : " + content, "Error", args...);
	    msg = addLine (msg, loc, end);
	    return msg;
	}

	template <typename ... TArgs>
	std::string makeOccurAndNote (const lexing::Word & loc, const std::string & note, const std::string &content, TArgs ... args) {
	    auto msg = format ("%(r) : " + content, "Error", args...);
	    msg = addLine (msg, loc);
	    msg = addNote (loc, msg, note);
	    return msg;
	}

	
	template <typename ... TArgs>
	std::string makeOccur (const lexing::Word & loc, const std::string &content, TArgs ... args) {
	    auto msg = format ("%(r) : " + content, "Error", args...);
	    msg = addLine (msg, loc);
	    return msg;
	}
	
	
	template <typename ... TArgs>
	std::string makeWarn (const lexing::Word & loc, const std::string & content, TArgs ... args) {
	    auto msg = format ("%(y) : " + content, "Warning", args...);
	    msg = addLine (msg, loc);
	    return msg;
	}

	/**
	 * \brief Throw a warning error
	 * \brief A program needs to compile with no warning and no error at all
	 * \brief Warning means the compilation could have continue, but we don't wan't to, to force the user to have a well formed program
	 * \param loc the location of the warning
	 * \param content the content text of the warning
	 * \param args the format arguments
	 * \throw ErrorCode::EXTERNAL
	 */
	template <typename ... TArgs>
	void warn (const lexing::Word & loc, const std::string & content, TArgs ... args) {
	    auto msg = format ("%(y) : " + content, "Warning", args...);
	    msg = addLine (msg, loc);
	    THROW ((int) ErrorCode::EXTERNAL, msg);
	}

	template <typename ... TArgs>
	void occurAndNote (const lexing::Word & loc, const std::string & note, const std::string &content, TArgs ... args) {
	    auto msg = format ("%(r) : " + content, "Error", args...);
	    msg = addLine (msg, loc);
	    msg = addNote (loc, msg, note); 	   
	    THROW ((int) ErrorCode::EXTERNAL, msg);
	}
	
	template <typename ... TArgs>
	void occurAndNote (const lexing::Word & loc, const lexing::Word & loc2, const std::string & note, const std::string &content, TArgs ... args) {
	    auto msg = format ("%(r) : " + content, "Error", args...);
	    msg = addLine (msg, loc, loc2);
	    msg = addNote (loc, msg, note); 	   
	    THROW ((int) ErrorCode::EXTERNAL, msg);
	}

	
	template <typename ... TArgs>
	void occur (const std::string &content, TArgs ... args) {
	    auto msg = format ("%(r) : " + content, "Error", args...);	   
	    THROW ((int) ErrorCode::EXTERNAL, msg);
	}

	template <typename ... TArgs>
	void warn (const std::string & content, TArgs ... args) {
	    auto msg = format ("%(y) : " + content, "Warning", args...);
	    THROW ((int) ErrorCode::EXTERNAL, msg);
	}
	
	/**
	   \brief Cause a throw FAIL_ERROR
	   \param format_ the content of the error
	   \param args the parameters of the format
	*/
	template <typename ... TArgs>
	void fail (const lexing::Word & loc, const std::string & content, TArgs ... args) {
	    auto msg = format (std::string ("%(r) : ") + content, "Error", args...);
	    msg = addLine (msg, loc);

	    //bt_print ();

	    THROW ((int) ErrorCode::FAIL, msg);
	}

	/**
	   \brief Stop the compilation (fatal_error, no throw)
	   \param format_ the message
	   \param args the parameters to use in format for the message
	*/
	template <typename ... TArgs>
	void end (const std::string& format_, TArgs ... args) {	    
	    fatal_error (UNKNOWN_LOCATION, format (format_, args...).c_str ());
	}       

	/**
	   \brief throw a FATAL_ERROR
	   \param word the location 
	   \param format_ the message
	   \param args the parameters to pass to format	   
	*/
	template <typename ... TArgs>
	void fatal (const lexing::Word& word, const std::string & content, TArgs ... args) {
	    std::string aux = format (std::string ("%(r) : ") + content, "Error", args...);
	    aux = addLine (aux, word);

	    //bt_print ();

	    THROW ((int) ErrorCode::FATAL, aux);
	}

	/**
	   \brief Print a note error on the error stream
	   \param word the location of the note
	   \param format_ the content of the note
	   \param args the parameter of the format
	 */
	template <typename ... TArgs>
	void note (const lexing::Word& word, const std::string& format_, TArgs ... args) {
	    std::string aux = format ("%(b) : " + format_, "Note", args...);
	    aux = addLine (aux, word);
	    std::string & msg = getLastError ();
	    msg += aux;
	}

	/**
	   \brief Print a note error on the error stream
	   \param word the location of the note
	   \param format_ the content of the note
	   \param args the parameter of the format
	 */
	template <typename ... TArgs>
	std::string createNote (const lexing::Word& word, const std::string& format_, TArgs ... args) {
	    std::string aux = format ("%(b) : " + format_, "Note", args...);
	    aux = addLine (aux, word);
	    return aux;
	}
	
	/**
	   \brief Print a note error on the error stream
	   \param word the location of the note
	   \param format_ the content of the note
	   \param args the parameter of the format
	 */
	template <typename ... TArgs>
	std::string createNoteOneLine (const std::string& format_, TArgs ... args) {
	    std::string aux = format ("%(b) : " + format_, "Note", args...);
	    return aux;
	}
	
	/**
	   \brief Create a note message
	   \param word the location of the note
	 */
	std::string createNote (const lexing::Word& word);
	
	
    }
    
}
