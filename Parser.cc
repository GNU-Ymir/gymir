#include "Parser.hh"
#include "Lexer.hh"
#include "Parser.hh"
#include "Visitor.hh"

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

namespace Ymir {

    void Parser::parse_program () {
	Syntax::AstPtr synt_ast = this -> syntax_analyse ();
	synt_ast -> print ();
	Semantic::Ast sem_ast = this -> semantic_analyse (synt_ast);
	this -> define_gcc_symbols ();
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

    Semantic::Ast Parser::semantic_analyse (Syntax::AstPtr&) {
	return Semantic::empty ();
    }

    void Parser::define_gcc_symbols () {	
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
