#pragma once

#include "_.hh"
#include "ast/_.hh"

namespace syntax {

    struct Visitor {

	Visitor (lexical::Lexer & lexer) : lexer (lexer) {}

	Program visit ();
	
    private:


	Program visit_program ();
	std::vector<Declaration> visit_public_block ();
	std::vector<Declaration> visit_private_block ();

	Declaration visit_declaration (bool);
	
	lexical::Lexer & lexer;

    };

};
