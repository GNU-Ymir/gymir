#include "Parser.hh"
#include "syntax/Lexer.hh"
#include "syntax/Token.hh"
#include "syntax/Visitor.hh"

#include "errors/Error.hh"
#include "ast/_.hh"
#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/pack/FrameTable.hh>
#include <ymir/utils/Mangler.hh>

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

    using namespace semantic;
    
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
    
    syntax::Program Parser::syntax_analyse () {
	auto visitor = syntax::Visitor (lexer);
	auto ret = visitor.visit ();
	if (Ymir::Error::nb_errors > 0) {
	    Ymir::Error::fail ("NB Error : %d", Ymir::Error::nb_errors);
	}
	this-> lexer.dispose ();
	return ret;
    }

    void Parser::semantic_time (syntax::Program prg) {
	Table::instance ().purge ();
	FrameTable::instance ().purge ();

	prg-> declare ();

	for (auto it : FrameTable::instance ().structs ()) {
	    it-> TempOp ({});
	}
	
	for (auto it : FrameTable::instance ().pures ()) {
	    it-> validate ();
	}
	
	if (Ymir::Error::nb_errors > 0)
	    Ymir::Error::fail ("NB Error : %d", Ymir::Error::nb_errors);	
    }

    void Parser::lint_time () {
	for (auto it : FrameTable::instance ().finals ()) {
	    it-> finalize ();
	}

	if (Ymir::Error::nb_errors > 0) // Ne doit pas arriver
	    Ymir::Error::assert ("NB Error : %d", Ymir::Error::nb_errors);
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
    auto prg = parser.syntax_analyse ();
    fclose (file);
    
    parser.semantic_time (prg);
    parser.lint_time ();    
}
