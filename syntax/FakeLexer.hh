#pragma once

#include "Word.hh"
#include "Token.hh"
#include "Lexer.hh"
#include <vector>
#include <map>
#include <stdlib.h>

namespace lexical {

    /**
       Cette classe permet d'analyser un fichier et de renvoyer le Token associe a chaque element
       Elle est l'element principale de l'analyse lexical
    */
    struct FakeLexer : Lexer {

	FakeLexer (const std::vector<Word> & words);

	void cutCurrentWord (ulong beg);
	
	FakeLexer cutOff ();
	
	std::string toString ();
	
    protected:

	bool getWord (Word& word) override;
	
    private :

	std::vector <Word> words;
	ulong fake_current;
	
    };

}
