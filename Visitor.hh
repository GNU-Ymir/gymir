#pragma once

#include "Lexer.hh"
#include "Syntax.hh"

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
    VarPtr visit_var (Lexical::Lexer&);
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
    ExpressionPtr visit_pth_wpar (Lexical::Lexer&, Lexical::TokenPtr);
    ExpressionPtr visit_pth_par (Lexical::Lexer&);
    ExpressionPtr visit_inside_if (Lexical::Lexer&);
    ExpressionPtr visit_inside_for (Lexical::Lexer&);
    ExpressionPtr visit_new (Lexical::Lexer&);
    ExpressionPtr visit_cast (Lexical::Lexer&);
    ExpressionPtr visit_constante (Lexical::Lexer&);
    ExpressionPtr visit_numeric (Lexical::Lexer&, Lexical::TokenPtr);
    ExpressionPtr visit_flottant (Lexical::Lexer&);
    ExpressionPtr visit_chaine (Lexical::Lexer&, Lexical::TokenPtr);
    ExpressionPtr visit_bool (Lexical::Lexer&, Lexical::TokenPtr);
    ExpressionPtr visit_null (Lexical::Lexer&, Lexical::TokenPtr);
    ExpressionPtr visit_suite (Lexical::Lexer&, Lexical::TokenPtr, ExpressionPtr);
    ExpressionPtr visit_par (Lexical::Lexer&, ExpressionPtr);
    ExpressionPtr visit_access (Lexical::Lexer&, ExpressionPtr);
    ExpressionPtr visit_dot (Lexical::Lexer&, ExpressionPtr);
    ExpressionPtr visit_left_op (Lexical::Lexer&);
    ExpressionPtr visit_after (Lexical::Lexer&, Lexical::TokenPtr, ExpressionPtr);
    ExpressionPtr visit_before_pth (Lexical::Lexer&, Lexical::TokenPtr);
    InstructionPtr visit_instruction (Lexical::Lexer&, Lexical::TokenPtr);
    InstructionPtr visit_if (Lexical::Lexer&);
    InstructionPtr visit_else (Lexical::Lexer&);
    InstructionPtr visit_return (Lexical::Lexer&);
    InstructionPtr visit_for (Lexical::Lexer&);
    InstructionPtr visit_foreach (Lexical::Lexer&);
    InstructionPtr visit_while (Lexical::Lexer&);
    InstructionPtr visit_break (Lexical::Lexer&);
    InstructionPtr visit_delete (Lexical::Lexer&);
};
