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
	
	Generator Visitor::validateBinary (const syntax::Binary & bin, bool isFromCall) {
	    if (bin.getLocation () == Token::DCOLON) {
		auto subVisitor = SubVisitor::init (*this);
		return subVisitor.validate (bin);
	    } else if (bin.getLocation () == Token::DOT) {
		auto dotVisitor = DotVisitor::init (*this);
		return dotVisitor.validate (bin, isFromCall);
	    } else if (bin.getLocation () == Token::DOT_AND) {
		try {
		    auto intr = syntax::Intrinsics::init (lexing::Word::init (bin.getLocation (), Keys::ALIAS), bin.getLeft ());
		    auto n_bin = syntax::Binary::init (lexing::Word::init (bin.getLocation (), Token::DOT), intr, bin.getRight (), bin.getType ());
		    return this-> validateBinary (n_bin.to <syntax::Binary> (), isFromCall);		    
		} catch (Error::ErrorList list) {
		    auto note = Ymir::Error::createNote (bin.getLocation ());
		    list.errors.back ().addNote (note);
		    throw list;
		}
	    } else {
		auto binVisitor = BinaryVisitor::init (*this);
		return binVisitor.validate (bin);
	    }
	}

	Generator Visitor::validateUnary (const syntax::Unary & un) {
	    auto unVisitor = UnaryVisitor::init (*this);
	    return unVisitor.validate (un);
	}

	Generator Visitor::validateMultOperator (const syntax::MultOperator & op) {
	    if (op.getEnd () == Token::RCRO) {
		auto bracketVisitor = BracketVisitor::init (*this);
		return bracketVisitor.validate (op);
	    } else if (op.getEnd () == Token::RPAR) {
		auto callVisitor = CallVisitor::init (*this);
		return callVisitor.validate (op);
	    }

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}

	
    }
}
