#pragma once

#include "Lexer.hh"
#include "Declaration.hh"

namespace Syntax {

    struct Visitor {

	Visitor (Lexical::Lexer & lexer) : lexer (lexer) {}

	Syntax::AstPtr visit ();
	
    private:
	
	Lexical::Lexer & lexer;

    };

    Syntax::AstPtr visit_program (Lexical::Lexer & lexer);
    Syntax::DeclarationPtr visit_def (Lexical::Lexer & lexer);
    Syntax::DeclarationPtr visit_import (Lexical::Lexer & lexer);
    Syntax::VarPtr visit_var_declaration (Lexical::Lexer & lexer);
    Syntax::VarPtr visit_type (Lexical::Lexer & lexer);
    Lexical::TokenPtr visit_identifiant (Lexical::Lexer & lexer);
    Syntax::BlockPtr visit_block (Lexical::Lexer & lexer);
    
};
