#pragma once

#include "syntax/_.hh"
#include <stdio.h>

/**
   Fonction appele par gcc
*/
extern void ymir_parse_files (int num_files, const char ** files);

namespace Ymir {
    
    struct Parser {

	Parser (const char *, FILE *);
	
	void parse_program ();
	
    private:

	syntax::Program syntax_analyse ();

	void semantic_analyse (syntax::Program &);

	void define_gcc_symbols ();
	
    private:
	
	lexical::Lexer lexer;
	
    };
    
};
