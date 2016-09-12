#include "Visitor.hh"
#include "config.h"
#include "coretypes.h"
#include "input.h"
#include "diagnostic.h"
#include <map>
#include <vector>


namespace Syntax {

    using namespace Lexical;

    Syntax::AstPtr Visitor::visit () {
	return visit_program (lexer);
    }

    Syntax::AstPtr visit_program (Lexer & lexer) {	
	TokenPtr token = lexer.next ();
	std::vector<Syntax::DeclarationPtr> decls;
	while (token->getId() != Token::EOF_TOKEN()) {
	    if (token->getId() == DEF) decls.push_back(visit_def (lexer));
	    else if (token->getId() == IMPORT) decls.push_back (visit_import (lexer));
	    else error_at (token->getLocus (),
			   "[%s, %s] attendues, mais %s trouvé\n",
			   getTokenDescription(DEF),
			   getTokenDescription(IMPORT),
			   getTokenDescription(token->getId ()));
	    
	    token = lexer.next ();
	}
	
    }

    Syntax::DeclarationPtr visit_import (Lexer & lexer) {
	
    }

    Syntax::DeclarationPtr visit_def (Lexer & lexer) {
	TokenPtr name = visit_identifiant (lexer);
	std::vector <Syntax::VarPtr> exps;
	TokenPtr word = lexer.next ();
	if (word->getId() != PAR_G) error_at (word->getLocus (),
					 "%s attendue, mais %s trouvé\n",
					 getTokenDescription(PAR_G),
					 getTokenDescription(word->getId ()));
	word = lexer.next ();
	if (word->getId() == PAR_D) {
	    lexer.rewind ();
	    while (1) {
		exps.push_back (visit_var_declaration (lexer));
		word = lexer.next ();
		if (word->getId() == PAR_D) break;
		else if (word->getId () == VIRGULE)
		    error_at (word->getLocus (),
			      "[%s, %s] attendue, mais %s trouvé\n",
			      getTokenDescription(PAR_D),
			      getTokenDescription(VIRGULE),
			      getTokenDescription(word->getId ()));
	    }
	}
	
	word = lexer.next ();
	if (word->getId() == DEUX_POINT) {
	    Syntax::VarPtr type = visit_type (lexer);
	    return (new Syntax::Function (name, type, exps, visit_block (lexer)));
	} else lexer.rewind ();
	return (new Syntax::Function (name, exps, visit_block (lexer)));
    }
       
    Syntax::VarPtr visit_var_declaration (Lexer & lexer) {
	TokenPtr token = visit_identifiant (lexer);
	TokenPtr next = lexer.next ();
	if (next->getId () == DEUX_POINT) {
	    auto type = visit_type ();
	    return (new TypedVar (token, type));
	} else lexer.rewind ();
	return (new Var (token));
    }

    Syntax::VarPtr visit_complex_var (Lexer & lexer) {
	TokenPtr token = visit_identifiant (lexer);
	TokenPtr next = lexer.next ();
	if (next->getId () == DEUX_POINT) {
	    auto type = visit_type ();
	    return (new TypedVar (token, type));
	} else error_at (next->getLocus (),
			 "%s attendus, mais %s trouvé",
			 getTokenDescription (DEUX_POINT),
			 getTokenDescription (next->getId ()));
    }
    
    Syntax::VarPtr visit_type (Lexer & lexer) {
	TokenPtr token = visit_identifiant (lexer);
	TokenPtr next = lexer.next ();
	if (next->getId () == NOT) {
	    std::vector<ExpressionPtr> params;
	    next = lexer.next ();
	    if (next->getId () == PAR_G) {
		while (1) {
		    params.push_back (visit_expression (lexer));
		    next = lexer.next ();
		    if (next->getId () == PAR_D) break;
		    else if (next->getId () != VIRGULE)
			error_at (next->getLocus (),
				  "[%s, %s] attendues, mais %s trouvé",
				  getTokenDescription(VIRGULE),
				  getTokenDescription (PAR_D),
				  getTokenDescription (next->getId ()));
		}
	    } else {
		lexer.rewind;
		params.push_back (visit_expression (lexer));
	    }
	    return new Var (token, params);
	} else lexer.rewind ();
	return (new Var (token));
    }

    TokenPtr visit_identifiant (Lexer & lexer) {
	TokenPtr token = lexer.next ();
	if (token->getId() == OTHER) {
	    return token;
	} else error_at (token->getLocus (), "Un identifiant est attendue, mais %s trouvé", getTokenDescription(token->getId()));
    }
    
    Syntax::BlockPtr visit_block (Lexer & lexer) {
    }

    bool find (TokenId id, std::vector <TokenId> tab) {
	for(auto & it : tab) {
	    if (it == id) return true;
	}
	return false;
    }

    Syntax::ExpressionPtr visit_expression_ult (Lexer & lexer) {
	ExpressionPtr left = visit_expression (lexer);
	TokenPtr token = lexer.next ();
	if (find (token->getId (),
		  {AFFECT, PLUS_AFF, MUL_AFF, SUB_AFF, DIV_AFF})) {
	    ExpressionPtr right = visit_expression (lexer);
	    return visit_expression_ult (lexer, new Binary (token->getId (), left, right));
	} else lexer.rewind();
	return left;
    }
    
