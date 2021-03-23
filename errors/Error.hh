#pragma once

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "input.h"
#include "diagnostic.h"
#include <cmath>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/errors/ListError.hh>
#include <list>
#include <ymir/errors/Exception.hh>
#include <ymir/lexing/Word.hh>

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

	struct ErrorMsg {
	private :
	    
	    lexing::Word begin;
	    
	    lexing::Word end;
	    
	    std::string msg;

	    bool one_line;

	    std::list <ErrorMsg> notes;
	    
	public :

	    ErrorMsg (const lexing::Word & begin, const std::string & msg);

	    ErrorMsg (const lexing::Word & begin, const lexing::Word & end, const std::string & msg);

	    ErrorMsg (const std::string & msg);
	    
	    void addNote (const ErrorMsg & note);
	    
	    void computeMessage (Ymir::OutBuffer & buf, unsigned long depth, unsigned long max_depth, bool writtenSub = false) const;

	    unsigned long computeMaxDepth () const;
	    
	    bool isEmpty () const;

	    const lexing::Word & getLocation () const;

	    const std::string & getMessage () const;

	    std::list <ErrorMsg> getNotes () const;
	    
	};

	
	struct ErrorList {

	    std::list <ErrorMsg> errors;      

	    void print () const;
    
	};

	struct FatalError {
	    std::string msg;
	    void print () const;
	};
	
	/** 
	    \brief add the line information on the error
	    \brief Only add underline information if the word string is equals to the line at the correct location
	    \param the chain to which the line information is append
	    \param word the location of the line	
	    \return a new string, with line information
	*/
	void addLine (Ymir::OutBuffer & buf, const std::string&, const lexing::Word& word);

	

	/** 
	    \brief add the line information on the error
	    \brief Only add underline information if the word string is equals to the line at the correct location
	    \param the chain to which the line information is append
	    \param word the location of the line	
	    \return a new string, with line information
	*/
	void addLine (Ymir::OutBuffer & buf, const std::string&, const lexing::Word& word, const lexing::Word & end);

	/**
	 * \brief Add a note to an error
	 */
	void addNote (const lexing::Word&, const std::string &, const std::string&);
	
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
	    throw ErrorList {std::list <ErrorMsg> {ErrorMsg (loc, msg)}};	    
	}

	template <typename ... TArgs>
	void occur (const lexing::Word & loc, const lexing::Word & end, const std::string &content, TArgs ... args) {
	    auto msg = format ("%(r) : " + content, "Error", args...);
	    throw ErrorList {std::list <ErrorMsg> {ErrorMsg (loc, end, msg)}};
	}


	template <typename ... TArgs>
	ErrorMsg makeOccur (const lexing::Word & loc, const lexing::Word & end, const std::string &content, TArgs ... args) {
	    auto msg = format ("%(r) : " + content, "Error", args...);
	    return ErrorMsg (loc, end, msg);
	}

	template <typename ... TArgs>
	ErrorMsg makeOccurAndNote (const lexing::Word & loc, const ErrorMsg & note, const std::string &content, TArgs ... args) {
	    auto msg = format ("%(r) : " + content, "Error", args...);
	    auto err = ErrorMsg (loc, msg);
	    err.addNote (note);
	    return err;
	}
	
	template <typename ... TArgs>
	ErrorMsg makeOccurAndNote (const lexing::Word & loc, const std::list <ErrorMsg> & notes, const std::string &content, TArgs ... args) {
	    auto msg = format ("%(r) : " + content, "Error", args...);
	    auto err = ErrorMsg (loc, msg);
	    for (auto & it : notes)
		err.addNote (it);
	    return err;
	}


	
	template <typename ... TArgs>
	ErrorMsg makeOccur (const lexing::Word & loc, const std::string &content, TArgs ... args) {
	    auto msg = format ("%(r) : " + content, "Error", args...);	    
	    return ErrorMsg (loc, msg);
	}
	
	
	template <typename ... TArgs>
	ErrorMsg makeWarn (const lexing::Word & loc, const std::string & content, TArgs ... args) {
	    auto msg = format ("%(y) : " + content, "Warning", args...);	    
	    return ErrorMsg (loc, msg);
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
	    throw ErrorList {std::list <ErrorMsg> {ErrorMsg (loc, msg)}};
	}

	template <typename ... TArgs>
	void occurAndNote (const lexing::Word & loc, const ErrorMsg & note, const std::string &content, TArgs ... args) {
	    auto msg = format ("%(r) : " + content, "Error", args...);
	    auto err = ErrorMsg (loc, msg);
	    err.addNote (note);	    
	    throw ErrorList {std::list <ErrorMsg> {err}};
	}
	

	template <typename ... TArgs>
	void occurAndNote (const lexing::Word & loc, const std::list <ErrorMsg> & notes, const std::string &content, TArgs ... args) {
	    auto msg = format ("%(r) : " + content, "Error", args...);
	    auto err = ErrorMsg (loc, msg);
	    for (auto & it : notes)
		err.addNote (it);	    
	    throw ErrorList {std::list <ErrorMsg> {err}};
	}
       
	
	template <typename ... TArgs>
	void occurAndNote (const lexing::Word & loc, const lexing::Word & loc2, const ErrorMsg & note, const std::string &content, TArgs ... args) {
	    auto msg = format ("%(r) : " + content, "Error", args...);
	    auto err = ErrorMsg (loc, loc2, msg);
	    err.addNote (note);
	    throw ErrorList {std::list <ErrorMsg> {err}};
	}

	template <typename ... TArgs>
	void occurAndNote (const lexing::Word & loc, const lexing::Word & loc2, const std::list <ErrorMsg> & notes, const std::string &content, TArgs ... args) {
	    auto msg = format ("%(r) : " + content, "Error", args...);
	    auto err = ErrorMsg (loc, loc2, msg);
	    for (auto & it : notes)
		err.addNote (it);
	    throw ErrorList {std::list <ErrorMsg> {err}};
	}

	
	template <typename ... TArgs>
	void occur (const std::string &content, TArgs ... args) {
	    auto msg = format ("%(r) : " + content, "Error", args...);
	    throw ErrorList {std::list <ErrorMsg> {ErrorMsg (lexing::Word::eof (), msg)}};
	}

	template <typename ... TArgs>
	void warn (const std::string & content, TArgs ... args) {
	    auto msg = format ("%(y) : " + content, "Warning", args...);
	    throw ErrorList {std::list <ErrorMsg> {ErrorMsg (lexing::Word::eof (), msg)}};
	}
	
	template <typename ... TArgs>
	void noteAndNote (const lexing::Word & loc, const std::list <ErrorMsg> & notes, const std::string &content, TArgs ... args) {
	    auto msg = format ("%(b) : " + content, "Note", args...);
	    auto err = ErrorMsg (loc, msg);
	    for (auto & it : notes)
		err.addNote (it);	    
	    throw ErrorList {std::list <ErrorMsg> {err}};
	}
	
	/**
	   \brief Cause a throw FAIL_ERROR
	   \param format_ the content of the error
	   \param args the parameters of the format
	*/
	template <typename ... TArgs>
	void fail (const lexing::Word & loc, const std::string & content, TArgs ... args) {
	    auto msg = format (std::string ("%(r) : ") + content, "Error", args...);
	    auto err = ErrorMsg (loc, msg);

	    //bt_print ();
	    throw ErrorList {std::list <ErrorMsg> (err)};
	}

	/**
	   \brief Stop the compilation (fatal_error, no throw)
	   \param format_ the message
	   \param args the parameters to use in format for the message
	*/
	template <typename ... TArgs>
	void end (const std::string& format_, TArgs ... args) {	    
	    fatal_error (UNKNOWN_LOCATION, "%s", format (format_, args...).c_str ());
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
	    Ymir::OutBuffer buf;
	    addLine (buf, aux, word);

	    //bt_print ();

	    throw FatalError {buf.str ()};
	}

	/**
	   \brief Print a note error on the error stream
	   \param word the location of the note
	   \param format_ the content of the note
	   \param args the parameter of the format
	 */
	template <typename ... TArgs>
	ErrorMsg createNote (const lexing::Word& word, const std::string& format_, TArgs ... args) {
	    std::string aux = format ("%(b) : " + format_, "Note", args...);
	    return ErrorMsg (word, aux);
	}
	
	/**
	   \brief Print a note error on the error stream
	   \param word the location of the note
	   \param format_ the content of the note
	   \param args the parameter of the format
	 */
	template <typename ... TArgs>
	ErrorMsg createNoteOneLine (const std::string& format_, TArgs ... args) {
	    std::string aux = format ("%(b) : " + format_, "Note", args...);
	    return ErrorMsg (aux);
	}

	/**
	   \brief Print a note error on the error stream
	   \param word the location of the note
	   \param format_ the content of the note
	   \param args the parameter of the format
	 */
	template <typename ... TArgs>
	ErrorMsg createNoteOneLineAndNote (const std::list <ErrorMsg> & notes, const std::string& format_, TArgs ... args) {
	    std::string aux = format ("%(b) : " + format_, "Note", args...);
	    auto ret = ErrorMsg (aux);
	    for (auto & it : notes) {
		ret.addNote (it);
	    }
	    return ret;
	}
	
	/**
	   \brief Create a note message
	   \param word the location of the note
	 */
	ErrorMsg createNote (const lexing::Word& word);
	
	
    }
    
}
