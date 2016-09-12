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
	while (!(*token == Token::EOF_TOKEN())) {
	    if (*token == DEF) decls.push_back(visit_def (lexer));
	    else if (*token == IMPORT) decls.push_back (visit_import (lexer));
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
	if (!(*word == PAR_G)) error_at (word->getLocus (),
					 "%s attendue, mais %s trouvé\n",
					 getTokenDescription(PAR_G),
					 getTokenDescription(word->getId ()));
	word = lexer.next ();
	if (!(*word == PAR_D)) {
	    lexer.rewind ();
	    while (1) {
		exps.push_back (visit_var_declaration (lexer));
		word = lexer.next ();
		if (!(*word == PAR_D)) break;
		else if (!(*word == VIRGULE))
		    error_at (word->getLocus (),
			      "[%s, %s] attendue, mais %s trouvé\n",
			      getTokenDescription(PAR_D),
			      getTokenDescription(VIRGULE),
			      getTokenDescription(word->getId ()));
	    }
	}
	
	word = lexer.next ();
	if ((*word == DEUX_POINT)) {
	    Syntax::VarPtr type = visit_type (lexer);
	    return DeclarationPtr (new Syntax::Function (name, type, exps, visit_block (lexer)));
	} else lexer.rewind ();
	return DeclarationPtr (new Syntax::Function (name, exps, visit_block (lexer)));
    }

    Syntax::VarPtr visit_var_declaration (Lexer & lexer) {
    }

    Syntax::VarPtr visit_type (Lexer & lexer) {
    }

    TokenPtr visit_identifiant (Lexer & lexer) {
    }
    
    Syntax::BlockPtr visit_block (Lexer & lexer) {
    }
    
    
};
