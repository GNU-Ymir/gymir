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
	    return DeclarationPtr (new Syntax::Function (name, type, exps, visit_block (lexer)));
	} else lexer.rewind ();
	return DeclarationPtr (new Syntax::Function (name, exps, visit_block (lexer)));
    }
       
    Syntax::VarPtr visit_var_declaration (Lexer & lexer) {
	TokenPtr token = visit_identifiant (lexer);
	TokenPtr next = lexer.next ();
	if (next->getId () == DEUX_POINT) {
	    auto type = visit_type ();
	    return VarPtr (new TypedVar (token, type));
	} else lexer.rewind ();
	return VarPtr (new Var (token));
    }

    Syntax::VarPtr visit_complex_var (Lexer & lexer) {
	TokenPtr token = visit_identifiant (lexer);
	TokenPtr next = lexer.next ();
	if (next->getId () == DEUX_POINT) {
	    auto type = visit_type ();
	    return VarPtr (new TypedVar (token, type));
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
		    
		}
	    } else {
		lexer.rewind;
	    }
	} else lexer.rewind ();
	return VarPtr (new Var (token));
    }

    TokenPtr visit_identifiant (Lexer & lexer) {
	TokenPtr token = lexer.next ();
	if (token->getId() == OTHER) {
	    return token;
	} else error_at (token->getLocus (), "Un identifiant est attendue, mais %s trouvé", getTokenDescription(token->getId()));
    }
    
    Syntax::BlockPtr visit_block (Lexer & lexer) {
    }
    
    
};
