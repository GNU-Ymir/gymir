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

    AstPtr visit_program (Lexical::Lexer&);
    DeclarationPtr visit_def (Lexical::Lexer&);
    DeclarationPtr visit_import (Lexical::Lexer&);
    VarPtr visit_var_declaration (Lexical::Lexer&);
    VarPtr visit_type (Lexical::Lexer&);
    Lexical::TokenPtr visit_identifiant (Lexical::Lexer&);
    BlockPtr visit_block (Lexical::Lexer&);
    ExpressionPtr visit_expression_ult (Lexical::Lexer&);
    ExpressionPtr visit_expression_ult (Lexical::Lexer&, ExpressionPtr);
    ExpressionPtr visit_expression (Lexical::Lexer&);
    ExpressionPtr visit_expression (Lexical::Lexer&, ExpressionPtr);
    ExpressionPtr visit_ulow (Lexical::Lexer&, ExpressionPtr);
    ExpressionPtr visit_ulow (Lexical::Lexer&);
    ExpressionPtr visit_low (Lexical::Lexer&, ExpressionPtr);
    ExpressionPtr visit_low (Lexical::Lexer&);
    ExpressionPtr visit_high (Lexical::Lexer&, ExpressionPtr);
    ExpressionPtr visit_high (Lexical::Lexer&);
    ExpressionPtr visit_pth (Lexical::Lexer&);    
    ExpressionPtr visit_before_pth (Lexical::Lexer&, Lexical::TokenPtr);
};