    ExpressionPtr visit_expression_ult (Lexer & lexer, ExpressionPtr left) {
	TokenPtr token = lexer.next ();       
	if (find (token->getId () ,
		  {AFFECT, PLUS_AFF, MUL_AFF, SUB_AFF, DIV_AFF})) {
	    ExpressionPtr right = visit_expression (lexer);
	    return visit_expression_ult (lexer, new Binary (token->getId (), left, right));
	} else lexer.rewind();
	return left;
    }

    
    Syntax::ExpressionPtr visit_expression (Lexer & lexer) {
	ExpressionPtr left = visit_ulow (lexer);
	TokenPtr token = lexer.next ();
	if (find (token->getId (), {OR, AND})) {
	    ExpressionPtr right = visit_ulow (lexer);
	    return visit_expression (lexer, new Binary (token->getId (), left, right));
	} else lexer.rewind();
	return left;
    }
    
    ExpressionPtr visit_expression (Lexer & lexer, ExpressionPtr left) {
	TokenPtr token = lexer.next ();       
	if (find (token->getId () , {OR, AND})) {
	    ExpressionPtr right = visit_ulow (lexer);
	    return visit_expression (lexer, new Binary (token->getId (), left, right));
	} else lexer.rewind();
	return left;
    }

    ExpressionPtr visit_ulow (Lexer & lexer) {
	ExpressionPtr left = visit_low (lexer);
	TokenPtr token = lexer.next ();       
	if (find (token->getId () ,
		  {INF, SUP, EQUALS, SUP_EQUALS, INF_EQUALS, NOT_EQUALS, IS, NOT})) {
	    ExpressionPtr right = visit_low (lexer);
	    return visit_ulow (lexer, new Binary (token->getId (), left, right));
	} else lexer.rewind();
	return left;	
    }

    ExpressionPtr visit_ulow (Lexer & lexer, ExpressionPtr left) {
	TokenPtr token = lexer.next ();       
	if (find (token->getId () ,
		  {INF, SUP, EQUALS, SUP_EQUALS, INF_EQUALS, NOT_EQUALS, IS, NOT})) {
	    ExpressionPtr right = visit_low (lexer);
	    return visit_ulow (lexer, new Binary (token->getId (), left, right));
	} else lexer.rewind();
	return left;	
    }

    ExpressionPtr visit_low (Lexer & lexer) {
	ExpressionPtr left = visit_high (lexer);
	TokenPtr token = lexer.next ();       
	if (find (token->getId () ,
		  {PLUS, SUB, OR_BIT, LEFT_BIT, RIGHT_BIT, XOR})) {
	    ExpressionPtr right = visit_high (lexer);
	    return visit_low (lexer, new Binary (token->getId (), left, right));
	} else lexer.rewind();
	return left;	
    }
    
    ExpressionPtr visit_low (Lexer & lexer, ExpressionPtr left) {
	TokenPtr token = lexer.next ();       
	if (find (token->getId () ,
		  {MUL, DIV, AND_BIT, MODULO, POINT_POINT, IN})) {
	    ExpressionPtr right = visit_high (lexer);
	    return visit_low (lexer, new Binary (token->getId (), left, right));
	} else lexer.rewind();
	return left;	
    }

    ExpressionPtr visit_high (Lexer & lexer) {
	ExpressionPtr left = visit_pth (lexer);
	TokenPtr token = lexer.next ();       
	if (find (token->getId () ,
		  {MUL, DIV, AND_BIT, MODULO, POINT_POINT, IN})) {
	    ExpressionPtr right = visit_pth (lexer);
	    return visit_high (lexer, new Binary (token->getId (), left, right));
	} else lexer.rewind();
	return left;	
    }

    ExpressionPtr visit_pth (Lexer & lexer, ExpressionPtr left) {
	TokenPtr token = lexer.next ();       
	if (find (token->getId () ,
		  {MUL, DIV, AND_BIT, MODULO, POINT_POINT, IN})) {
	    ExpressionPtr right = visit_pth (lexer);
	    return visit_high (lexer, new Binary (token->getId (), left, right));
	} else lexer.rewind();
	return left;	
    }
    
    ExpressionPtr visit_pth (Lexer & lexer) {
	TokenPtr token = lexer.next ();
	if (find (token->getId (),
		  {STAR, S_AND, MINUS, NOT})) {
	    return visit_before_pth (lexer, token);
	} else {
	    if (token->getId() == PAR_G)
		return visit_pth_par (lexer);
	    else
		return visit_pth_wpar (lexer, token);
    }
    
    ExpressionPtr visit_pth_par (Lexer & lexer) {	
	auto exp = visit_expression_ult (file);
	token = lexer.next ();
	if (token->getId () != PAR_D)
	    error_at (token->getLocus(),
		      "%s attendue, mais %s trouvé",
		      getTokenDescription (PAR_D),
		      getTokenDescription (token->getId()));
	token = lexer.getNext ();
	if (find (token->getId (),
		  {CROCHET_G, PAR_G, DOT})) {
	    return visit_suite (lexer, token, exp);
	} else lexer.rewind ();
	return exp;
    }

    ExpressionPtr visit_pth_wpar (Lexer & lexer, TokenPtr token) {
	if (token->getId() == IF) return visit_inside_if (lexer);
	else if (token->getId () == FOR) return visit_inside_for (lexer);
	else if (token->getId () == NEW) return visit_new (lexer);
	else if (token->getId () == CAST) return visit_cast (lexer);
	else {
	    lexer.rewind ();
	    ExpressionPtr constante = visit_constante (lexer);
	    if (constante != NULL) return constante;
	    ExpressionPtr left = visit_left_op (lexer);
	    token = lexer.next ();
	    if (find (token->getId(),
		      {PPLUS, SSUB})) {
		return visit_after (lexer, token, left);
	    } else lexer.rewind ();
	    return left;
	}
    }
        
};
