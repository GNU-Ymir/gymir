#include <ymir/parsing/Parser.hh>
#include <ymir/errors/_.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/lexing/Token.hh>
#include <ymir/syntax/declaration/Function.hh>
#include <ymir/syntax/declaration/Module.hh>
#include <ymir/syntax/visitor/Visitor.hh>
#include <ymir/semantic/declarator/Visitor.hh>
#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/generator/Visitor.hh>
#include <ymir/utils/Ref.hh>
#include <ymir/semantic/Symbol.hh>
#include <ymir/global/State.hh>
#include <ymir/global/Core.hh>

using namespace Ymir;


void ymir_parse_file (const char * filename) {
    {
	Ymir::Parser parser (filename);
	// TODO generate docs
	parser.run ();
    }
    
    control_memory_leakage ();
}

void ymir_parse_files (int nb_files, const char ** files) {
    { 
	// TODO generate docs
	// TODO The version file is a thing for a dependency manager, not directly the compiler
	Parser::readVersionFile (); // No need to do it for each file
    
	for (int i = 0 ; i < nb_files ; i++) {
	    ymir_parse_file (files [i]);
	}
    }
    control_memory_leakage ();
}

namespace Ymir {

    Parser::Parser (const char * filename) :
	_path (filename),
	_module (syntax::Declaration::empty ())
    {}

    Parser::~Parser () {}

    void Parser::readVersionFile () {
	auto file_path = global::State::instance ().getVersionFile ();
	auto file = fopen (file_path.c_str (), "r");
	if (file != NULL) {
	    log ("Reading version file : ", file_path);
	    lexing::Lexer lexer (file_path.c_str (), file,
				 {Token::SPACE, Token::RETURN, Token::RRETURN, Token::TAB},
				 {
				     {Token::LCOMM1, {Token::RCOMM1, ""}},
					 {Token::LCOMM2, {Token::RETURN, ""}},
					     {Token::LCOMM3, {Token::RCOMM3, ""}},
						 {Token::LCOMM4, {Token::RCOMM3, Token::STAR}},
						     {Token::LCOMM5, {Token::RCOMM5, Token::PLUS}} 
				 }
	    );
	    while (true) {
		auto name = lexer.next ();
		if (!name.isEof ())
		    global::State::instance ().activateVersion (name.str);
		else break;
	    }
	    fclose (file);
	} else {
	}
    }
    
    void Parser::run () {
	std::vector <std::string> errors;
	try {
	    syntaxicTime ();
	    semanticTime ();
	} catch (Error::ErrorList list) {
	    list.print ();
	    Error::end (ExternalError::get (COMPILATION_END));
	} catch (Error::FatalError ftl) {
	    ftl.print ();
	    Error::end (ExternalError::get (COMPILATION_END));
	}
    }
    
    void Parser::syntaxicTime () {
	auto file = fopen (this-> _path.c_str (), "r");
	if (file == NULL) Error::occur (ExternalError::get (NO_SUCH_FILE), _path);
	
	auto visitor = syntax::Visitor::init (this-> _path, file);
	this-> _module = visitor.visitModGlobal ();
	fclose (file);
    }

    void Parser::semanticTime () {
	auto declarator = semantic::declarator::Visitor::init ();
	auto module = declarator.visit (this-> _module);	
	
	auto validator = semantic::validator::Visitor::init ();	
	validator.validate (module);
	
	auto generator = semantic::generator::Visitor::init ();
	for (auto & gen : validator.getGenerators ()) {
	    generator.generate (gen);
	}
	
	generator.finalize ();
	semantic::Symbol::purge ();
    }
    
}
