#include <ymir/semantic/validator/_.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/semantic/declarator/Visitor.hh>
#include <ymir/semantic/generator/Mangler.hh>
#include <ymir/utils/map.hh>
#include <ymir/global/Core.hh>
#include <ymir/utils/Path.hh>
#include <ymir/global/State.hh>
#include <string>
#include <algorithm>

using namespace global;

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;       

	syntax::Expression Visitor::createVarFromPath (const lexing::Word & loc, const std::vector <std::string> & names_) {
	    auto names = names_;
	    syntax::Expression last = syntax::Var::init (lexing::Word::init (loc, names [0]));
	    names = std::vector <std::string> (names.begin () + 1, names.end ());
	    while (names.size () > 0) {
		last = syntax::Binary::init (
		    lexing::Word::init (loc, Token::DCOLON),
		    last,
		    syntax::Var::init (lexing::Word::init (loc, names [0])),
		    syntax::Expression::empty ()
		    );
		names = std::vector <std::string> (names.begin () + 1, names.end ());
	    }	    
	    return last;
	}

	syntax::Expression Visitor::createClassTypeFromPath (const lexing::Word & loc, const std::vector <std::string> & names_) {
	    auto last = createVarFromPath (loc, names_);
	    return syntax::Unary::init (
		lexing::Word::init (loc, Token::AND),
		last
		);
	}

	Generator Visitor::addElseToConditional (const Generator & gen, const Generator & _else) {
	    match (gen) {
		of (Conditional, cd) {
		    if (cd.getElse ().isEmpty ()) {
			return Conditional::init (cd.getLocation (), cd.getType (), cd.getTest (), cd.getContent (), _else, cd.isComplete ());
		    } else {
			auto addElse_ = addElseToConditional (cd.getElse (), _else);
			return Conditional::init (cd.getLocation (), cd.getType (), cd.getTest (), cd.getContent (), addElse_, cd.isComplete ());
		    }
		}
		elfo {
		    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");   
		}
	    }
	    return Generator::empty ();
	}

	Generator Visitor::setCompleteConditional (const Generator & gen) {
	    match (gen) {
		of (Conditional, cd) {
		    if (cd.getElse ().isEmpty ()) {
			return Conditional::init (cd.getLocation (), cd.getType (), cd.getTest (), cd.getContent (), Generator::empty (), true);
		    } else {
			auto _else = setCompleteConditional (cd.getElse ());
			return Conditional::init (cd.getLocation (), cd.getType (), cd.getTest (), cd.getContent (), _else, cd.isComplete ());
		    }
		} fo;		
	    }	    
	    return gen;
	}
                     				
    }
    
}
