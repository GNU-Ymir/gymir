#pragma once

#include "config.h"
#include "system.h"
#include "coretypes.h"

#include "YTypes.hh"
#include "Token.hh"
#include <vector>
#include <map>

namespace Lexical {

    /**
       Cette classe permet d'analyser un fichier et de renvoyer le Token associe a chaque element
       Elle est l'element principale de l'analyse lexical
    */
    struct Lexer {

	Lexer (const char * filename, FILE * input);

	TokenPtr next ();

	void rewind (unsigned long nb = 1);

	void setKeys (std::vector<std::string> keys);
	
	void setSkip (std::vector<std::string> skip);

	void removeSkip (std::string elem);

	void addSkip (std::string elem);
	
	void setComments (std::vector <std::pair<std::string, std::string> > comments);

	void setComments (bool on);
    
	~Lexer ();

    private:

	TokenPtr get ();
    
	location_t getCurrentLocation ();

	TokenId isComment (TokenPtr);

	bool isSkip (TokenPtr);
      
	std::string readln (FILE *);

	void seek (const std::string & info, long pos);

	long tellg ();

	bool eof ();
	
    private:

	static const int maxColumnHint = 80;
	int currentLine, currentColumn;    
	bool commentOn = true;
	std::vector <std::string> skip;
	std::vector <std::string> keys;
	std::vector <std::pair <std::string, std::string> > comments;
	std::vector <TokenPtr> read;
	long currentWord;
	FILE * input;

	const struct line_map * line_map;
    
    };
  
};
