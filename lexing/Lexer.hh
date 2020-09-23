#pragma once


#include "config.h"
#include "system.h"
#include "coretypes.h"

#include "Word.hh"
#include "Token.hh"
#include <vector>
#include <map>
#include <stdlib.h>

namespace lexing {

    /**
     * This class allows you to analyze a file and return the Token associated with each element. It is the main element of lexical analysis 
     * implemented in lexing/Lexer.cc
    */
    struct Lexer {

	/**
	 * \brief Empty lexer, always EOF
	 */
	Lexer ();

	Lexer (const Lexer & lex);

	const Lexer & operator= (const Lexer & lex);
	
	/**
	 * \param filename the name of the file
	 * \param file the file (already opened)
	 * \param the token to skip
	 * \param comments the tokens used to define comments
	 */
	Lexer (const char * filename, FILE * file, const std::vector<std::string> &skips, const std::map <std::string, std::pair <std::string, std::string> > &comments);


	/**
	 * Create a lexer that read from a string instead of a file	
	 * \param line, the init line (string content are generally from macros, and the macro is in a file, thus we need to ) 
	 */
	static Lexer initFromString (const std::string & content, const std::string & filename, const std::vector<std::string> &skips, const std::map <std::string, std::pair <std::string, std::string> > &comments, ulong line);

	
	/**
	 * \return the name of the file
	 */
	std::string getFilename () const;

	/**
	 * \brief enable a skip token
	 * \param elem the token to skip
	 * \param on is this token skipable ?
	 */
	void skipEnable (const std::string& elem, bool on = true);

	/**
	 * \brief enable a list of skip token
	 * \param elem a list of token to skip
	 * \param on is this token skipable ?
	 */
	void skipEnable (const std::vector <std::string> &elem, bool on = true);

	/**
	 * \brief Does the lexer skip the comments ? (does not remove documentation comments)
	 */
	void commentEnable (bool on = true);

	/**
	 * \brief return (by reference) the next word
	 * \return this lexer (to enable sequencing) 
	 */
	Lexer& next (Word &word);

	/**
	 * \brief return (by reference) the documentation, and the next word
	 * \brief the documentation returned is the one associated with the word
	 * \return this lexer (to enable sequencing)
	 */
	Lexer& nextWithDocs (std::string & , Word &word);

	/**
	 * \brief return (by reference) the documentation, and the next word
	 * \brief the documentation returned is the one associated with the word
	 * \return the next word
	 */
	Word nextWithDocs (std::string & docs);

	/**
	 * \return the next word
	 */
	Word next ();

	/**
	 * \brief Verify that the next word is in the list of mandatories token
	 * \brief throw FAIL if not
	 * \return the next word
	 */
	Word next (const std::vector <std::string> &mandatories);

	/**
	 * \brief Verify that the next word is in the list of mandatories token
	 * \brief return the associated documentation (by reference)
	 * \brief throw FAIL if not
	 * \return the next word
	 */
	Word nextWithDocs (std::string & docs, const std::vector <std::string> &mandatories);


	/**
	 * \brief Consume the next token if it is inside the optional token list
	 * \brief Otherwise does nothing
	 * \return the next token, or an empty word position at the current file location
	 */
	Word consumeIf (const std::vector <std::string> & optional);
	
	/**
	 * \brief rewind to previous word
	 * \warning (does not include the skiped words)
	 * \param nb the number of word to rewind
	 */
	virtual Lexer& rewind (ulong nb = 1);

	/**
	 * \return the index of the current word
	 */
	ulong tell () const;

	/**
	 * \brief Move to a specific word index 
	 * \param loc the index of the word that will be the current one
	 */
	void seek (ulong loc);

	/**
	 * \return an empty word with file information (filename)
	 */
	Word fileLocus ();

	/**
	 * \brief close the file
	 */
	void dispose ();

	const std::string & getContent () const;

	const std::string & getStringName () const;
	
	~Lexer ();

    protected:

	/**
	 * \brief get the current word by reference
	 * \return this
	 */
	Lexer& get (Word &word);

	/**
	 * \brief get the current word by reference
	 * \brief and the associated doc
	 * \return this
	 */
	Lexer& getWithDocs (std::string & docs, Word &word);

	/**
	 * \brief Tell if the word is a comment 
	 * \brief if it is, return the closing token
	 * \param retour the closing token
	 * \param ign the token ignore at each line in this comments for exemple (*, for comment multiline comment)
	 */
	bool isComment (const Word& elem, std::string &retour, std::string & ign);

	/**
	 * \return Do we need to skip this word ?
	 */
	bool isSkip (const Word &word);

	/**
	 * \brief return the current word (read in the file) by reference
	 * \return true if a ord has been read, false otherwise
	 */
	virtual bool getWord (Word &word);

	/**
	 * \return the minimal value between a and b
	 */
	ulong min (ulong a, ulong b);

	/**
	 * \brief construct a new word from line informations
	 */
	void constructWord (Word &word, ulong beg, ulong _max, const std::string &line,
			    ulong where);
	
    protected:

	static const int maxColumnHint = 80;
	ulong line, column;    
	bool enableComment, disposed;
	std::string filename;

	
	std::map <std::string, bool> skips;
	std::vector <std::string> tokens;	
	std::map <std::string, std::pair<std::string, std::string> > comments;
	std::vector <Word> reads;
	std::vector <std::string> docs;
	
	long current;
	FILE * file;

	const struct line_map * line_map;

	std::string content = "";
	bool isFromString = false;
	std::string string_name;
	ulong start = 0;
	
    };    
    
};
