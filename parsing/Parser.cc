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
#include <ymir/documentation/Visitor.hh>
#include <ymir/utils/Ref.hh>
#include <ymir/semantic/Symbol.hh>
#include <ymir/global/State.hh>
#include <ymir/global/Core.hh>

using namespace Ymir;


void ymir_parse_file (const char * filename) {
    {
	Ymir::Parser parser (filename);
	parser.run ();
    }
    
    control_memory_leakage ();
}

void ymir_parse_files (int nb_files, const char ** files) {
    {     
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

	if (global::State::instance ().isDocDumpingActive ()) {
	    auto doc_visit = documentation::Visitor::init (validator);
	    auto res = doc_visit.dump (module);
	    println (res);
	}
	
	auto generator = semantic::generator::Visitor::init ();
	for (auto & gen : validator.getGenerators ()) {
	    generator.generate (gen);
	}
	
	generator.finalize ();
	semantic::Symbol::purge ();
    }
    
}
