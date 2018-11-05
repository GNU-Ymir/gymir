#pragma once


#include "config.h"
#include "system.h"
#include "coretypes.h"

#include "Word.hh"
#include "Token.hh"
#include <vector>
#include <map>
#include <stdlib.h>

namespace lexical {

    /**
       Cette classe permet d'analyser un fichier et de renvoyer le Token associe a chaque element
       Elle est l'element principale de l'analyse lexical
    */
    struct Lexer {

	Lexer (const char * filename, FILE * file, std::vector<std::string> skips, std::map <std::string, std::pair <std::string, std::string> > comments);
	
	std::string getFilename () const;

	void skipEnable (std::string elem, bool on = true);

	void skipEnable (std::vector <std::string> elem, bool on = true);
	
	void commentEnable (bool on = true);

	Lexer& next (Word &word);

	Lexer& nextWithDocs (std::string & , Word &word);

	Word nextWithDocs (std::string & docs);
	
	Word next ();

	Word next (std::vector <std::string> mandatories);

	Word nextWithDocs (std::string & docs, std::vector <std::string> mandatories);
	
	virtual Lexer& rewind (ulong nb = 1);

	ulong tell ();

	void seek (ulong);
       	
	Word fileLocus ();

	void dispose ();
	
	~Lexer ();

    protected:

	Lexer& get (Word &word);

	Lexer& getWithDocs (std::string & docs, Word &word);

	bool isComment (Word elem, std::string &retour, std::string & ign);

	bool isSkip (Word elem);

	virtual bool getWord (Word &word);

	ulong min (ulong, ulong);

	void constructWord (Word &word, ulong beg, ulong _max, std::string line,
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
    
    };
  
};
