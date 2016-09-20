#pragma once

#include "Lexer.hh"
#include "Token.hh"
#include "Syntax.hh"
#include "ast.hh"

/**
   Fonction appele par gcc
*/
extern void ymir_parse_files (int num_files, const char ** files);

namespace Ymir {
    
    struct Parser {

	Parser (Lexical::Lexer & file) : lexer (file) {}
	
	void parse_program ();
	
    private:

	Syntax::AstPtr syntax_analyse ();

	Syntax::AstPtr semantic_analyse (Syntax::AstPtr &);

	void define_gcc_symbols ();
	
    private:
	
	Lexical::Lexer & lexer;
	
    };
    
};
