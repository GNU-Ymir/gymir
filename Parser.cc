#include "Parser.hh"
#include "Lexer.hh"
#include "Parser.hh"
#include "Visitor.hh"
#include "AstGC.hh"
#include "Tree.hh"
#include "TypeInfo.hh"
#include "Error.hh"

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "target.h"
#include "tree.h"
#include "tree-iterator.h"
#include "input.h"
#include "diagnostic.h"
#include "stringpool.h"
#include "cgraph.h"
#include "gimplify.h"
#include "gimple-expr.h"
#include "convert.h"
#include "print-tree.h"
#include "stor-layout.h"
#include "fold-const.h"
#include "PureFrames.hh"

namespace Ymir {
       
    void stop () {
	Syntax::AstGC::instance().clean ();
	Semantic::TypeInfo::clear ();
	fatal_error (UNKNOWN_LOCATION,
		     "Trop d'erreur la compilation a du terminer");
    }

    void Parser::parse_program () {
	Syntax::AstPtr synt_ast = this -> syntax_analyse ();
	if (Ymir::Error::nb_errors > 0) stop ();
	this -> semantic_analyse (synt_ast);
	if (Ymir::Error::nb_errors > 0) stop ();
	this -> define_gcc_symbols ();
	if (Ymir::Error::nb_errors > 0) stop ();
			
	//Vidange memoire
	Syntax::AstGC::instance().clean ();
	Semantic::TypeInfo::clear ();
    }

    Syntax::AstPtr Parser::syntax_analyse () {
	lexer.setKeys ({":", ";", "|", "[", "]", "?", "'", " ", "\n", "\t", "<->",
		    "<-", "=", "+=", "*=", "-=", "/=", "&&", "||", "<", ">", "==", ">=", "<=", "!=", "++", "--",
		    "+", "-", "|", "<<", ">>", "^", "*", "/", "&", "%", "!",
		    "\r", "(", ")", "{", "}", ",", ".", "'", "\"", "..", "*#", "#*", "#", "\\", "$", "//", "/*", "*/"});
	
	lexer.setComments ({std::pair<std::string, std::string> ("#", "\n"),
		    std::pair<std::string, std::string> ("/*", "*/"),
		    std::pair<std::string, std::string> ("//", "\n")
		    });
	
	lexer.setSkip ({" ", "\n", "\r", "\t"});
	Syntax::Visitor visitor (lexer);
	return visitor.visit ();
    }

    void Parser::semantic_analyse (Syntax::AstPtr& ast) {
	ast -> semantic ();
    }

    void Parser::define_gcc_symbols () {	
	for (auto & it : Semantic::PureFrames::allFinal ()) {
	    it.finalize ();
	}
    }
    
};


static void ymir_parse_file (const char * filename);

void ymir_parse_files (int num_files, const char ** files) {
    for (int i = 0; i < num_files; i++)
	ymir_parse_file ( files[i] );
}

static void ymir_parse_file (const char * filename) {
    FILE * file = fopen (filename, "r");
    if (file == NULL) 
	fatal_error (UNKNOWN_LOCATION, "cannot open filename %s: %m", filename);
    Lexical::Lexer lexer (filename, file);
    Ymir::Parser parser (lexer);
    
    parser.parse_program ();
    fclose (file);
}
