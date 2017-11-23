#include "Parser.hh"
#include "syntax/Lexer.hh"
#include "syntax/Token.hh"
#include "syntax/Visitor.hh"

#include "errors/Error.hh"
#include "ast/_.hh"

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

    Parser::Parser (const char * filename, FILE * file) :
	lexer (filename, file,
	       {Token::SPACE, Token::RETURN, Token::RRETURN, Token::TAB},
	       {
		   {Token::LCOMM1, Token::RCOMM1},
		       {Token::LCOMM2, Token::RETURN},
			   {Token::LCOMM3, Token::RCOMM3}
	       }
	)
    {}    

    void Parser::parse_program () {
	auto prg = this-> syntax_analyse ();
	if (Ymir::Error::nb_errors > 0) {
	    Ymir::Error::assert ("NB Error : %d", Ymir::Error::nb_errors);
	}

	prg-> declare ();
	if (Ymir::Error::nb_errors > 0)
	    Ymir::Error::assert ("NB Error : %d", Ymir::Error::nb_errors);
	
	// this -> define_gcc_symbols ();
	// if (Ymir::Error::nb_errors > 0) stop ();
			
	// //Vidange memoire
	// Semantic::TypeInfo::clear ();
    }

    syntax::Program Parser::syntax_analyse () {
	auto visitor = syntax::Visitor (lexer);
	return visitor.visit ();
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

    Ymir::Parser parser (filename, file);
    
    parser.parse_program ();
    fclose (file);
}
