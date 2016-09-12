#pragma once

#include "Lexer.hh"
#include "Declaration.hh"

namespace Lexical {

    struct Visitor {

	Visitor (Lexer & lexer) : lexer (lexer) {}

	Syntax::Ast visit ();
	
    private:
	
	Lexer & lexer;

    };

    Syntax::Ast visit_program (Lexer & lexer);
    Syntax::Declaration visit_def (Lexer & lexer);
    Syntax::Declaration visit_import (Lexer & lexer);
    
};
