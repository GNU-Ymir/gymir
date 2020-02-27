#include <ymir/parsing/Parser.hh>
#include <ymir/errors/_.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/syntax/declaration/Function.hh>
#include <ymir/syntax/declaration/Module.hh>
#include <ymir/syntax/visitor/Visitor.hh>
#include <ymir/semantic/declarator/Visitor.hh>
#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/generator/Visitor.hh>
#include <ymir/utils/Ref.hh>
#include <ymir/semantic/Symbol.hh>

using namespace Ymir;


void ymir_parse_file (const char * filename) {
    Ymir::Parser parser (filename);
    // TODO generate docs
    parser.run ();
}

void ymir_parse_files (int nb_files, const char ** files) {
    // TODO generate docs
    for (int i = 0 ; i < nb_files ; i++) {
	ymir_parse_file (files [i]);
    }    
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
