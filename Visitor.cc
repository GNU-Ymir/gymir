#include "syntax/_.hh"
#include "errors/_.hh"
#include "syntax/Keys.hh"

#include <map>
#include <vector>

using namespace std;

namespace syntax {

    using namespace lexical;

    std::string join (std::vector <std::string> elems) {
	stringstream ss;
	int i = 0;
	for (auto it : elems) {
	    ss << it << (i == (int) elems.size () - 1 ? "" : ", ");
	    i++;
	}
	return ss.str ();
    }
    
    Program Visitor::visit () {
    	return NULL; 
    }

    Program Visitor::visit_program () {	
	auto token = lexer.next ();
	std::vector<Declaration> decls;
	while (token.isEof ()) {
	    auto decl = visit_declaration (false);
	    if (decl != NULL) decls.push_back (decl);
	    else if (token == Keys::PUBLIC) {
		auto pub_decls = visit_public_block ();
		for (auto it : pub_decls) decls.push_back (it);
	    } else if (token == Keys::PRIVATE) {
		auto prv_decls = visit_private_block ();
		for (auto it : prv_decls) decls.push_back (it);
	    } 
	}
	return new IProgram (Word::eof (), decls);
    }
    
    std::vector<Declaration> Visitor::visit_public_block () {
    	auto next = lexer.next ();
	std::vector <Declaration> decls;
    	if (next == Token::LACC) {
    	    while (true) {
    		auto decl = visit_declaration (false);
    		if (decl) {
    		    decls.push_back (decl);
    		    decls.back ()-> is_public (true);
    		} else {
    		    auto tok = lexer.next ();
    		    if (tok != Token::RACC)
    			Ymir::Error::fatal (tok.getLocus (),
    					    "[%s] attendues, mais %s trouvé\n",
					    join ({Keys::DEF, Keys::IMPORT, Keys::EXTERN, Keys::STRUCT, Keys::ENUM,
							Keys::STATIC, Keys::SELF, Keys::TRAIT, Keys::IMPL }),
					    tok.getStr ()
			);
    		    break;
    		}
    	    }
    	} else {
    	    decls.push_back (visit_declaration (true));
    	    decls.back ()-> is_public (true);
    	}
    	return decls;
    }

    std::vector<Declaration> Visitor::visit_private_block () {
    	auto next = lexer.next ();
	std::vector <Declaration> decls;
    	if (next == Token::LACC) {
    	    while (true) {
    		auto decl = visit_declaration (false);
    		if (decl) {
    		    decls.push_back (decl);
    		    decls.back ()-> is_public (false);
    		} else {
    		    auto tok = lexer.next ();
    		    if (tok != Token::RACC)
    			Ymir::Error::fatal (tok.getLocus (),
    					    "[%s] attendues, mais %s trouvé\n",
					    join ({Keys::DEF, Keys::IMPORT, Keys::EXTERN, Keys::STRUCT, Keys::ENUM,
							Keys::STATIC, Keys::SELF, Keys::TRAIT, Keys::IMPL }),
					    tok.getStr ()
			);
    		    break;
    		}
    	    }
    	} else {
    	    decls.push_back (visit_declaration (true));
    	    decls.back ()-> is_public (false);
    	}
    	return decls;
    }

    Declaration Visitor::visit_declaration (bool fatal) {
    	auto token = lexer.next ();
    	if (token == Keys::DEF) return NULL;//visit_function ();
    	else if (token == Keys::IMPORT) return NULL;//visit_import ();
    	else if (token == Keys::EXTERN) return NULL;//visit_extern ();
    	else if (token == Keys::STRUCT) return NULL;//visit_struct ();
    	else if (token == Keys::ENUM) return NULL;//visit_enum ();
    	else if (token == Keys::STATIC) return NULL;//visit_global ();
    	else if (token == Keys::SELF) return NULL;//visit_self ();
    	else if (token == Keys::TRAIT) return NULL;//visit_trait ();
    	else if (token == Keys::IMPL) return NULL;//visit_impl ();	      	
    	else if (fatal) Ymir::Error::fatal (token.getLocus (),
					    "[%s] attendues, mais %s trouvé\n",
					    join ({Keys::DEF, Keys::IMPORT, Keys::EXTERN, Keys::STRUCT, Keys::ENUM,
							Keys::STATIC, Keys::SELF, Keys::TRAIT, Keys::IMPL }),
					    token.getStr ()	
	);
    	else lexer.rewind ();
    	return NULL;
    }

    
};
