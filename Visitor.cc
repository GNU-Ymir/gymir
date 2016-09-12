#include "Visitor.hh"
#include "config.h"
#include "coretypes.h"
#include "input.h"
#include "diagnostic.h"

namespace Lexical {

    Syntax::Ast Visitor::visit () {
	return visit_program (lexer);
    }

    Syntax::Ast visit_program (Lexer & lexer) {
	auto declCreators = {
	    {DEF, &visit_def},
	    {IMPORT, &visit_import},
	};
	
	TokenPtr token = lexer.next ();
	std::vector<Syntax::Declaration> decls;
	while (token->token_id != Token::EOF()) {
	    auto elem = declCreators.find (token->token_id);
	    if (elem != declCreators.end ())
		decls.insertBack ((*elem) (lexer));
	    else error_at (token->locus,
			   "[%s, %s] attendues, mais %s trouvé\n",
			   get_token_description(DEF),
			   get_token_description(IMPORT),
			   token->get_token_description());
	    
	    token = lexer.next ();
	}
    }

    Syntax::Declaration visit_import (Lexer & lexer) {
	
    }

    Syntax::Declaration visit_def (Lexer & lexer) {
    }
    

};
